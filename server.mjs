import http from "node:http";
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { pathToFileURL } from "node:url";
const types = new Map([
  ['/index.html', 'text/html; charset=utf-8'],
  ['/style.css', 'text/css; charset=utf-8'],
  ['/app.js', 'text/javascript; charset=utf-8'],
  ['/life_engine.js', 'text/javascript; charset=utf-8'],
  ['/life_engine.wasm', 'application/wasm']
]);
// Flags make production subpath testing identical on PowerShell and POSIX shells.
const args = process.argv.slice(2);
const option = (name, fallback) => {
  const index = args.indexOf(name);
  if (index < 0) return fallback;
  if (!args[index + 1] || args[index + 1].startsWith("--"))
    throw Error(`Missing ${name} value`);
  return args[index + 1];
};
const root = pathToFileURL(resolve(option("--dir", ".")) + "/");
const base = option("--base", "/");
if (
  !/^\/(?:[A-Za-z0-9._-]+\/)*$/.test(base) ||
  base.includes("/../") ||
  base.includes("/./")
)
  throw Error("Base must be a slash-delimited path, such as /my-repository/");
const port = Number(option("--port", process.env.PORT || 4173));
if (!Number.isInteger(port) || port < 1 || port > 65535)
  throw Error("PORT must be 1..65535");
const server = http.createServer(async (req, res) => {
  const pathname = new URL(req.url, "http://localhost").pathname;
  // Match the directory redirect used by static hosts, including Pages.
  if (base !== "/" && pathname === base.slice(0, -1)) {
    res.writeHead(301, { Location: base }).end();
    return;
  }
  if (!pathname.startsWith(base)) {
    res.writeHead(404).end("Not found");
    return;
  }
  const route = "/" + (pathname.slice(base.length) || "index.html");
  if (!types.has(route)) {
    res.writeHead(404).end("Not found");
    return;
  }
  try {
    const body = await readFile(new URL("." + route, root));
    res.writeHead(200, { "Content-Type": types.get(route) });
    res.end(body);
  } catch {
    res.writeHead(500).end("Unable to load website asset");
  }
});
server.on("error", (error) => {
  console.error(error.message);
  process.exitCode = 1;
});
server.listen(port, "127.0.0.1", () =>
  console.log(`Life playground: http://127.0.0.1:${port}${base}`),
);
