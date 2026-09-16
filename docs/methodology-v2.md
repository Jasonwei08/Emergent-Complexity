# Version 2 methodology and replay

C++ is the source of transitions and measurements. Binary internal age bytes retain original behavior but semantic hashes normalize positive bytes to 1. Generations uses C=3..8, active state 1 only, and refractory progression 2,...,C-1,0. B/S masks permit empty sets; the interface supports B3/S23/C5 by storing B/S and C separately. Presets: binary Life and HighLife; Brian's Brain B2/S/C3; refractory Life B3/S23/C5.

Noise occurs after each deterministic synchronous transition. Binary flips state. Generations replaces the next state uniformly with a different state using rejection sampling. Zero consumes no noise RNG draws. Historical v1 perturbation results are incompatible and remain separate.

Density d means probability of nonzero initialization. Active-only uses state 1. All-state distributes nonzero cells approximately uniformly among 1..C-1 (32-bit scaled draw). Thus its expected active density is d/(C-1), a documented comparison confound. The ordered light-to-dark palette uses hollow refractory cells, numerals at >=13 pixel cell width, and keyboard announcements; it is not an accessibility certification.

## Batch design

`npm run experiments` runs 64x64 toroidal grids for 500 updates. Exactly 100 distinct high-18-bit LCG rule draws, duplicate rejection, master seed 2026. Five trials per rule per density .10/.30/.50: 1500 sampled trials. Absent Life/HighLife controls are separate (30). Ranking chooses a sampled candidate for 10 trials at each density and p=0,1e-5,1e-4,1e-3,1e-2, alongside Life/HighLife (450). Generations crosses B2/S and B3/S23, C=3/5/8, two initializers, three densities, five p levels, five trials (900).

Initial seed = (master + (trial+1)*2654435761 + round(density*1e6)*2246822519) mod 2^32. Rule index is deliberately excluded to share grids across rules, and is recorded separately. Noise seed = initial seed XOR 0x9e3779b9. LCG32 next = 1664525*x+1013904223. Probability events compare next to floor(p*2^32). The deterministic streams are not a proof of statistical independence.

Every generation (including zero) stores per-state population, active/nonzero densities, active-status births/deaths after perturbation, semantic change, fixed-coordinate active bounding-box fraction, normalized centroid movement and FNV-1a grid hash. Initial/final grids permit replay. Toroidal seam artifacts limit spatial/mobility interpretation. First-empty includes temporary active extinction; it is absorbing only at zero noise without B0. All trajectories run to the horizon.

The first/last quarter-window density difference is growth; dividing by steps-window+1 gives the rate between window centers. Spatial growth uses bounding-box fraction. Tail mean change is sustained activity; tail centroid displacement is mobility. Recent-grid hashes require full equality before recurrence; max period 16. First recurrence and terminal period can refer to different events. Noise grid recurrence does not include RNG state and cannot prove an oscillator.

C++ Criteria exports maxPeriod=16, growth=.1, disorder=.15, persistent=.001. Priority: extinction (p=0, no B0, final active zero), stable (terminal p=0 period1), oscillatory (terminal p=0 period2..16), growing (density AND spatial growth>.1), disordered (tail change>.15), bounded persistent activity (final active>0 and tail change>.001), otherwise unclassified. These are finite empirical categories. Each rule keeps its trial distribution and dominant observed category.

Interest = mean(0<finalDensity<.9 + terminalPeriod>2 + max(0,spatialGrowth) + min(1,20*mobility) + 4*a*(1-a)) + (number of categories-1)/6; indicator comparisons contribute 0 or 1 and a is tail change. Ties use rule text order. This cannot detect life or reproduction.

## Evidence

`results/v2/{sampled,controls,detail,generations}.json` contains complete versioned batches. `summary.json` has derived trial summaries, rankings and runtime. `sampled-rules.json` preserves the exact list. `replay-*.json` stores selected-rule trial 0..2 at each density with p=0. Use the browser's Reopen exported JSON input, select a trial, then Replay. The production artifact intentionally excludes research data; download it from the repository and import locally.

`python scripts/generate-report.py` validates all 2880 trial schemas, population conservation, masks, and paired sampled initial grids, and produces figures/tables. Install matplotlib to regenerate plots. Local optional packages are in ignored build/report-tools. Report bibliography metadata was checked against publisher/author sources. No synthetic experimental image is used.
