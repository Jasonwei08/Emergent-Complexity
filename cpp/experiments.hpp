#pragma once
#include "engine.hpp"

namespace life {
struct Criteria {
    int maxPeriod = 16;
    double growth = 0.1, disorder = 0.15, persistent = 0.001;
};
struct TrialConfig {
    int cols = 64, rows = 64, steps = 500;
    double density = 0.2;
    std::uint32_t seed = 42;
    Options options;
    bool allStates = false;
    Criteria criteria;
};
struct TrialResult {
    std::vector<double> density, occupied, change, distance;
    std::vector<std::uint8_t> initial, finalCells;
    std::vector<std::vector<int>> populations;
    std::vector<double> spread, movement;
    std::vector<int> births, deaths;
    std::vector<std::uint32_t> hashes;
    int recurrenceGeneration = -1;
    double meanDensity = 0, maxDensity = 0, spatialGrowth = 0, mobility = 0;
    int firstEmpty = -1, period = 0;
    double tailChange = 0, growth = 0;
    std::string category;
};
// Compare semantic states: binary ages are visual metadata, not CA states.
std::vector<std::uint8_t> semantic(std::vector<std::uint8_t> cells, int states);
std::uint32_t gridHash(const std::vector<std::uint8_t>& cells);
bool confirmedMatch(const std::vector<std::uint8_t>& a, const std::vector<std::uint8_t>& b, std::uint32_t ha, std::uint32_t hb);
int recurrence(const std::vector<std::vector<std::uint8_t>>& history,
               const std::vector<std::uint8_t>& cells);
std::vector<Rule> sampleRules(std::uint32_t seed, int count = 100);
TrialResult runTrial(const TrialConfig& config);
std::string trialJson(const TrialConfig& config, const TrialResult& result);
} // namespace life
