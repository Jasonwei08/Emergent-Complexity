#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace life {

constexpr int COLS = 80;
constexpr int ROWS = 48;

struct Pattern {
    std::string id;
    std::string name;
    std::string type;
    std::vector<std::string> rows;
    std::string description;
};

extern const std::vector<Pattern> patterns;

struct EvolveResult {
    std::vector<std::uint8_t> cells;
    int births = 0;
    int deaths = 0;
    int population = 0;
};

struct Rule {
    std::uint16_t birth = 1 << 3;
    std::uint16_t survival = (1 << 2) | (1 << 3);
    static Rule parse(const std::string& text);
    std::string notation() const;
};

// Explicit generator and integer threshold mapping keep native/Wasm runs identical.
struct Random {
    std::uint32_t state;
    explicit Random(std::uint32_t seed) : state(seed) {}
    std::uint32_t next();
    bool event(double probability);
};

struct Options {
    Rule rule;
    int states = 2; // 2: binary age bytes; 3..8: explicit Generations states.
    double noise = 0;
};

bool active(std::uint8_t cell, int states);
std::vector<std::uint8_t> randomGrid(int cols, int rows, double density, std::uint32_t seed, int states = 2, bool allStates = false);
EvolveResult advance(const std::vector<std::uint8_t>& cells, int cols, int rows,
                     const Options& options, Random& random);

// Advances the grid by one generation using Conway's B3/S23 rules on a
// toroidal (wrapping) grid. A surviving cell's value is incremented
// (capped at 255) so callers can distinguish long-lived cells from
// newborns.
EvolveResult evolve(const std::vector<std::uint8_t>& cells, int cols = COLS, int rows = ROWS);

// Places a pattern centered on a cols x rows grid and returns the
// resulting cell buffer.
std::vector<std::uint8_t> createPattern(const Pattern& pattern, int cols = COLS, int rows = ROWS);

} // namespace life
