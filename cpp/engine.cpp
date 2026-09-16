#include "engine.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <stdexcept>

namespace life {

const std::vector<Pattern> patterns = {
    {"glider", "Glider", "Spaceship",
     {"010", "001", "111"},
     "A tiny traveler. This five-cell pattern moves diagonally across the grid every four generations."},
    {"pulsar", "Pulsar", "Oscillator",
     {"0011100011100", "0000000000000", "1000010100001", "1000010100001", "1000010100001",
      "0011100011100", "0000000000000", "0011100011100", "1000010100001", "1000010100001",
      "1000010100001", "0000000000000", "0011100011100"},
     "A beautifully symmetric oscillator that cycles through three distinct states."},
    {"gosper", "Glider gun", "Generator",
     {"000000000000000000000000100000000000",
      "000000000000000000000010100000000000",
      "000000000000110000001100000000000011",
      "000000000001000100001100000000000011",
      "110000000010000010001100000000000000",
      "110000000010001011000010100000000000",
      "000000000010000010000000100000000000",
      "000000000001000100000000000000000000",
      "000000000000110000000000000000000000"},
     "Order that keeps on giving. Gosper’s glider gun produces a new glider every 30 generations."},
    {"pentomino", "R-pentomino", "Methuselah",
     {"011", "110", "010"},
     "Five cells, a long story. This little seed unfolds into a surprisingly complex world."},
    {"lwss", "Lightweight ship", "Spaceship",
     {"10010", "00001", "10001", "01111"},
     "A compact spaceship that travels horizontally, repeating its shape every four generations."},
    {"blinker", "Blinker", "Oscillator",
     {"111"},
     "The simplest oscillator. Three cells alternate between a horizontal and vertical line."},
};

namespace {
void dimensions(int cols, int rows) {
    if (cols < 3 || rows < 3 || cols > 512 || rows > 512)
        throw std::invalid_argument("Grid dimensions must be 3..512");
}
void probability(double p) {
    if (!std::isfinite(p) || p < 0 || p > 1)
        throw std::invalid_argument("Probability must be finite and in [0,1]");
}
}

Rule Rule::parse(const std::string& text) {
    Rule rule{0, 0};
    if (text.empty() || text[0] != 'B') throw std::invalid_argument("Use B[counts]/S[counts]");
    const auto slash = text.find("/S");
    if (slash == std::string::npos) throw std::invalid_argument("Missing /S");
    auto counts = [&](std::size_t first, std::size_t last, std::uint16_t& mask) {
        for (auto i = first; i < last; ++i) {
            const char c = text[i];
            if (c < '0' || c > '8' || (mask & (1 << (c - '0'))))
                throw std::invalid_argument("Counts must be unique digits 0..8");
            mask |= 1 << (c - '0');
        }
    };
    counts(1, slash, rule.birth);
    counts(slash + 2, text.size(), rule.survival);
    return rule;
}

std::string Rule::notation() const {
    std::string text = "B";
    for (int i = 0; i <= 8; ++i) if (birth & (1 << i)) text += char('0' + i);
    text += "/S";
    for (int i = 0; i <= 8; ++i) if (survival & (1 << i)) text += char('0' + i);
    return text;
}

std::uint32_t Random::next() {
    // Numerical Recipes LCG; unsigned overflow is defined modulo 2^32.
    state = state * 1664525u + 1013904223u;
    return state;
}
bool Random::event(double p) {
    return static_cast<double>(next()) < std::floor(p * 4294967296.0);
}
bool active(std::uint8_t cell, int states) { return states == 2 ? cell > 0 : cell == 1; }

std::vector<std::uint8_t> randomGrid(int cols, int rows, double density, std::uint32_t seed, int states, bool allStates) {
    dimensions(cols, rows);
    probability(density);
    Random random(seed);
    std::vector<std::uint8_t> cells(static_cast<std::size_t>(cols) * rows);
    if (states < 2 || states > 8) throw std::invalid_argument("States must be 2..8");
    for (auto& c : cells) {
        c = random.event(density) ? 1 : 0;
        if(c && allStates) c = 1 + static_cast<unsigned>(static_cast<double>(random.next()) / 4294967296.0 * (states-1));
    }
    return cells;
}

EvolveResult advance(const std::vector<std::uint8_t>& cells, int cols, int rows,
                     const Options& options, Random& random) {
    dimensions(cols, rows);
    probability(options.noise);
    if (cells.size() != static_cast<std::size_t>(cols) * rows || options.states < 2 ||
        options.states > 8 || options.rule.birth > 511 || options.rule.survival > 511)
        throw std::invalid_argument("Invalid buffer or rule options");
    if (options.states > 2 && std::any_of(cells.begin(), cells.end(),
        [&](auto c) { return c >= options.states; })) throw std::invalid_argument("Invalid cell state");
    EvolveResult result;
    result.cells.assign(cells.size(), 0);
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int n = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    const int ny = (y + dy + rows) % rows;
                    const int nx = (x + dx + cols) % cols;
                    n += active(cells[static_cast<std::size_t>(ny) * cols + nx], options.states);
                }
            }
            const std::size_t i = static_cast<std::size_t>(y) * cols + x;
            const bool alive = active(cells[i], options.states);
            std::uint8_t value = 0;
            if (options.states > 2 && cells[i] >= 2) {
                value = cells[i] + 1 < options.states ? cells[i] + 1 : 0;
            } else if ((alive ? options.rule.survival : options.rule.birth) & (1 << n)) {
                value = options.states == 2 && alive ? std::min(255, cells[i] + 1) : 1;
            } else if (alive && options.states > 2) {
                value = 2;
            }
            // Noise follows the synchronous update: binary flip, or a uniformly
            // chosen different semantic state in Generations.
            // p=0 consumes no random values and preserves binary age bytes exactly.
            if (options.noise > 0 && random.event(options.noise))
                {
                if (options.states == 2) value = value ? 0 : 1;
                else { // Uniform different semantic state, using rejection to avoid modulo bias.
                    const unsigned choices = options.states - 1;
                    const unsigned limit = 0xffffffffu - (0xffffffffu % choices);
                    unsigned draw; do { draw = random.next(); } while (draw >= limit);
                    unsigned replacement = draw % choices;
                    value = replacement >= value ? replacement + 1 : replacement;
                }
            }
            result.cells[i] = value;
            const bool now = active(value, options.states);
            result.population += now;
            result.births += !alive && now;
            result.deaths += alive && !now;
        }
    }
    return result;
}

EvolveResult evolve(const std::vector<std::uint8_t>& cells, int cols, int rows) {
    Random random(0);
    return advance(cells, cols, rows, Options{}, random);
}

std::vector<std::uint8_t> createPattern(const Pattern& pattern, int cols, int rows) {
    dimensions(cols, rows);
    std::vector<std::uint8_t> cells(static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows), 0);

    std::size_t width = 0;
    for (const auto& row : pattern.rows) width = std::max(width, row.size());

    if (width > static_cast<std::size_t>(cols) || pattern.rows.size() > static_cast<std::size_t>(rows))
        throw std::invalid_argument("Pattern does not fit grid");
    const int ox = (cols - static_cast<int>(width)) / 2;
    const int oy = (rows - static_cast<int>(pattern.rows.size())) / 2;

    for (std::size_t y = 0; y < pattern.rows.size(); ++y) {
        const auto& row = pattern.rows[y];
        for (std::size_t x = 0; x < row.size(); ++x) {
            if (row[x] == '1') {
                const std::size_t idx = static_cast<std::size_t>(oy + static_cast<int>(y)) * cols +
                                         static_cast<std::size_t>(ox + static_cast<int>(x));
                cells[idx] = 1;
            }
        }
    }

    return cells;
}

} // namespace life
