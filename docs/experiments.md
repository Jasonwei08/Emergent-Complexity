> Historical v1 document. Superseded by [v2 methodology](methodology-v2.md) and the [complete report](../report/main.tex). Do not combine v1 and v2 noise results.

# Recorded experiments (actual execution)

All results below were produced by the rebuilt C++ engine through Wasm using
`scripts/representative-experiments.mjs`. The environment used Node 24.21.0 and
Emscripten 6.0.9. These are measurements from small finite experiments, not a
complete report or a claim about all configurations.

## Sample

Master seed 2026; 100 distinct binary rules; three trials per rule; 32x32 torus;
initial active density probability 0.2; 200 steps; zero noise. Trial seeds are
2654437787, 1013906252 and 3668342013. All 300 trials completed.

| Trajectory category | Count |
| --- | ---: |
| Disordered (turnover heuristic) | 209 |
| Stable | 43 |
| Oscillatory (period <=16) | 38 |
| Growing | 3 |
| Unclassified | 7 |
| Extinction | 0 |

These are trial counts, not counts of intrinsically disordered or stable rules.
No extinction was recorded in this sample at this horizon; that is not evidence
that extinction is impossible in the wider rule space. Category priority and
cutoffs are defined in `design.md`. Full density, occupancy, change-rate and
reference-distance traces plus initial/final grids are preserved in
`results/sampled.json`.

## A selected rule worth closer inspection

Selection was algorithmic: the first sampled trajectory with a terminal period
above two (fallback: any oscillator, then the first rule). This chose
**B3678/S045678**. Its three trajectories ended with periods **1, 4 and 2**,
respectively. Their late-window change rates were 0, 0.005859375 and 0.01560546875.
The period-2 run had a large early-to-late mean-density increase of about 0.8353;
its label is oscillatory because terminal recurrence takes priority over growth.

To inspect in the website, run the default sample with seed 2026, find that rule,
click Inspect, select each seed and replay. The density/change chart and the
step control make the differences reviewable. Alternatively select this B/S rule
and use current-rule experiments with the same settings. Its mixtures of sparse
and dense outcomes deserve a larger density/seed sweep. No visual claim about
specific objects, mobility or reproduction has been made.

## Noise comparisons

The same 32x32/density/steps settings were used with five matched seeds per rule
and four noise levels. The first three seeds match the sample. Each trajectory
also has an independently updated zero-noise reference. The table reports mean
final active density and mean final semantic distance from that reference.

| Rule | p | Active density | Distance |
| --- | ---: | ---: | ---: |
| B3/S23 | 0 | 0.07910 | 0 |
| B3/S23 | 0.001 | 0.04785 | 0.12070 |
| B3/S23 | 0.01 | 0.17813 | 0.22598 |
| B3/S23 | 0.05 | 0.31641 | 0.33770 |
| B36/S23 | 0 | 0.05742 | 0 |
| B36/S23 | 0.001 | 0.07148 | 0.12422 |
| B36/S23 | 0.01 | 0.21953 | 0.24805 |
| B36/S23 | 0.05 | 0.35000 | 0.37188 |
| B3678/S045678 | 0 | 0.41621 | 0 |
| B3678/S045678 | 0.001 | 0.23809 | 0.23711 |
| B3678/S045678 | 0.01 | 0.61914 | 0.24199 |
| B3678/S045678 | 0.05 | 0.94551 | 0.57461 |

**Observation:** final mean distance increased across these four levels for each
of these rules. Mean density was not universally monotonic: Conway and the
selected rule first decreased at p=0.001. Such finite-sample changes are not
proof of a phase transition or of motif survival. The five individual outcomes
and traces are in `results/noise-comparison.json`; no confidence intervals or
critical threshold estimates are claimed.

## Three states

B2/S, C=3 used the same five seeds and four noise levels (20 completed trials).
At zero noise, four of the five runs had absorbing active extinction by the end;
the remaining run was unclassified. Mean final active density was 0.001171875.
At p=0.001, 0.01 and 0.05 the means were 0.0458984375, 0.13046875 and
0.198046875. Mean semantic distances were 0.0962890625, 0.255078125 and
0.3935546875. These distances include refractory differences, so they must not
be interpreted as binary active-cell disagreement alone. External reseeding by
noise is a plausible explanation for sustained activity; this is an
interpretation to investigate, not proof of robust self-maintenance.

## Verification and next experiments

The original 14 C++ checks passed before the extension. The expanded C++ suite
and Wasm API/replay tests passed after the extension. HTTP and worker transport
checks are documented with the final verification results. A native C++ compiler
and a connected browser were not available, so native execution and visual,
interactive and accessibility review remain to be done.

Recommended next steps: increase trial count; include several grid sizes and
initial densities; extend runs past 200 steps; inspect the selected rule's
spatial structures; repeat HighLife with deliberately chosen seeds/patterns;
compare C=3/4/6 at matched settings; predefine a robustness criterion and refine
noise levels around any crossover; compute uncertainty from paired trials.
Export actual screenshots or animations after visual inspection. Object-level
tracking is needed before making claims about motion, reproduction or ecology.
