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

## Compile the production website

Install Node.js and the Emscripten SDK and activate its environment. Install
CMake 3.31.6 and Ninja 1.11.1.4. On Windows the build also detects `C:/emsdk`
and local tools installed with:

```sh
python -m pip install --target build/tools cmake==3.31.6 ninja==1.11.1.4
npm ci
npm run build
npm run check
npm run check:deployment
npm run serve:production
```

Open http://127.0.0.1:4173/life-cellular-playground/.
`npm run build` (also `npm run build:site`) compiles C++ to WebAssembly, runs
its tests, and stages six static files in `build/wasm/site` for hosting.
`npm test` rebuilds and runs the same C++ tests. `npm start` previews the root
using its checked-in engine artifacts.

For browser tests, run `npx playwright install chromium`, then
`npm run test:production`. On Windows with Edge installed, use
`$env:PLAYWRIGHT_CHANNEL='msedge'; npm.cmd run test:production`.
Tests cover Wasm, simulation controls, drawing, direct loads and refreshes at
both `/` and `/life-cellular-playground/`.

The GitHub Pages workflow builds and tests the same artifact. To publish, set
repository Settings > Pages > Source to GitHub Actions and push to `main`.
The current source implements the Conway playground; the removed rule laboratory
and experiment runner are not part of this website build.

Release is the default build type. Use `npm run build:debug` for a development
build (or set `BUILD_TYPE`). On this Windows machine, Application Control blocks
the SDK optimizer `wasm-opt.exe`, so use the Debug build locally. Release builds
require an environment where that tool is permitted to run.
