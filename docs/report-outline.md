> Historical v1 document. Superseded by [v2 methodology](methodology-v2.md) and the [complete report](../report/main.tex). Do not combine v1 and v2 noise results.

# Report outline (not a complete report)

1. System architecture and controls: C++ engine, embind/Wasm, canvas, worker; drawing, playback, reset, seeds and experiment replay.
2. Editable rules: 18 Boolean choices, B/S masks, parsing/validation, synchronous update invariants, Conway/HighLife comparison.
3. Methodology: 100 distinct sampled rules, repeated random initial configurations, seeds, toroidal boundary, dimensions/density/steps, exports and measurement definitions.
4. Sample classification and evidence: per-trial categories, category mixtures, density/change charts, finite-horizon limitations; use recorded data, not labels as proof.
5. Interesting rule: investigate B3678/S045678; replay its fixed/period-4/period-2 outcomes, extend seeds and horizons, examine spatial structures and sensitivity to density.
6. Noise investigation: perturbation semantics, paired deterministic reference, robustness criteria, noise sweep and uncertainty; separate crossovers from demonstrated thresholds.
7. Multi-state investigation: Generations transition table, C=2 compatibility, B2/S/C3 results, refractory-state rendering, hypotheses and tests for C=4 and C=6.
8. Technical/UI decisions: byte representation, age metadata, validation, rendering accessibility, worker cancellation, computational limits and tests.
9. Verification and artificial-life interpretation: use the draft section in verification-section.md, clearly separating formal, empirical, philosophical and speculative claims.
10. AI usage: AI assisted design, implementation, debugging, tests, experiment execution and documentation. Explain reviewed decisions, verification performed, and the remaining human responsibility for understanding and interpreting behavior.
11. Limitations and future work: sampling bias, small trials/grids/horizons, heuristic disorder, absent motion/reproduction detectors, missing visual browser QA, uncertainty analysis and broader parameter sweeps.

Evidence sources within the repository: `results/*.json`, `results/summary.json`, `docs/experiments.md`, tests and reproducible scripts. Add real screenshots/animations from browser replay after visual review. Do not claim unobserved HighLife replicators or self-reproducing species.
