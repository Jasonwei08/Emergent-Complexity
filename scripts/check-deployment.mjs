import assert from "node:assert/strict";
import { readFileSync, readdirSync, lstatSync } from "node:fs";
import { spawnSync } from "node:child_process";
import { parseDocument } from "yaml";
const document = parseDocument(
  readFileSync(".github/workflows/pages.yml", "utf8"),
  { uniqueKeys: true },
);
assert.deepEqual(
  document.errors,
  [],
  "Workflow must parse without duplicate keys",
);
const workflow = document.toJS();
assert.deepEqual(workflow.on.push.branches, ["main"]);
assert.ok(Object.hasOwn(workflow.on, "workflow_dispatch"));
assert.equal(workflow.jobs.deploy.needs, "build");
assert.equal(workflow.jobs.deploy.permissions.pages, "write");
assert.equal(workflow.jobs.deploy.permissions["id-token"], "write");
assert.equal(workflow.jobs.deploy.environment.name, "github-pages");
assert.match(workflow.jobs.deploy.if, /pull_request/);
const steps = Object.values(workflow.jobs).flatMap((job) => job.steps);
for (const step of steps.filter((step) => step.uses))
  assert.match(
    step.uses,
    /@[a-f0-9]{40}$/,
    "Actions must use immutable commits",
  );
const upload = steps.find((step) =>
  step.uses?.startsWith("actions/upload-pages-artifact@"),
);
assert.equal(upload.with.path, "build/wasm/site");
const expected = [
  ".nojekyll",
  "index.html",
  "style.css",
  "app.js",
  "lab.js",
  "experiment-worker.js",
  "life_engine.js",
  "life_engine.wasm",
].sort();
assert.deepEqual(
  readdirSync(upload.with.path).sort(),
  expected,
  "Only static runtime assets may be deployed",
);
for (const file of expected) {
  const path = `${upload.with.path}/${file}`;
  assert.ok(lstatSync(path).isFile() && !lstatSync(path).isSymbolicLink());
  if (file.endsWith(".js"))
    assert.equal(
      spawnSync(process.execPath, ["--check", path]).status,
      0,
      path,
    );
}
assert.deepEqual(
  [...readFileSync(`${upload.with.path}/life_engine.wasm`).subarray(0, 4)],
  [0, 97, 115, 109],
);
for (const file of ["index.html", "app.js", "lab.js", "experiment-worker.js"]) {
  const text = readFileSync(`${upload.with.path}/${file}`, "utf8");
  assert.doesNotMatch(
    text,
    /(?:src|href)=["']\/|(?:from|import)\s*["']\/|new URL\(["']\//,
    `${file}: root-relative asset path`,
  );
}
console.log(
  "Workflow YAML, pinned actions, permissions, deployment directory and staged asset checks passed.",
);
