// Production packaging extends the native project's CMake configuration.
import { existsSync } from "node:fs";
import { spawnSync } from "node:child_process";
import { resolve } from "node:path";
const buildType = process.argv.includes("--debug") ? "Debug" : process.env.BUILD_TYPE || "Release";
if (!["Release", "Debug", "RelWithDebInfo", "MinSizeRel"].includes(buildType))
  throw Error(`Unsupported BUILD_TYPE: ${buildType}`);
const windows = process.platform === "win32";
const localTool = (name, fallback) => {
  const path = `build/tools/${name}/data/bin/${name}${windows ? ".exe" : ""}`;
  return existsSync(path) ? resolve(path) : fallback;
};
const cmake = process.env.CMAKE || localTool("cmake", "cmake");
const ninja =
  process.env.NINJA ||
  (existsSync("build/tools/bin/ninja.exe")
    ? resolve("build/tools/bin/ninja.exe")
    : localTool("ninja", "ninja"));
const emcmake =
  process.env.EMCMAKE ||
  (existsSync("C:/emsdk/upstream/emscripten/emcmake.exe")
    ? "C:/emsdk/upstream/emscripten/emcmake.exe"
    : "emcmake");
function run(command, args) {
  const result = spawnSync(command, args, {
    stdio: "inherit",
    env: { ...process.env, EMCC_CORES: "4" },
  });
  if (result.error) throw result.error;
  if (result.status !== 0) process.exit(result.status ?? 1);
}
run(emcmake, [
  cmake,
  "-S",
  "cpp",
  "-B",
  "build/wasm",
  "-G",
  "Ninja",
  `-DCMAKE_MAKE_PROGRAM=${ninja}`,
  `-DCMAKE_BUILD_TYPE=${buildType}`,
]);
run(cmake, [
  "--build",
  "build/wasm",
  "--target",
  "site",
  "life_tests",
  "--parallel",
  "4",
]);
run(process.execPath, ["build/wasm/life_tests.cjs"]);
console.log("Production static site: build/wasm/site");
