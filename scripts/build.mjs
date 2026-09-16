import { spawnSync } from "node:child_process";
import { existsSync, mkdirSync } from "node:fs";
const compiler =
  process.env.EMXX ||
  (existsSync("C:/emsdk/upstream/emscripten/em++.exe")
    ? "C:/emsdk/upstream/emscripten/em++.exe"
    : "em++");
const test = process.argv.includes("--test");
mkdirSync("build", { recursive: true });
const sources = [
  "cpp/engine.cpp",
  "cpp/experiments.cpp",
  test ? "cpp/tests.cpp" : "cpp/wasm_bindings.cpp",
];
const args = [
  ...sources,
  "-std=c++17",
  "-O2",
  "-Wall",
  "-Wextra",
  "-fexceptions",
  "-sALLOW_MEMORY_GROWTH=1",
  ...(test
    ? ["-sENVIRONMENT=node", "-o", "build/tests.cjs"]
    : [
        "--bind",
        "-sMODULARIZE=1",
        "-sEXPORT_ES6=1",
        "-sENVIRONMENT=web,worker,node",
        "-o",
        "life_engine.js",
      ]),
];
const result = spawnSync(compiler, args, {
  stdio: "inherit",
  env: { ...process.env, EMCC_CORES: "4" },
});
if (result.error) throw result.error;
if (result.status) process.exit(result.status);
if (test) {
  const tests = spawnSync(process.execPath, ["build/tests.cjs"], {
    stdio: "inherit",
  });
  process.exit(tests.status ?? 1);
}
