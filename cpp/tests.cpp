// Correctness tests for the native Game of Life engine. Not a general test
// framework -- just direct checks against known Game of Life behavior,
// run via `tests.exe`, exiting non-zero on the first failure.

#include "engine.hpp"
#include "experiments.hpp"
#include <set>
#include <limits>

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

    {
        check(life::Rule::parse("B36/S23").birth == 72, "HighLife birth mask");
        check(life::Rule::parse("B63/S32").notation() == "B36/S23", "canonical ordering");
        check(life::Rule::parse("B/S").notation() == "B/S", "empty masks accepted");
        for (const auto* text : {"", "B9/S23", "B33/S23", "B3/S22", "B-1/S", "S23/B3", "B3/S23x", "B3/S23/S", "B3S23"}) {
            bool rejected = false;
            try { life::Rule::parse(text); } catch (const std::invalid_argument&) { rejected = true; }
            check(rejected, std::string("reject malformed rule: ") + text);
        }
        // Exhaust every local center/neighbor-count case for each mask bit.
        bool truthTable = true;
        const int neighbors[] = {6,7,8,11,13,16,17,18};
        for (int bit = 0; bit < 18; ++bit) for (int center = 0; center < 2; ++center)
            for (int n = 0; n <= 8; ++n) {
                std::vector<std::uint8_t> grid(25);
                grid[12] = center;
                for (int i = 0; i < n; ++i) grid[neighbors[i]] = 1;
                life::Options o;
                o.rule = {static_cast<std::uint16_t>(bit < 9 ? 1 << bit : 0),
                          static_cast<std::uint16_t>(bit >= 9 ? 1 << (bit - 9) : 0)};
                life::Random random(7);
                const auto out = life::advance(grid, 5, 5, o, random);
                truthTable &= (out.cells[12] > 0) == (bit == center * 9 + n);
            }
        check(truthTable, "all 18 independent birth/survival choices");
    }
    {
        auto grid = life::randomGrid(20, 20, 0.3, 55);
        life::Random a(12), b(12), unused(99);
        life::Options zero;
        check(life::advance(grid,20,20,zero,unused).cells == life::evolve(grid,20,20).cells,
              "zero noise preserves exact binary age bytes");
        check(unused.state == 99, "zero noise consumes no RNG values");
        life::Options noisy; noisy.noise = 0.1;
        check(life::advance(grid,20,20,noisy,a).cells == life::advance(grid,20,20,noisy,b).cells,
              "seeded noise reproduces cells");
        noisy.noise = 1;
        auto deterministic = life::evolve(grid,20,20).cells;
        auto flipped = life::advance(grid,20,20,noisy,a).cells;
        bool complement = true;
        for (std::size_t i=0;i<grid.size();++i) complement &= (flipped[i]>0) != (deterministic[i]>0);
        check(complement, "p=1 complements post-update occupancy");
        life::Random statsRandom(42); int hits=0;
        for(int i=0;i<100000;++i) hits += statsRandom.event(0.1);
        check(hits>9500 && hits<10500, "seeded perturbation frequency near configured probability");
        check(life::randomGrid(20,20,0.3,55) == grid, "seeded initial grids reproduce");
        bool rejected=false;
        try { noisy.noise=std::numeric_limits<double>::quiet_NaN();life::advance(grid,20,20,noisy,a); }
        catch(const std::invalid_argument&) {rejected=true;}
        check(rejected,"reject non-finite noise");
        rejected=false;
        try { life::advance(grid,3,3,zero,a); } catch(const std::invalid_argument&) {rejected=true;}
        check(rejected,"reject grid size mismatch");
    }
    {
        life::Options generations{life::Rule::parse("B2/S"),4,0};
        life::Random random(4);
        std::vector<std::uint8_t> grid(25);grid[12]=1;
        auto first=life::advance(grid,5,5,generations,random).cells;
        auto second=life::advance(first,5,5,generations,random).cells;
        auto third=life::advance(second,5,5,generations,random).cells;
        check(first[12]==2 && second[12]==3 && third[12]==0,"active death and complete refractory decay");
        grid.assign(25,0);grid[11]=1;grid[13]=1;
        check(life::advance(grid,5,5,generations,random).cells[12]==1,"Generations birth counts active neighbors");
        grid[12]=2;
        check(life::advance(grid,5,5,generations,random).cells[12]==3,"refractory cell ignores birth condition");
        grid[12]=0;grid[11]=2;grid[13]=3;
        check(life::advance(grid,5,5,generations,random).cells[12]==0,"refractory neighbors do not count");
        generations.rule=life::Rule::parse("B/S0");grid.assign(25,0);grid[12]=1;
        check(life::advance(grid,5,5,generations,random).cells[12]==1,"Generations survival remains active");
        generations.noise=1;grid[12]=2;
        check(life::advance(grid,5,5,generations,random).cells[12]!=3,"noise replaces deterministic refractory state");
        grid[12]=4;bool rejected=false;
        try {life::advance(grid,5,5,generations,random);}catch(const std::invalid_argument&){rejected=true;}
        check(rejected,"invalid explicit state rejected");
    }
    {
        const auto rules=life::sampleRules(2026), repeat=life::sampleRules(2026);
        std::set<std::string> unique;
        bool same=true;
        for(std::size_t i=0;i<rules.size();++i){unique.insert(rules[i].notation());same &= rules[i].notation()==repeat[i].notation();}
        check(unique.size()==100 && same,"100 distinct reproducibly sampled rules");
        life::TrialConfig c; c.cols=12;c.rows=12;c.steps=30;c.options.noise=0.01;
        auto first=life::runTrial(c), second=life::runTrial(c);
        check(life::trialJson(c,first)==life::trialJson(c,second),"complete seeded experiment reproducibility");
        check(first.density.size()==31 && first.distance.size()==31,"time series include initial generation");
        c.options.noise=0;
        auto zero=life::runTrial(c);
        check(std::all_of(zero.distance.begin(),zero.distance.end(),[](auto d){return d==0;}),"zero-noise baseline distance is zero");
        c.options.rule=life::Rule::parse("B/S");
        check(life::runTrial(c).category=="extinction","extinction classification");
        c.options.rule=life::Rule::parse("B0/S");c.density=0;c.steps=2;
        auto b0=life::runTrial(c);
        check(b0.category=="oscillatory" && b0.period==2,"B0 empty grid revives; not absorbing extinction");
        auto blinker=life::createPattern(findPattern("blinker"));
        auto phase=life::evolve(blinker).cells;
        auto returned=life::evolve(phase).cells;
        check(life::recurrence({life::semantic(blinker,2),life::semantic(phase,2)},life::semantic(returned,2))==2,"period-2 detection ignores ages");
        check(life::recurrence({{0,1,2}}, {0,1,2})==1,"fixed-point detection");
        check(life::recurrence({{0,1,2}}, {0,1,3})==0,"refractory states are distinct for recurrence");
    }

    {
        // Independent Conway reference preserves the original formula and ages.
        auto grid=life::randomGrid(12,9,0.4,761);
        for(auto& c:grid) if(c) c=255;
        std::vector<std::uint8_t> reference(grid.size());
        for(int y=0;y<9;++y) for(int x=0;x<12;++x){
            int n=0;
            for(int dy=-1;dy<=1;++dy) for(int dx=-1;dx<=1;++dx)
                if(dx||dy)n+=grid[((y+dy+9)%9)*12+(x+dx+12)%12]>0;
            int i=y*12+x;
            if(n==3||(grid[i]>0&&n==2))reference[i]=grid[i]?255:1;
        }
        check(life::evolve(grid,12,9).cells==reference,"independent Conway reference, toroidal edges and age saturation");
        life::Random random(123);
        life::Options options{life::Rule::parse("B36/S23"),5,0};
        grid=life::randomGrid(12,9,0.3,18);
        check(life::advance(grid,12,9,options,random).cells==life::advance(grid,12,9,options,random).cells && random.state==123,
              "multi-state zero-noise update is deterministic and does not consume RNG");
    }

    {
        check(!life::confirmedMatch({0,1},{1,0},42,42),"hash collision requires full equality");
        check(life::randomGrid(16,16,.3,1)!=life::randomGrid(16,16,.3,2),"different initial seeds differ");
        life::TrialConfig c;c.cols=8;c.rows=8;c.steps=5;c.options.states=3;c.options.rule=life::Rule::parse("B2/S");c.allStates=true;
        auto r=life::runTrial(c);bool valid=true;for(auto counts:r.populations){int sum=0;for(auto n:counts)sum+=n;valid &= sum==64 && counts.size()==3;}
        check(valid,"per-state counts conserve grid size");
        std::vector<std::uint8_t> grid(25);grid[12]=1;life::Random rng(1);
        auto a=life::advance(grid,5,5,c.options,rng).cells;auto b=life::advance(a,5,5,c.options,rng).cells;
        check(a[12]==2 && b[12]==0,"Brian's Brain active refractory dead cycle");
        check(life::trialJson(c,r).find("life-trial-v2")!=std::string::npos,"versioned JSON schema identifier");
    }
    {
        bool truth=true;
        const int neighbor[]={6,7,8,11,13,16,17,18};
        for(int states=3;states<=8;++states)for(int center=0;center<states;++center)for(int n=0;n<=8;++n){
            std::vector<std::uint8_t> grid(25);grid[12]=center;for(int i=0;i<n;++i)grid[neighbor[i]]=1;
            life::Options options{life::Rule::parse("B36/S23"),states,0};life::Random rng(123);
            int expected=center==0?(n==3||n==6):center==1?((n==2||n==3)?1:2):(center+1)%states;
            truth &= life::advance(grid,5,5,options,rng).cells[12]==expected && rng.state==123;
        }
        check(truth,"all C=3..8 center states and neighbor counts follow Generations definition");
    }
    if (failures == 0) {
        std::cout << "\nAll tests passed.\n";
        return 0;
    }
    std::cerr << "\n" << failures << " test(s) failed.\n";
    return 1;
}
