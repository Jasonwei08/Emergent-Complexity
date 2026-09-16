import { test, expect } from "@playwright/test";

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
  await expect(page.locator("#status-label")).toHaveText("PAUSED");
});
test.afterEach(async ({ page }) => expect(page.__failures).toEqual([]));

test("direct loads, refresh and original controls", async ({
  page,
  baseURL,
}) => {
  await page.reload();
  await expect(page.locator("#status-label")).toHaveText("PAUSED");
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

test("patterns, oscillator evolution, and help", async ({ page }) => {
  await expect(page.locator("[data-pattern]")).toHaveCount(6);
  await page.locator('[data-pattern="blinker"]').click();
  await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText("3");
  await expect(page.locator("#births")).toHaveText("2");
  await page.locator("#step").click();
  await expect(page.locator("#population")).toHaveText("3");
  await expect(page.locator("#generation")).toHaveText("0002");
  await page.locator("#about").click();
  await expect(page.locator("#help")).toBeVisible();
  await page.locator("#close-help").click();
  await expect(page.locator("#help")).not.toBeVisible();
});
