# Life cellular playground

An editable cellular-automaton laboratory with a C++ simulation core compiled to WebAssembly. The site runs entirely in the browser.

## Current features

- Binary Conway Life B3/S23 and HighLife B36/S23 retain original occupancy behavior.
- Explicit Generations states C=3..8; presets Brian's Brain B2/S/C3 and refractory Life B3/S23/C5; editable birth/survival masks and optional /C notation.
- State painting, keyboard inspection, ordered decay colors, hollow refractory marks and state numerals at sufficient zoom.
- Seeded active-only or all-state initialization, randomize/reset, and precisely defined post-update noise: binary flip or uniformly selected different Generations state.
- Paired batch trials with default 64x64 torus, 500 updates, densities 0.10/0.30/0.50 and five trials per density. Exactly 100 sampled rules plus absent Life/HighLife controls kept separate.
- Per-state populations, births/deaths, activity, spatial and mobility proxies, full-equality-confirmed recurrence, trial categories, explicit interest ranking in the evidence runner, JSON download and saved-trial import/replay.

## Run and verify

```sh
npm start
npm test
npm run build
npm run test:integration
npm run check
npm run experiments
npm run build:site
npm run check:deployment
npm run test:production
```

Use `npm.cmd` on Windows if PowerShell blocks npm.ps1. `npm start` serves the root preview at http://127.0.0.1:4173. After C++ edits, `npm run build` regenerates its root loader/Wasm. Production compilation uses CMake and stages `build/wasm/site`. C++ tests execute as Wasm under Node; native CMake targets remain available in `cpp/CMakeLists.txt`.

The core resides in `cpp/engine.cpp` and `cpp/experiments.cpp`; embind exposes it through `cpp/wasm_bindings.cpp`. `app.js` renders the grid, `lab.js` provides the laboratory, and `experiment-worker.js` runs batches away from the UI thread.

## Report and evidence

- Latest supplied report: `Emergent_Complexity.zip`. Run `npm run build:report` to stage its LaTeX, PDF and figures in `build/report/`.
- [v2 methodology and measurement definitions](docs/methodology-v2.md).
- [Measured summaries and rankings](results/v2/summary.json), [sampled rule list](results/v2/sampled-rules.json).
- Full time series and grids: `results/v2/sampled.json`, `controls.json`, `detail.json`, `generations.json`. The recorded campaign has 2880 trials, master seed 2026. All raw files are JSON; the largest is approximately 98 MB.
- Selected-rule replay files: `results/v2/replay-*.json`. Use **Reopen exported JSON**, select a trial, and click **Replay exact initial grid**.
- [Verification and limitations](docs/verification-v2.md).

Stage and verify the supplied report without regenerating its edited text or figures:

```sh
npm run build:report
python -m pip install --target build/report-python PyMuPDF Pillow
npm run verify:report
```

The build defaults to the root `Emergent_Complexity.zip`; an alternative archive
can be passed with `npm run build:report -- path/to/report.zip`. The archive must
contain `report/main.tex`, `report/main.pdf`, and all referenced inputs and figures.
The build records archive and file SHA-256 hashes in `build/report-manifest.json`.
PDF review images and fresh verification results go to `build/report-review/`;
verification metadata supplied inside the ZIP is preserved. The root archive has
a citation but no bibliography declaration; staging reports this source warning.
The supplied PDF is copied unchanged. Its title differs from the edited LaTeX;
verification flags this mismatch. To recompile edited LaTeX, run Tectonic
from `build/report/`. The report remains separate from the production site.

Regenerate the campaign with `npm run experiments`, then `python scripts/generate-report.py` (requires matplotlib). Compile from `report/`:

```sh
pdflatex main
bibtex main
pdflatex main
pdflatex main
```

Alternatively run `tectonic --keep-logs main.tex`. The verified local compiler is Tectonic 0.15.0 (XeTeX); pdflatex was not installed. `python scripts/verify-report.py` uses PyMuPDF and Pillow to render pages and check references. Optional local packages were installed in ignored `build/report-tools`; normal installed Python packages also work.

Historical v1 data and documents are retained separately. Their smaller experiments, noise model and outline-only report are superseded by v2 and must not be mixed with it. The production Pages artifact intentionally excludes research exports; download them from the repository for import.

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
