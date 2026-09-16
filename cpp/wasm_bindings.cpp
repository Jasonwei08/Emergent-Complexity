// Emscripten/embind bindings exposing the C++ engine to the browser as a
// WebAssembly module. This is the only file that knows about the browser;
// engine.hpp/engine.cpp remain plain, dependency-free C++.

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <stdexcept>

#include "engine.hpp"
#include "experiments.hpp"

using emscripten::val;

namespace {

val patternToVal(const life::Pattern& p) {
    val obj = val::object();
    obj.set("id", val(p.id));
    obj.set("name", val(p.name));
    obj.set("type", val(p.type));
    obj.set("description", val(p.description));

    val rows = val::array();
    for (std::size_t i = 0; i < p.rows.size(); ++i) rows.set(i, val(p.rows[i]));
    obj.set("rows", rows);

    return obj;
}

// Copies a cell buffer into a real JS Uint8Array (not a live view into the
// wasm heap, which can be invalidated by memory growth).
val cellsToUint8Array(const std::vector<std::uint8_t>& cells) {
    val view(emscripten::typed_memory_view(cells.size(), cells.data()));
    val out = val::global("Uint8Array").new_(cells.size());
    out.call<void>("set", view);
    return out;
}

const life::Pattern& patternById(const std::string& id) {
    for (const auto& p : life::patterns) {
        if (p.id == id) return p;
    }
    throw std::out_of_range("unknown pattern id: " + id);
}

} // namespace

val jsPatterns() {
    val arr = val::array();
    for (std::size_t i = 0; i < life::patterns.size(); ++i) arr.set(i, patternToVal(life::patterns[i]));
    return arr;
}

val jsCreatePattern(const std::string& id) {
    return cellsToUint8Array(life::createPattern(patternById(id)));
}

val jsEvolve(val jsCells) {
    auto cells = emscripten::vecFromJSArray<std::uint8_t>(jsCells);
    if (cells.size() != static_cast<std::size_t>(life::COLS) * life::ROWS)
        throw std::length_error("cell buffer has the wrong size");

    auto result = life::evolve(cells);

    val out = val::object();
    out.set("cells", cellsToUint8Array(result.cells));
    out.set("births", result.births);
    out.set("deaths", result.deaths);
    out.set("population", result.population);
    return out;
}

std::string jsValidate(const std::string& text) {
    try { return life::Rule::parse(text).notation(); }
    catch (const std::exception&) { return ""; }
}
val jsAdvance(val jsCells, int cols, int rows, const std::string& rule,
              int states, double noise, std::uint32_t seed) {
    life::Random random(seed);
    auto result = life::advance(emscripten::vecFromJSArray<std::uint8_t>(jsCells),
        cols, rows, {life::Rule::parse(rule), states, noise}, random);
    val out = val::object();
    out.set("cells", cellsToUint8Array(result.cells));
    out.set("births", result.births);
    out.set("population", result.population);
    out.set("rngState", random.state);
    return out;
}
val jsRandom(int cols, int rows, double density, std::uint32_t seed) {
    return cellsToUint8Array(life::randomGrid(cols, rows, density, seed));
}
val jsSample(std::uint32_t seed) {
    val out = val::array();
    auto rules = life::sampleRules(seed);
    for (std::size_t i = 0; i < rules.size(); ++i) out.set(i, rules[i].notation());
    return out;
}
std::string jsTrial(const std::string& rule, int states, double noise, int cols,
                    int rows, double density, int steps, std::uint32_t seed) {
    life::TrialConfig c{cols, rows, steps, density, seed, {life::Rule::parse(rule), states, noise}, false, {}};
    return life::trialJson(c, life::runTrial(c));
}

std::string jsTrialAll(const std::string& rule,int states,double noise,int cols,int rows,double density,int steps,std::uint32_t seed,bool allStates) {
    life::TrialConfig c{cols,rows,steps,density,seed,{life::Rule::parse(rule),states,noise},false,{}};
    c.allStates=allStates;return life::trialJson(c,life::runTrial(c));
}
val jsRandomAll(int cols,int rows,double density,std::uint32_t seed,int states,bool allStates) {return cellsToUint8Array(life::randomGrid(cols,rows,density,seed,states,allStates));}
EMSCRIPTEN_BINDINGS(life_module) {
    emscripten::function("runTrialAll", &jsTrialAll);
    emscripten::function("randomGridAll", &jsRandomAll);
    emscripten::function("validateRule", &jsValidate);
    emscripten::function("advance", &jsAdvance);
    emscripten::function("randomGrid", &jsRandom);
    emscripten::function("sampleRules", &jsSample);
    emscripten::function("runTrial", &jsTrial);
    emscripten::function("patterns", &jsPatterns);
    emscripten::function("createPattern", &jsCreatePattern);
    emscripten::function("evolve", &jsEvolve);
    emscripten::constant("COLS", life::COLS);
    emscripten::constant("ROWS", life::ROWS);
}
