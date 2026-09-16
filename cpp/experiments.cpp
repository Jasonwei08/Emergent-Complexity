#include "experiments.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace life {
std::vector<std::uint8_t> semantic(std::vector<std::uint8_t> cells, int states) {
    if (states == 2) for (auto& c : cells) c = c > 0;
    return cells;
}
std::uint32_t gridHash(const std::vector<std::uint8_t>& cells) {
    std::uint32_t h=2166136261u; for(auto v:cells) {h^=v;h*=16777619u;} return h;
}
bool confirmedMatch(const std::vector<std::uint8_t>& a,const std::vector<std::uint8_t>& b,std::uint32_t ha,std::uint32_t hb) {return ha==hb && a==b;}
int recurrence(const std::vector<std::vector<std::uint8_t>>& history,
               const std::vector<std::uint8_t>& cells) {
    for (int p = 1; p <= static_cast<int>(history.size()); ++p)
        if (confirmedMatch(cells, history[history.size()-p], gridHash(cells), gridHash(history[history.size()-p]))) return p;
    return 0;
}
std::vector<Rule> sampleRules(std::uint32_t seed, int count) {
    if (count < 1 || count > 262144) throw std::invalid_argument("Invalid sample size");
    Random random(seed);
    std::vector<bool> seen(262144, false);
    std::vector<Rule> rules;
    while (static_cast<int>(rules.size()) < count) {
        const auto bits = random.next() >> 14; // Use the high bits, not weak LCG low bits.
        if (seen[bits]) continue;
        seen[bits] = true;
        rules.push_back(Rule{static_cast<std::uint16_t>(bits & 511),
                             static_cast<std::uint16_t>(bits >> 9)});
    }
    return rules;
}
TrialResult runTrial(const TrialConfig& c) {
    if (c.steps < 1 || c.steps > 2000) throw std::invalid_argument("Steps must be 1..2000");
    if(c.criteria.maxPeriod<1 || c.criteria.maxPeriod>128 || !std::isfinite(c.criteria.growth) || !std::isfinite(c.criteria.disorder) || !std::isfinite(c.criteria.persistent)) throw std::invalid_argument("Invalid classification criteria");
    auto cells = randomGrid(c.cols, c.rows, c.density, c.seed, c.options.states, c.allStates);
    TrialResult r;
    r.initial = cells;
    auto baseline = cells;
    auto deterministic = c.options;
    deterministic.noise = 0;
    Random noise(c.seed ^ 0x9e3779b9u), unused(0);
    std::vector<std::vector<std::uint8_t>> history;
    std::vector<std::uint32_t> historyHashes;
    const double size = cells.size();
    double lastX=0,lastY=0; bool hadPopulation=false;
    auto record = [&](const std::vector<std::uint8_t>& grid) {
        const auto population = std::count_if(grid.begin(), grid.end(),
            [&](auto v) { return active(v, c.options.states); });
        std::vector<int> counts(c.options.states,0);
        int minX=c.cols,minY=c.rows,maxX=-1,maxY=-1; double sx=0,sy=0;
        for(std::size_t i=0;i<grid.size();++i) {
            ++counts[c.options.states==2 ? (grid[i]>0) : grid[i]];
            if(active(grid[i],c.options.states)) {int x=i%c.cols,y=i/c.cols; minX=std::min(minX,x);maxX=std::max(maxX,x);minY=std::min(minY,y);maxY=std::max(maxY,y);sx+=x;sy+=y;}
        }
        r.populations.push_back(counts);
        r.spread.push_back(population ? (maxX-minX+1)*(maxY-minY+1)/size : 0);
        double x=population?sx/population:0,y=population?sy/population:0;
        r.movement.push_back(population && hadPopulation ? std::hypot(x-lastX,y-lastY)/std::hypot(c.cols,c.rows):0);
        lastX=x;lastY=y;hadPopulation=population>0;
        r.hashes.push_back(gridHash(semantic(grid,c.options.states)));
        r.density.push_back(population / size);
        r.occupied.push_back(std::count_if(grid.begin(), grid.end(), [](auto v) { return v > 0; }) / size);
        if (population == 0 && r.firstEmpty < 0) r.firstEmpty = static_cast<int>(r.density.size()) - 1;
    };
    record(cells);
    r.births.push_back(0); r.deaths.push_back(0);
    r.change.push_back(0);
    r.distance.push_back(0);
    for (int t = 1; t <= c.steps; ++t) {
        auto previous = semantic(cells, c.options.states);
        history.push_back(previous); historyHashes.push_back(gridHash(previous));
        if (history.size() > static_cast<std::size_t>(c.criteria.maxPeriod)) { history.erase(history.begin()); historyHashes.erase(historyHashes.begin()); }
        auto update = advance(cells, c.cols, c.rows, c.options, noise);
        cells = update.cells; r.births.push_back(update.births);r.deaths.push_back(update.deaths);
        if (c.options.noise > 0) baseline = advance(baseline, c.cols, c.rows, deterministic, unused).cells;
        auto current = semantic(cells, c.options.states);
        auto reference = c.options.noise > 0 ? semantic(baseline, c.options.states) : current;
        int changed = 0, different = 0;
        for (std::size_t i = 0; i < cells.size(); ++i) {
            changed += previous[i] != current[i];
            different += reference[i] != current[i];
        }
        r.change.push_back(changed / size);
        r.distance.push_back(different / size);
        record(cells);
        r.period=0; const auto hash=gridHash(current);
        for(int p=1;p<=static_cast<int>(history.size());++p) if(confirmedMatch(current,history[history.size()-p],hash,historyHashes[history.size()-p])) {r.period=p;break;}
        if(r.period && r.recurrenceGeneration<0) r.recurrenceGeneration=t;
    }
    const int window = std::max(1, c.steps / 4);
    auto tailMean = [&](const std::vector<double>& v) {
        return std::accumulate(v.end() - window, v.end(), 0.0) / window;
    };
    r.tailChange = tailMean(r.change);
    r.growth = tailMean(r.density) -
        std::accumulate(r.density.begin(), r.density.begin() + window, 0.0) / window;
    r.meanDensity=std::accumulate(r.density.begin(),r.density.end(),0.0)/r.density.size();
    r.maxDensity=*std::max_element(r.density.begin(),r.density.end());
    r.spatialGrowth=tailMean(r.spread)-std::accumulate(r.spread.begin(),r.spread.begin()+window,0.0)/window;
    r.mobility=tailMean(r.movement);
    // These labels describe an observed trajectory, never the whole rule space.
    if (r.density.back() == 0 && c.options.noise == 0 && !(c.options.rule.birth & 1))
        r.category = "extinction";
    else if (r.period == 1 && c.options.noise == 0) r.category = "stable";
    else if (r.period > 1 && c.options.noise == 0) r.category = "oscillatory";
    else if (r.growth > c.criteria.growth && r.spatialGrowth > c.criteria.growth) r.category = "growing";
    else if (r.tailChange > c.criteria.disorder) r.category = "disordered";
    else if(r.density.back()>0 && r.tailChange>c.criteria.persistent) r.category="bounded persistent activity";
    else r.category = "unclassified";
    r.finalCells = cells;
    return r;
}
std::string trialJson(const TrialConfig& c, const TrialResult& r) {
    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\"version\":2,\"rule\":\"" << c.options.rule.notation()
        << "\",\"states\":" << c.options.states << ",\"noise\":" << c.options.noise
        << ",\"seed\":" << c.seed << ",\"noiseSeed\":" << (c.seed ^ 0x9e3779b9u)
        << ",\"cols\":" << c.cols << ",\"rows\":" << c.rows << ",\"steps\":" << c.steps
        << ",\"initialDensity\":" << c.density << ",\"category\":\"" << r.category
        << "\",\"firstEmpty\":" << r.firstEmpty << ",\"terminalPeriod\":" << r.period
        << ",\"tailChange\":" << r.tailChange << ",\"growth\":" << r.growth;
    out << ",\"schema\":\"life-trial-v2\",\"boundary\":\"toroidal\",\"initialization\":\"" << (c.allStates?"all-states":"active-only")
        << "\",\"notation\":\"" << c.options.rule.notation() << (c.options.states>2?"/C"+std::to_string(c.options.states):"")
        << "\",\"birthMask\":" << c.options.rule.birth << ",\"survivalMask\":" << c.options.rule.survival
        << ",\"recurrenceGeneration\":" << r.recurrenceGeneration << ",\"meanDensity\":" << r.meanDensity << ",\"maxDensity\":" << r.maxDensity
        << ",\"finalDensity\":" << r.density.back() << ",\"spatialGrowth\":" << r.spatialGrowth << ",\"mobility\":" << r.mobility
        << ",\"growthRate\":" << r.growth / (c.steps-std::max(1,c.steps/4)+1)
        << ",\"elapsedGenerations\":" << c.steps
        << ",\"extinct\":" << (r.category=="extinction"?"true":"false")
        << ",\"extinctionGeneration\":" << (r.category=="extinction"?r.firstEmpty:-1)
        << ",\"fixedPoint\":" << (r.category=="stable"?"true":"false")
        << ",\"oscillator\":" << (r.category=="oscillatory"?"true":"false")
        << ",\"reachedLimitUnclassified\":" << (r.category=="unclassified"?"true":"false")
        << ",\"criteria\":{\"maxPeriod\":" << c.criteria.maxPeriod << ",\"growth\":" << c.criteria.growth << ",\"disorder\":" << c.criteria.disorder << ",\"persistent\":" << c.criteria.persistent << "}";
    out << ",\"populations\":[";
    for(std::size_t t=0;t<r.populations.size();++t) {if(t)out<<',';out<<'[';for(std::size_t k=0;k<r.populations[t].size();++k){if(k)out<<',';out<<r.populations[t][k];}out<<']';}out<<']';
    auto array = [&](const char* name, const auto& values) {
        out << ",\"" << name << "\":[";
        bool first = true;
        for (auto value : values) { if (!first) out << ','; first = false; out << +value; }
        out << ']';
    };
    array("births",r.births);array("deaths",r.deaths);array("spread",r.spread);array("movement",r.movement);array("hashes",r.hashes);
    array("density", r.density); array("occupied", r.occupied);
    array("change", r.change); array("distance", r.distance);
    array("initial", r.initial); array("final", r.finalCells);
    out << '}';
    return out.str();
}
} // namespace life
