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

// Advances the grid by one generation using Conway's B3/S23 rules on a
// toroidal (wrapping) grid. A surviving cell's value is incremented
// (capped at 255) so callers can distinguish long-lived cells from
// newborns.
EvolveResult evolve(const std::vector<std::uint8_t>& cells, int cols = COLS, int rows = ROWS);

// Places a pattern centered on a cols x rows grid and returns the
// resulting cell buffer.
std::vector<std::uint8_t> createPattern(const Pattern& pattern, int cols = COLS, int rows = ROWS);

} // namespace life
