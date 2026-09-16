#include "engine.hpp"

#include <algorithm>
#include <cstddef>

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

EvolveResult evolve(const std::vector<std::uint8_t>& cells, int cols, int rows) {
    EvolveResult result;
    result.cells.assign(cells.size(), 0);

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int n = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;
                    int ny = ((y + dy) % rows + rows) % rows;
                    int nx = ((x + dx) % cols + cols) % cols;
                    if (cells[static_cast<std::size_t>(ny) * cols + nx] > 0) ++n;
                }
            }

            const std::size_t i = static_cast<std::size_t>(y) * cols + x;
            const bool alive = cells[i] > 0;

            if (n == 3 || (alive && n == 2)) {
                result.cells[i] = alive ? static_cast<std::uint8_t>(std::min(255, cells[i] + 1))
                                         : static_cast<std::uint8_t>(1);
                ++result.population;
                if (!alive) ++result.births;
            } else if (alive) {
                ++result.deaths;
            }
        }
    }

    return result;
}

std::vector<std::uint8_t> createPattern(const Pattern& pattern, int cols, int rows) {
    std::vector<std::uint8_t> cells(static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows), 0);

    std::size_t width = 0;
    for (const auto& row : pattern.rows) width = std::max(width, row.size());

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
