import { test, expect } from "@playwright/test";
import { readFile } from "node:fs/promises";

test.beforeEach(async ({ page, baseURL }) => {
  const failures = [];
  page.on("pageerror", (error) => failures.push(error.message));
  page.on("response", (response) => {
    if (response.status() >= 400)
      failures.push(`${response.status()} ${response.url()}`);
  });
  page.on("request", (request) => {
    if (request.url().startsWith("http"))
      expect(request.url().startsWith(baseURL)).toBeTruthy();
  });
  page.__failures = failures;
  const wasm = page.waitForResponse(
    (response) => response.url() === `${baseURL}life_engine.wasm`,
  );
  await page.goto(baseURL);
  expect((await wasm).status()).toBe(200);
  await expect(page.locator("#rule-text")).toHaveValue("B3/S23");
});
test.afterEach(async ({ page }) => expect(page.__failures).toEqual([]));

test("direct loads, refresh and original controls", async ({
  page,
  baseURL,
}) => {
  await page.reload();
  await expect(page.locator("#rule-text")).toHaveValue("B3/S23");
  await page.goto(`${baseURL}index.html`);
  await page.locator('[data-pattern="blinker"]').click();
  await expect(page.locator("#population")).toHaveText("3");
  await page.locator("#step").click();
  await expect(page.locator("#generation")).toHaveText("0001");
  await page.locator("#reset").click();
  await expect(page.locator("#generation")).toHaveText("0000");
  await page.locator("#speed").fill("30");
  await expect(page.locator("#speed-output")).toHaveText("30 gen/s");
  await page.locator("#play").click();
  await expect(page.locator("#generation")).not.toHaveText("0000");
  await page.locator("#play").click();
  await expect(page.locator("#status-label")).toHaveText("PAUSED");
  await page.locator("#random").click();
  await expect(page.locator("#population")).not.toHaveText("0");
  await page.locator("#clear").click();
  await expect(page.locator("#population")).toHaveText("0");
  await page.locator("#grid").focus();
  await page.keyboard.press("Enter");
  await expect(page.locator("#population")).toHaveText("1");
  await page.locator("#grid").click({ position: { x: 30, y: 30 } });
  await expect(page.locator("#population")).toHaveText("2");
});

test("editable B/S, noise, state transitions and reset", async ({ page }) => {
  await page.locator("#highlife").click();
  await expect(page.locator("#rule-text")).toHaveValue("B36/S23");
  await expect(page.locator('[data-count="B6"]')).toBeChecked();
  await page.locator('[data-count="B6"]').uncheck();
  await expect(page.locator("#rule-text")).toHaveValue("B3/S23");
  await page.locator("#rule-text").fill("B9/S23");
  await page.locator("#rule-form button").click();
  await expect(page.locator("#rule-text")).toHaveAttribute(
    "aria-invalid",
    "true",
  );
  await expect(page.locator(".rule-badge")).toHaveText("B3/S23");
  await page.locator("#conway").click();
  await page.locator("#clear").click();
  await page.locator("#noise").fill("1");
  await page.locator("#noise").press("Tab");
  await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText("3,840");
  await page.locator("#reset").click();
  await expect(page.locator("#population")).toHaveText("0");
  await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText("3,840");
  await page.locator("#conway").click();
  await expect(page.locator("#noise")).toHaveValue("0");
  await page.locator("#brain").click();
  await expect(page.locator("#states")).toHaveValue("3");
  await page.locator("#clear").click();
  await page.locator("#brush").selectOption("1");
  await page.locator("#grid").focus();
  await page.keyboard.press("Enter");
  await expect(page.locator("#population")).toHaveText("1");
  await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText("0");
  await page.locator("#grid").focus();
  await page.keyboard.press("ArrowRight");
  await page.keyboard.press("ArrowLeft");
  await expect(page.locator("#announcement")).toContainText("state 2");
  await page.locator("#step").click();
  await page.locator("#grid").focus();
  await page.keyboard.press("ArrowRight");
  await page.keyboard.press("ArrowLeft");
  await expect(page.locator("#announcement")).toContainText("state 0");
  await page.locator("#brush").selectOption("2");
  await page.locator("#grid").focus();
  await page.keyboard.press("Enter");
  await expect(page.locator("#announcement")).toContainText("state 2");
});

async function smallBatch(page) {
  await page.locator("#experiment-cols").fill("12");
  await page.locator("#experiment-rows").fill("12");
  await page.locator("#additional-densities").fill("");
  await page.locator("#experiment-trials").fill("2");
  await page.locator("#experiment-steps").fill("20");
}
async function exported(page) {
  const pending = page.waitForEvent("download");
  await page.locator("#export-experiment").click();
  const download = await pending;
  expect(download.suggestedFilename()).toMatch(/\.json$/);
  return JSON.parse(await readFile(await download.path(), "utf8"));
}
test("100 sampled rules: worker, inspection, replay and JSON download", async ({
  page,
}) => {
  await smallBatch(page);
  await page.locator("#run-experiment").click();
  await expect(page.locator("#experiment-status")).toContainText(
    "Completed 204 trials",
  );
  await expect(page.locator("#results tr")).toHaveCount(102);
  const data = await exported(page);
  expect(data.complete).toBe(true);
  expect(new Set(data.rules).size).toBe(100);
  expect(data.results.filter(r=>!r.control)).toHaveLength(200);
  expect(data.results[0].density).toHaveLength(21);
  await page.locator("#results button").first().click();
  await expect(page.locator("#trial-chart svg")).toBeVisible();
  await page.locator("#replay").click();
  await expect(page.locator(".grid-dimensions")).toHaveText("12 x 12");
  const trial = data.results[0];
  await expect(page.locator("#population")).toHaveText(
    String(trial.initial.filter((c) => c > 0).length),
  );
  for (let i = 0; i < trial.steps; i++) await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText(
    String(trial.final.filter((c) => c > 0).length),
  );
});

test("multi-state noise comparison exports reproducible trials", async ({
  page,
}) => {
  await page.locator("#brain").click();
  await smallBatch(page);
  await page.locator("#experiment-mode").selectOption("current");
  await page.locator("#noise-levels").fill("0,0.01");
  await page.locator("#run-experiment").click();
  await expect(page.locator("#experiment-status")).toContainText(
    "Completed 4 trials",
  );
  const first = await exported(page);
  expect(first.results.every((r) => r.states === 3)).toBe(true);
  expect(first.results[0].initial).toEqual(first.results[2].initial);
  expect(first.results[0].distance.every((v) => v === 0)).toBe(true);
  await page.locator("#run-experiment").click();
  await expect(page.locator("#experiment-status")).toContainText(
    "Completed 4 trials",
  );
  expect(await exported(page)).toEqual(first);
});
