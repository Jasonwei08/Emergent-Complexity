import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import createLifeModule from "../life_engine.js";
const Life = await createLifeModule();
assert.equal(Life.validateRule("B63/S32"), "B36/S23");
assert.equal(Life.validateRule("B99/S23"), "");
assert.equal(Life.sampleRules(2026).length, 100);
for (const [rule, states, noise] of [
  ["B3/S23", 2, 0],
  ["B36/S23", 2, 0.01],
  ["B2/S", 3, 0.05],
  ["B0/S", 5, 0],
]) {
  const trial = JSON.parse(
    Life.runTrial(rule, states, noise, 20, 16, 0.2, 50, 123),
  );
  let grid = Uint8Array.from(trial.initial),
    rng = trial.noiseSeed;
  for (let t = 0; t < 50; t++) {
    const next = Life.advance(grid, 20, 16, rule, states, noise, rng);
    grid = next.cells;
    rng = next.rngState;
  }
  assert.deepEqual(
    Array.from(grid),
    trial.final,
    `${rule}: exact UI-style replay`,
  );
  assert.deepEqual(
    Array.from(Life.randomGrid(20, 16, 0.2, 123)),
    trial.initial,
  );
}
for(const states of [2,3,5,8]) {
 const r=JSON.parse(Life.runTrialAll("B2/S",states,.001,8,8,.5,10,123,true));
 assert.equal(r.schema,"life-trial-v2");assert.equal(r.boundary,"toroidal");
 for(const key of ["density","occupied","births","deaths","change","hashes","populations","spread","movement"]) assert.equal(r[key].length,11);
 for(const counts of r.populations) {assert.equal(counts.length,states);assert.equal(counts.reduce((a,b)=>a+b),64);}
 let grid=Uint8Array.from(r.initial),seed=r.noiseSeed;
 for(let i=0;i<10;i++){const n=Life.advance(grid,8,8,r.rule,states,r.noise,seed);grid=n.cells;seed=n.rngState;}
 assert.deepEqual(Array.from(grid),r.final);
}
const pattern = Life.createPattern("blinker");
assert.deepEqual(
  Life.evolve(pattern).cells,
  Life.advance(pattern, 80, 48, "B3/S23", 2, 0, 42).cells,
);
assert.ok(
  readFileSync(new URL("../life_engine.wasm", import.meta.url)).length > 0,
);
console.log(
  "Wasm bindings: parsing, sampling, random initialization, legacy compatibility and four exact replays passed.",
);

// Exercise the actual worker module with Node's standard worker transport.
const { Worker } = await import("node:worker_threads");
const { mkdirSync, writeFileSync } = await import("node:fs");
mkdirSync("build", { recursive: true });
const bridge = new URL("../build/worker-bridge.mjs", import.meta.url);
writeFileSync(
  bridge,
  `import {parentPort} from 'node:worker_threads';
globalThis.self={postMessage:data=>parentPort.postMessage(data)};
await import('../experiment-worker.js');
parentPort.on('message',data=>self.onmessage({data}));
parentPort.postMessage({ready:true});`,
);
const worker = new Worker(bridge);
try {
  await new Promise((resolve, reject) => {
    const timer = setTimeout(
      () => reject(Error("Worker test timed out")),
      15000,
    );
    const runs = [];
    worker.on("error", (error) => {
      clearTimeout(timer);
      reject(error);
    });
    worker.on("message", (data) => {
      try {
        if (data.ready)
          worker.postMessage({
            rules: ["B3/S23", "B2/S"],
            config: {
              seed: 2026,
              cols: 12,
              rows: 12,
              density: 0.2,
              steps: 20,
              trials: 2,
              states: 2,
              noiseLevels: [0, 0.01],
            },
          });
        if (data.error) throw Error(data.error);
        if (data.result) {
          assert.equal(data.total, 8);
          runs.push(data.result);
        }
        if (data.done) {
          assert.equal(runs.length, 8);
          for (const run of runs) {
            const direct = JSON.parse(
              Life.runTrial(
                run.rule,
                run.states,
                run.noise,
                run.cols,
                run.rows,
                run.initialDensity,
                run.steps,
                run.seed,
              ),
            );
            assert.deepEqual({ ...direct, trial: run.trial, control: false, ruleIndex: run.ruleIndex }, run);
          }
          clearTimeout(timer);
          resolve();
        }
      } catch (error) {
        clearTimeout(timer);
        reject(error);
      }
    });
  });
} finally {
  await worker.terminate();
}

const { spawn } = await import("node:child_process");
const server = spawn(process.execPath, ["server.mjs"], {
  env: { ...process.env, PORT: "4187" },
  stdio: ["ignore", "pipe", "pipe"],
});
try {
  await new Promise((resolve, reject) => {
    const timer = setTimeout(
      () => reject(Error("Preview server timed out")),
      10000,
    );
    server.once("error", (error) => {
      clearTimeout(timer);
      reject(error);
    });
    server.once("exit", (code) => {
      clearTimeout(timer);
      reject(Error(`Preview server exited: ${code}`));
    });
    server.stdout.once("data", () => {
      clearTimeout(timer);
      resolve();
    });
  });
  for (const asset of [
    "",
    "app.js",
    "lab.js",
    "experiment-worker.js",
    "style.css",
    "life_engine.js",
    "life_engine.wasm",
  ]) {
    const response = await fetch(`http://127.0.0.1:4187/${asset}`);
    assert.equal(response.status, 200, asset);
    assert.ok((await response.arrayBuffer()).byteLength > 0);
    if (asset.endsWith(".js"))
      assert.match(response.headers.get("content-type"), /javascript/);
    if (asset.endsWith(".wasm"))
      assert.equal(response.headers.get("content-type"), "application/wasm");
  }
  assert.equal(
    (await fetch("http://127.0.0.1:4187/cpp/engine.cpp")).status,
    404,
  );
} finally {
  server.kill();
}
console.log(
  "Actual experiment worker (8 trials) and all preview HTTP assets passed. Browser UI was not exercised.",
);
