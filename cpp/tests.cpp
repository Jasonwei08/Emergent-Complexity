// Correctness tests for the native Game of Life engine. Not a general test
// framework -- just direct checks against known Game of Life behavior,
// run via `tests.exe`, exiting non-zero on the first failure.

#include "engine.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace {

int failures = 0;

void check(bool condition, const std::string& label) {
    if (!condition) {
        std::cerr << "FAIL: " << label << "\n";
        ++failures;
    } else {
        std::cout << "ok: " << label << "\n";
    }
}

const life::Pattern& findPattern(const std::string& id) {
    auto it = std::find_if(life::patterns.begin(), life::patterns.end(),
                            [&](const life::Pattern& p) { return p.id == id; });
    if (it == life::patterns.end()) throw std::runtime_error("missing pattern: " + id);
    return *it;
}

int countAlive(const std::vector<std::uint8_t>& cells) {
    int n = 0;
    for (auto c : cells) n += c > 0;
    return n;
}

} // namespace

int main() {
    check(life::patterns.size() == 6, "six patterns are defined");

    // Blinker oscillates with period 2: two generations must restore the
    // exact same set of alive cells.
    {
        const auto& blinker = findPattern("blinker");
        auto gen0 = life::createPattern(blinker);
        check(countAlive(gen0) == 3, "blinker starts with 3 live cells");

        auto gen1 = life::evolve(gen0).cells;
        check(countAlive(gen1) == 3, "blinker has 3 live cells after 1 generation");

        auto gen2 = life::evolve(gen1).cells;
        bool sameAsStart = true;
        for (std::size_t i = 0; i < gen0.size(); ++i) {
            if ((gen0[i] > 0) != (gen2[i] > 0)) { sameAsStart = false; break; }
        }
        check(sameAsStart, "blinker returns to its starting shape after 2 generations");

        // The blinker's center cell is shared by both the horizontal and
        // vertical phase; only the two end cells actually move.
        int overlap = 0;
        for (std::size_t i = 0; i < gen0.size(); ++i) {
            if ((gen0[i] > 0) && (gen1[i] > 0)) ++overlap;
        }
        check(overlap == 1, "blinker's horizontal and vertical phases share exactly the center cell");
    }

    // Glider translates by (+1, +1) every 4 generations.
    {
        const auto& glider = findPattern("glider");
        auto gen0 = life::createPattern(glider);
        check(countAlive(gen0) == 5, "glider starts with 5 live cells");

        auto cells = gen0;
        for (int i = 0; i < 4; ++i) cells = life::evolve(cells).cells;
        check(countAlive(cells) == 5, "glider still has 5 live cells after 4 generations");

        bool shifted = true;
        for (int y = 0; y < life::ROWS; ++y) {
            for (int x = 0; x < life::COLS; ++x) {
                int srcY = (y - 1 + life::ROWS) % life::ROWS;
                int srcX = (x - 1 + life::COLS) % life::COLS;
                bool expectedAlive = gen0[static_cast<std::size_t>(srcY) * life::COLS + srcX] > 0;
                bool actualAlive = cells[static_cast<std::size_t>(y) * life::COLS + x] > 0;
                if (expectedAlive != actualAlive) shifted = false;
            }
        }
        check(shifted, "glider shifts by (+1,+1) after 4 generations");
    }

    // births/deaths bookkeeping: a cell survives, an empty cell is born
    // exactly where evolve() says a birth happened, and a dying cell is
    // counted as a death and excluded from population.
    {
        std::vector<std::uint8_t> cells(life::COLS * life::ROWS, 0);
        const auto& blinker = findPattern("blinker");
        cells = life::createPattern(blinker);
        auto result = life::evolve(cells);
        check(result.births == 2, "blinker step reports 2 births");
        check(result.deaths == 2, "blinker step reports 2 deaths");
        check(result.population == 3, "blinker step reports population of 3");
        check(static_cast<int>(result.cells.size()) == life::COLS * life::ROWS,
              "evolve() preserves grid size");
    }

    // A surviving cell's value increments (capped at 255) instead of
    // resetting to 1, so long-lived cells can be told apart from newborns.
    {
        std::vector<std::uint8_t> cells(life::COLS * life::ROWS, 0);
        const auto& blinker = findPattern("blinker");
        cells = life::createPattern(blinker);
        for (auto& c : cells) if (c > 0) c = 5; // pretend this cell has survived 5 generations
        auto result = life::evolve(cells);
        bool foundIncremented = false;
        for (std::size_t i = 0; i < cells.size(); ++i) {
            if (cells[i] == 5 && result.cells[i] == 6) foundIncremented = true;
        }
        check(foundIncremented, "a surviving cell's age increments rather than resetting to 1");
    }

    // createPattern centers the pattern on the grid.
    {
        const auto& blinker = findPattern("blinker"); // single row "111"
        auto cells = life::createPattern(blinker);
        const int expectedRow = (life::ROWS - 1) / 2;
        const int expectedCol0 = (life::COLS - 3) / 2;
        check(cells[static_cast<std::size_t>(expectedRow) * life::COLS + expectedCol0] > 0 &&
                  cells[static_cast<std::size_t>(expectedRow) * life::COLS + expectedCol0 + 1] > 0 &&
                  cells[static_cast<std::size_t>(expectedRow) * life::COLS + expectedCol0 + 2] > 0,
              "createPattern centers the blinker at the expected row/column");
    }

    if (failures == 0) {
        std::cout << "\nAll tests passed.\n";
        return 0;
    }
    std::cerr << "\n" << failures << " test(s) failed.\n";
    return 1;
}
