# Life — a cellular playground

An interactive Conway's Game of Life website. The interface is HTML, CSS, and vanilla JavaScript; the simulation engine itself is C++, compiled to WebAssembly. No runtime dependencies or build step are needed to run the site — the compiled `life_engine.js`/`life_engine.wasm` are checked in.

## Run locally

Install Node.js, then run:

```sh
npm start
```

Open http://127.0.0.1:4173 in your browser. To choose a different port, set the `PORT` environment variable.

## Play

- Draw or erase cells by clicking and dragging on the grid.
- Choose from six classic patterns: Glider, Pulsar, Gosper glider gun, R-pentomino, Lightweight ship, and Blinker.
- Play, pause, advance one generation, adjust speed, randomize, clear, or reset the world.
- Follow the generation count, population, births, and density.
- Use Space to play/pause, N to step, and R to reset. Focus the canvas and use arrow keys and Enter to edit cells with the keyboard.

The grid contains 80 × 48 cells and wraps at its edges. The simulation uses Conway's B3/S23 rules: an empty cell with three neighbors is born, and a living cell with two or three neighbors survives. Fading trails are visual only.

## Files

- `index.html`: page entrypoint and metadata
- `style.css`: responsive visual design
- `app.js`: interface, controls, rendering, and optional WebMCP integration
- `life_engine.js` / `life_engine.wasm`: compiled WebAssembly build of the C++ engine (generated from `cpp/`; see below)
- `server.mjs`: dependency-free local preview server
- `cpp/`: the simulation engine's source of truth — patterns and simulation rules, in C++ (`engine.hpp`/`engine.cpp`), embind bindings for the browser (`wasm_bindings.cpp`), a native CLI demo (`main.cpp`), and tests (`tests.cpp`)

To host the website, serve `index.html`, `style.css`, `app.js`, `life_engine.js`, and `life_engine.wasm` together using any static web host. No credentials, database, or environment secrets are required.

### Rebuilding the engine

After changing anything in `cpp/`, regenerate `life_engine.js`/`life_engine.wasm` with the [Emscripten SDK](https://emscripten.org/):

```sh
em++ cpp/engine.cpp cpp/wasm_bindings.cpp --bind -O2 -std=c++17 \
  -s MODULARIZE=1 -s EXPORT_ES6=1 -s ENVIRONMENT=web \
  -o life_engine.js
```

To build and run the native CLI demo and test suite instead (no browser involved), see `cpp/CMakeLists.txt`.

## Syntax checks

```sh
npm run check
```

Inspired by John Conway's Game of Life (1970).
<<<<<<< HEAD


## Rule laboratory

- Edit birth/survival checkboxes or enter B/S notation. Apply validates the rule; invalid text leaves the previous rule active. Conway restores binary Life with zero noise; HighLife selects B36/S23.
- Select 3..8 states for Generations, or try the B2/S, 3-state preset. State 1 is active; states 2 onward decay through a refractory sequence to 0. Only active neighbors count. Use the state brush or keyboard to draw/inspect states.
- Set noise probability, simulation seed and random initialization density. Noise flips active/inactive status after each synchronous update; zero preserves deterministic behavior. Changing model/noise/seed pauses and makes the current grid the new starting checkpoint. Reset restores that grid and the noise stream.
- In Rule experiments, configure seed, dimensions, density, trials, steps and comma-separated noise levels. Choose 100 sampled binary rules or the current rule/state model. Run, cancel, inspect individual trials, view traces, replay exact grids, and export complete or partial JSON.
- Trial seeds and noise seeds are separate; matched trials share initial grids across rules and noise levels. UI results are held in memory: export before starting a new batch or closing the page.

## Build and verification

Requires Node.js >=18 to serve the checked-in build. Rebuilding/testing C++ requires the Emscripten SDK; no npm runtime packages are required.

```sh
npm run build
npm test
npm run test:integration
npm run check
npm run experiments
npm start
```

`npm test` compiles and executes the dependency-free C++ suite as Wasm under Node.
`npm run test:integration` verifies the distributed Wasm API, exact replay,
worker results and preview-server assets. `npm run experiments` regenerates the
recorded evidence in `results/` (overwrites the four named result files).

For a native toolchain:

```sh
cmake -S cpp -B build/native
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
```

The native `life` CLI still runs the original pattern demo. Native experiments
are available through `life::runTrial`; the browser and Node evidence script
provide the ready-to-use experiment interfaces.

## Design and evidence

- [Design, rule semantics, reproducibility and limitations](docs/design.md)
- [Actual experiment observations and follow-up questions](docs/experiments.md)
- [Compact measured results](results/summary.json); full traces/grids in `results/sampled.json`, `results/noise-comparison.json` and `results/generations.json`
- [Report outline only](docs/report-outline.md)
- [Draft section on verification and artificial life](docs/verification-section.md)

The added source files are `cpp/experiments.hpp/.cpp` (C++ measurements and sampling),
`lab.js` (controls/results), `experiment-worker.js` (batch orchestration), and
`scripts/` (build, validation and recorded experiments). Existing engine,
bindings, canvas, server, styles and tests were extended rather than replaced.

## Change inventory

| Files | Purpose |
| --- | --- |
| `cpp/engine.hpp`, `cpp/engine.cpp` | B/S masks and parsing; seeded grids; noise and Generations transitions; shape/state validation; legacy Conway wrapper |
| `cpp/experiments.hpp`, `cpp/experiments.cpp` | Distinct rule sampling, repeated-trial measurements, recurrence, heuristic labels and JSON serialization |
| `cpp/wasm_bindings.cpp`, `cpp/CMakeLists.txt` | Expose the additions to Wasm and include the experiment module in native builds |
| `app.js` | Connect the existing canvas/playback to configurable C++; state drawing, rendering, announcements and exact replay |
| `lab.js`, `experiment-worker.js` | Rule/noise/state controls, experiment configuration, worker batches, charts, inspection, cancellation and export |
| `style.css`, `server.mjs` | Lab styling/accessibility and serving new modules |
| `cpp/tests.cpp`, `scripts/integration-test.mjs` | C++ regression/semantics tests and Wasm/replay/worker/HTTP integration checks |
| `scripts/build.mjs`, `package.json`, `.gitignore` | Repeatable build/test commands and ignored temporary build products |
| `scripts/representative-experiments.mjs` | Regenerate the recorded 380-trial sample and comparison evidence |
| `life_engine.js`, `life_engine.wasm` | Rebuilt distributable C++ engine |
| `results/sampled.json`, `results/noise-comparison.json`, `results/generations.json`, `results/summary.json` | Actual reproducible experiment outputs |
| `README.md`, `docs/design.md`, `docs/experiments.md` | Instructions, mathematical/design rationale, measurement definitions, actual findings and limitations |
| `docs/report-outline.md`, `docs/verification-section.md` | Report outline and the requested substantive draft section only |

Verification in this environment: Emscripten build passed; original and expanded
C++ suites passed; exact replay for four rule/state/noise combinations passed;
actual worker execution (eight additional integration trials) passed; all site
assets returned HTTP 200 with correct JavaScript/Wasm content types; JavaScript
syntax and formatting/whitespace checks passed. The new JavaScript modules were
formatted with Prettier 3.6.2 without adding an npm runtime dependency. Native compilation was not available. The later production deployment work added
real headless browser tests; see the deployment verification below.


## Static production build and GitHub Pages

The production build extends `cpp/CMakeLists.txt` with an Emscripten-only
`life_web` target and a `site` staging target. The native library/demo/tests
remain intact. `npm run build:site` configures CMake with `emcmake`, builds the
Wasm application, runs the C++ tests under Node, and stages exactly eight files
in **`build/wasm/site`**: the HTML, CSS, UI and worker modules, generated JS/Wasm,
and `.nojekyll`. This directory is the Pages artifact, not the repository root.

C++ still owns rule parsing, transitions, sampling and measurements. The small
embind interface is shared by the canvas and worker. JavaScript handles controls,
rendering, orchestration and JSON downloads. No server-side C++, database,
private configuration, or persistent backend is needed. The local Node server
is a development tool; it is not included in the deployed site.

HTML references are relative, UI imports resolve beside their modules, the
worker URL uses `import.meta.url`, and Emscripten resolves its Wasm companion
beside the generated loader. No build-time repository-name replacement is
needed. `/repository-name/` and `/repository-name/index.html` are ordinary
static page loads; refresh needs no SPA rewrite. There are no application routes
beneath the page. The local preview also redirects `/repository-name` to its
trailing-slash directory URL.

The existing tracked root `life_engine.js`/`.wasm` are retained for the original
`npm start` no-build workflow. Pages always compiles from C++ and never uses those
copies. New production artifacts are under the already ignored `build/` and
should not be committed. The npm lockfile pins test-only dependencies; no
third-party npm code is bundled into the website.

### Install/activate tools: Windows PowerShell

From the repository directory, for a new local SDK installation:

```powershell
git clone --branch 6.0.9 --depth 1 https://github.com/emscripten-core/emsdk.git build/emsdk
.\build\emsdk\emsdk.ps1 install 6.0.9
.\build\emsdk\emsdk.ps1 activate 6.0.9
.\build\emsdk\emsdk_env.ps1
python -m pip install --target build/tools cmake==3.31.6 ninja==1.11.1.4
npm ci
```

For the already installed SDK at `C:/emsdk`, instead use:

```powershell
& C:/emsdk/emsdk.ps1 install 6.0.9
& C:/emsdk/emsdk.ps1 activate 6.0.9
& C:/emsdk/emsdk_env.ps1
```

Run the environment activation again in a new terminal. The build wrapper also
recognizes the existing Windows SDK and the tools installed in `build/tools`.
`EMCMAKE`, `CMAKE`, and `NINJA` may optionally override executable paths.

### Install/activate tools: Linux or macOS

With Git, Python and Node installed, from the repository directory:

```sh
git clone --branch 6.0.9 --depth 1 https://github.com/emscripten-core/emsdk.git build/emsdk
./build/emsdk/emsdk install 6.0.9
./build/emsdk/emsdk activate 6.0.9
source build/emsdk/emsdk_env.sh
python3 -m venv build/tool-venv
source build/tool-venv/bin/activate
python -m pip install cmake==3.31.6 ninja==1.11.1.4
npm ci
```

### Build, serve and test locally

These commands work from the repository root:

```sh
npm run build:site
npm run check:deployment
npm run serve:production
```

Open **http://127.0.0.1:4173/life-cellular-playground/**. To simulate any other
repository name, or to serve at `/`, use one of:

```sh
node server.mjs --dir build/wasm/site --base /my-repository/ --port 4173
node server.mjs --dir build/wasm/site --port 4173
```

Always serve Wasm over **HTTP or HTTPS**. Do not double-click `index.html` or use
`file://`; browser module loading, workers and Wasm fetching require an HTTP
origin. `npm start` still serves the root development files; after changing C++,
use `npm run build` for that legacy preview, or `npm run build:site` plus the
production server to test the CI build. Re-run `build:site` after UI changes to
restage the source files.

For browser tests, install the pinned Playwright Chromium revision and run:

```sh
npx playwright install chromium
npm run test:production
```

On Linux CI use `npx playwright install --with-deps chromium`. On a Windows
machine with Edge already installed, the equivalent local smoke run is:

```powershell
$env:PLAYWRIGHT_CHANNEL = 'msedge'
npm run test:production
```

The tests start and stop their own production HTTP servers on ports 4188/4189.
They exercise both root and repository-subpath URLs, direct loads, refresh,
real Wasm loading, drawing, playback, B/S validation, noise/reset, Generations
states, the 100-rule worker batch, exact-seed comparison, inspection/replay and
browser JSON downloads. The workflow validator parses YAML, checks action pins,
permissions and the artifact path, and validates the staged file allowlist,
JavaScript syntax and Wasm magic bytes. It cannot verify repository Pages
settings or execute the GitHub-hosted deployment service locally.

### Enable and trigger GitHub Pages

1. Commit and push these source/configuration changes (including
   `package-lock.json` and `.github/workflows/pages.yml`) to `main`. Do not add
   `build/` or `node_modules/`.
2. In the GitHub repository, open **Settings > Pages > Build and deployment >
   Source** and select **GitHub Actions**. Enable Actions if disabled by repository
   or organization policy. No personal access token or custom secret is needed;
   the workflow uses the automatically supplied `GITHUB_TOKEN`.
3. Open **Actions > Build and deploy Pages > Run workflow**, select `main`, and
   run it. Later pushes to `main` trigger it automatically. Pull requests run
   build/test jobs but never deploy. If your default branch has another name,
   update the workflow's two branch filters and deployment condition.
4. If the `github-pages` environment has approval or branch restrictions, allow
   deployment from `main` and approve the deployment when prompted.
5. After the deploy job succeeds, visit its reported `page_url`, normally
   `https://<username>.github.io/<repository-name>/`. Confirm refresh, experiments
   and a JSON download at the public URL.

With the GitHub CLI authenticated, the optional equivalents are (replace
`USERNAME` and `REPOSITORY`; these commands are for you to run):

```sh
gh api --method POST repos/USERNAME/REPOSITORY/pages -f build_type=workflow
# If Pages already exists, update it instead of POST:
gh api --method PUT repos/USERNAME/REPOSITORY/pages -f build_type=workflow
gh workflow run pages.yml --ref main --repo USERNAME/REPOSITORY
gh run list --workflow pages.yml --repo USERNAME/REPOSITORY
gh api repos/USERNAME/REPOSITORY/pages --jq .html_url
```

The workflow pins official actions to immutable commits, Node to 22.16.0,
Emscripten to 6.0.9 (including SDK commit), CMake to 3.31.6, Ninja to 1.11.1.4,
and browser-test packages via the lockfile. The Ubuntu runner image and system
browser dependencies can still change; this is not a byte-for-byte hermetic
build. The build job uploads `build/wasm/site`; the deployment job alone receives
`pages: write` and `id-token: write`. No live deployment was performed or public
Pages URL verified during this work.

References: [GitHub custom Pages workflows](https://docs.github.com/en/pages/getting-started-with-github-pages/using-custom-workflows-with-github-pages),
[Emscripten CMake builds](https://emscripten.org/docs/compiling/Building-Projects.html),
and [Playwright installation](https://playwright.dev/docs/intro), and
[GitHub Pages settings API](https://docs.github.com/en/rest/pages/pages).


### Deployment verification performed

- Production CMake/Emscripten Release build and the complete C++ suite: passed.
- Actual production files served over HTTP at `/` and
  `/life-cellular-playground/`: passed.
- Eight headless Edge browser smoke tests (four at each base path): passed,
  including Wasm fetch/initialization, Task 2-4 controls, 100-rule experiment
  workers, trial replay, reproducibility and actual JSON downloads.
- Existing Wasm/worker/HTTP integration tests: passed after the server changes.
- Workflow YAML parsing, action commit pins, deployment permissions, stage
  allowlist, JavaScript checks and Wasm header: passed.
- Test-only dependency audit: zero reported vulnerabilities at installation.

The browser tests exposed and now guard against an experiment-worker startup
race: the initial message could arrive while top-level Wasm initialization was
still awaited. The worker now registers its listener synchronously and awaits
the module inside the message handler. Simulation behavior remains in C++.
The earlier Node worker bridge had waited for readiness and did not expose
this browser-specific startup sequence.

Deployment-specific changes: `cpp/CMakeLists.txt` and `scripts/build-site.mjs`
add production compilation/staging; `server.mjs` supports explicit static roots
and URL prefixes; `.github/workflows/pages.yml` builds/tests/deploys;
`scripts/check-deployment.mjs`, `playwright.config.mjs` and
`tests/production.spec.mjs` validate deployment; `package.json` and
`package-lock.json` define commands and pinned test-only dependencies;
`experiment-worker.js` fixes startup; this README documents setup and handoff.
The new `build/wasm/site` output is ignored. Existing generated root artifacts
were not changed for this deployment extension.

The workflow has been prepared and locally validated, but the GitHub-hosted
Linux job and public Pages URL have not been executed or verified here. Local
browser tests used installed Edge; CI is configured to install the Chromium
revision pinned by Playwright. Manual visual/accessibility review on other
browsers remains useful.
=======
>>>>>>> parent of 7ffab3f (added both latter parts, used Codex to implement part B)
