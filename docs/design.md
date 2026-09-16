> Historical v1 document. Superseded by [v2 methodology](methodology-v2.md) and the [complete report](../report/main.tex). Do not combine v1 and v2 noise results.

# Design and experiment methodology

## Architecture and scope

The original C++ engine remains the simulation source of truth. `engine.cpp` implements rules, seeded initialization, synchronous toroidal updates and perturbations. `experiments.cpp` calls that engine and computes measurements; it has no browser dependency. Embind copies cell buffers into JavaScript typed arrays. The existing canvas and playback remain in `app.js`; `lab.js` adds controls and result presentation. A module worker runs C++ experiments without blocking drawing or playback. The small preview server explicitly serves the added assets.

The original pattern catalogue, binary age coloring, pointer/keyboard drawing, play/pause, step, speed, randomize, clear and reset remain. No simulation has been ported to JavaScript. Native CMake targets include the experiment module. Node-based build/test scripts also permit running the same C++ tests through Wasm where no native compiler is installed.

## Editable-rule mathematics and invariants

For a binary Moore-neighborhood outer-totalistic rule, the local input is `(a,n)`, where `a` is 0 or 1 and `n` is one of 0..8. There are 2*9=18 inputs. Choosing one Boolean output independently for each gives 2^18=262,144 rules. Two nine-bit masks store these outputs:

`f(0,n) = (birth >> n) & 1`; `f(1,n) = (survival >> n) & 1`.

Birth and survival are separate even when their neighbor counts overlap. B3/S23 and B36/S23 differ only in birth at six neighbors. C++ accepts unique digits, sorts notation on output, and rejects malformed syntax, duplicates and counts outside 0..8. The UI additionally trims outside whitespace and uppercases text before validation. B/S (both sets empty) and B0 are valid. Invalid text leaves the active rule untouched. Checkboxes, notation and the rule badge are synchronized after successful changes.

The synchronous update reads only the previous buffer. Dimensions must be 3..512, avoiding ambiguous repeated Moore neighbors on one- or two-cell toroidal axes; buffers must have the matching size. Binary positive bytes represent active cells and preserve ages up to 255; ages never affect transitions or recurrence detection. Births/deaths measure net active-state changes after noise, not separate deterministic/noise events. Conway resets to binary B3/S23 with zero noise; HighLife sets its B/S masks in the visible state model. Pattern behavior descriptions refer to standard binary Conway; they are not promises under edited rules.

## Multi-state family

For C=3..8, bytes are states 0..C-1. State 0 is empty, 1 is active, and 2..C-1 are refractory. Count only neighboring state-1 cells. The complete deterministic rule is:

- Empty 0 becomes 1 if the count is in B, otherwise remains 0.
- Active 1 remains 1 if the count is in S, otherwise becomes 2.
- Refractory k advances to k+1; C-1 becomes 0. Refractory cells ignore B and S.

C=2 explicitly selects the original binary/age interpretation; it is not a refractory model. Switching state counts preserves active occupancy and clears refractory states, normalizes active bytes to 1, pauses, and starts a new reset checkpoint. This prevents age bytes becoming unintended states. B2/S with C=3 provides a simple excitable example: two active neighbors cause birth, activity lasts one step, followed by one refractory step. No randomness is needed unless initialization or noise is enabled.

One byte per cell supports both interpretations without expanding the existing representation. Active cells use solid light-green squares; refractory states use distinct colors and hollow centers, plus state numbers when cells are large enough. The labeled brush includes every state, including erasing with 0. Keyboard traversal announces coordinates and the exact state, so inspecting a state does not depend on color. Binary drawing retains click-to-toggle behavior. Palette distinctions at very small cell sizes still need human accessibility review.

**Design hypothesis:** refractory delays may suppress immediate rebirth and support traveling activity fronts. Longer refractory times and sparse birth conditions are reasonable design variables; neither guarantees mobility or complexity. Compare C=3,4,6 at matched seeds and densities before claiming a principle.

## Sampling, seeds and preservation

The UI samples 100 distinct binary masks by taking the high 18 bits of a 32-bit linear congruential generator (LCG) and rejecting duplicates. This is pseudorandom sampling without replacement, not a claim of ideal independent entropy. The LCG recurrence is `x = 1664525*x + 1013904223 mod 2^32`. Its low bits are not used for rule selection. A probability event occurs when `next() < floor(p*2^32)`, using integer-valued doubles. Thus resolution is 2^-32. Seed zero is valid. This explicit mapping avoids implementation-dependent standard-library probability distributions. LCG correlations remain a limitation for large scientific studies.

Trial t (zero-based) uses `(experimentSeed + (t+1)*2654435761) mod 2^32` for row-major Bernoulli initialization, with only 0/1 initial states. Noise uses a separate stream initialized to `trialSeed XOR 0x9e3779b9`. Rules and noise levels share initial grids and noise seeds per trial, supporting paired comparisons. Different trials have different seeds. No early stopping is used, even after recurrence or an empty generation.

Every export records schema/engine version, algorithm, rules, boundary, dimensions, initial density, state count, noise, steps, trial IDs, both seeds, complete time series, and initial/final cell arrays. The initial arrays permit replay without relying solely on the initializer. `complete:false` distinguishes cancelled partial batches. Results remain in memory until exported or replaced by a new batch. JSON is the preservation format; no database is needed.

The main grid starts at 80x48. Inspect a results row, select a trial, then replay its exact initial buffer, state model, rule and noise seed at the recorded dimensions. Step/play and Reset use that recorded stream. Loading a built-in pattern restores the original dimensions. Changing settings or drawing creates a new trajectory rather than claiming to continue the recorded experiment. For reproducibility across future changes, preserve this code revision alongside exports.

## Measurements and classification (version 1)

Each series includes t=0 through the requested last step. Active density is active count / grid area. Occupied density counts any nonzero state, including refractory states. Change rate is the fraction of semantic cell states differing from the previous step; binary ages are first mapped to 0/1. Initial change is recorded as zero by convention.

`firstEmpty` is the first zero-active generation, or -1 if unobserved. It is NOT necessarily an extinction time: B0 and noise can reactivate an empty grid. The extinction category requires a final zero-active grid, zero noise, and no B0, which makes active extinction absorbing for this family. Remaining refractory states can still decay.

Exact semantic arrays from the preceding 16 steps are compared, nearest first. `terminalPeriod` is the shortest exact recurrence seen at the terminal step, or zero if none is found within that window. For a deterministic transition, identical complete semantic states imply identical futures, conditional on implementation correctness. For noise this is just an observed coincidence and never triggers the stable/oscillatory label. Global recurrence does not track translating objects; wraparound and finite dimensions affect observed periods.

Let W=max(1,floor(steps/4)). `tailChange` averages the last W change rates. `growth` is mean density over the last W generations minus the first W recorded densities (starting at t=0). Ordered categories are:

1. Extinction as defined above.
2. Stable: zero noise and terminalPeriod=1.
3. Oscillatory: zero noise and terminalPeriod=2..16.
4. Growing: growth > 0.10 (absolute density points).
5. Disordered: tailChange > 0.15.
6. Unclassified otherwise.

The priority means an oscillator can also have grown substantially earlier. "Growing" describes finite-window density change, not unlimited growth on a finite torus. "Disordered" means persistent turnover here, not proven chaos, entropy, positive Lyapunov exponent or lack of structure. Categories apply to trials; tables show category counts across trials. No automatic mobility, object recognition or self-reproduction detector is claimed. Bounding-box growth is omitted because a naive box is misleading on a torus.

## Noise and robustness

After each deterministic synchronous update, each cell has perturbation probability p. In binary mode the perturbation flips occupancy. In Generations it sends an active cell to empty and any inactive (empty or refractory) cell to active. This is an explicit intervention, not thermal noise or a biologically calibrated model. In particular, it can bypass refractory delays. A nonzero p consumes one RNG draw per cell; p=0 consumes none and preserves every deterministic output byte, including ages.

For each noisy trial the experiment engine evolves a matched zero-noise reference in parallel. `distance[t]` is the semantic Hamming distance / area between those trajectories. It is identically zero at p=0. Density, occupancy, turnover, first-empty events and distance are available for investigating robustness. Exact-grid distance measures trajectory fidelity, not whether a recognizable motif survived; those are different research questions.

Compare paired differences across several seeds, plot individual trajectories and distributions, and define a robustness criterion before studying a threshold (for example, motif survival at a fixed time, or mean late-window distance exceeding a chosen cutoff). Increase trials, run lengths and grid sizes, and refine noise levels near any apparent crossover. Include uncertainty estimates and test sensitivity to the cutoff. This implementation does not infer a critical noise threshold from four levels or from a single final mean. Nonzero noise can continually reseed activity, so sustained density alone is not evidence of robust organisms.

## Performance and limits

Each deterministic step is O(N) time with eight neighbor lookups and O(N) cell buffers. Noise adds O(N) generator/threshold operations; nonzero-noise experiments also run a reference step. Recurrence keeps at most 16 grids, using O(16N) extra storage and comparisons. Output has O(T) metric storage and O(N) initial/final snapshots per trial. Embind transfers copy arrays; the UI's visual trails are separate and never used by C++.

Experiments execute in a dedicated worker and can be terminated between or during trials; only completed trials are preserved. UI limits are 128x128, 2..20 trials, 1..2000 steps, at most eight distinct noise levels, and 250 million total cell updates (including references). These bounds prevent accidental huge browser jobs, not guarantee a frame rate. Native callers can use dimensions up to 512. Three-state grids use the same bytes as binary grids but add branch logic and richer rendering.

No parallel agents, GPU dependencies, alternate simulation backend, server database, or UI framework were introduced. Potential follow-ups are better RNG families with recorded versioning, object-level motion tracking, uncertainty summaries, import of archived experiments, sparse updates where valid (B0 requires care), and manual browser/accessibility validation.
