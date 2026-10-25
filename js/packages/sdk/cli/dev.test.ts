import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { once } from "node:events";
import { rm } from "node:fs/promises";
import { createServer, type IncomingMessage, type ServerResponse } from "node:http";
import type { AddressInfo } from "node:net";
import test, { type TestContext } from "node:test";
import { unzipSync } from "fflate";
import { cliPath, makeDeckAppProject, tsxBin, writeProjectFile } from "./test-project";

type ReceivedUpload = { method: string; url: string; contentType: string; body: Buffer };
type Reply = { status: number; body: unknown };

const INSTALLED = { status: 200, body: { ok: true, packageName: "sample-deck.vdapp", version: "1.2.3" } };

async function readBody(request: IncomingMessage): Promise<Buffer> {
  const chunks: Buffer[] = [];
  for await (const chunk of request) chunks.push(chunk as Buffer);
  return Buffer.concat(chunks);
}

async function startStub(t: TestContext, replies: Reply[]): Promise<{ address: string; uploads: ReceivedUpload[] }> {
  const uploads: ReceivedUpload[] = [];
  async function handle(request: IncomingMessage, response: ServerResponse): Promise<void> {
    uploads.push({
      method: request.method ?? "",
      url: request.url ?? "",
      contentType: request.headers["content-type"] ?? "",
      body: await readBody(request),
    });
    const reply = replies[Math.min(uploads.length, replies.length) - 1] ?? INSTALLED;
    response.writeHead(reply.status, { "Content-Type": "application/json" });
    response.end(JSON.stringify(reply.body));
  }
  const server = createServer((request, response) => {
    void handle(request, response);
  });
  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  t.after(() => {
    server.close();
  });
  const { port } = server.address() as AddressInfo;
  return { address: `127.0.0.1:${port}`, uploads };
}

async function startDev(t: TestContext, address: string) {
  const projectRoot = await makeDeckAppProject();
  const child = spawn(tsxBin, [cliPath, "dev", address], { cwd: projectRoot });
  let output = "";
  const onData = (chunk: Buffer): void => {
    output += chunk.toString();
  };
  child.stdout.on("data", onData);
  child.stderr.on("data", onData);
  const exited = new Promise<number | null>((resolve) => {
    child.once("exit", resolve);
  });
  t.after(async () => {
    child.kill("SIGKILL");
    await rm(projectRoot, { recursive: true, force: true });
  });

  function waitFor(pattern: RegExp, count = 1): Promise<void> {
    const matches = (): number => (output.match(new RegExp(pattern.source, "gu")) ?? []).length;
    return new Promise((resolve, reject) => {
      const done = (error?: Error): void => {
        clearTimeout(timer);
        child.stdout.off("data", check);
        child.off("exit", onExit);
        if (error) reject(error);
        else resolve();
      };
      const check = (): void => {
        if (matches() >= count) done();
      };
      const onExit = (): void => {
        done(new Error(`dev exited before ${pattern.source} x${count}. Output:\n${output}`));
      };
      const timer = setTimeout(() => {
        done(new Error(`Timed out waiting for ${pattern.source} x${count}. Output:\n${output}`));
      }, 30_000);
      child.stdout.on("data", check);
      child.once("exit", onExit);
      check();
    });
  }

  function stop(): Promise<number | null> {
    child.kill("SIGINT");
    return exited;
  }

  return { projectRoot, waitFor, stop, output: () => output };
}

void test("dev uploads a zip on start and again after a source change", async (t) => {
  const stub = await startStub(t, [INSTALLED]);
  const dev = await startDev(t, stub.address);

  await dev.waitFor(/\] installed sample-deck\.vdapp 1\.2\.3/u);
  const [first] = stub.uploads;
  assert.ok(first);
  assert.equal(first.method, "POST");
  assert.equal(first.url, "/upload");
  assert.equal(first.contentType, "application/zip");
  const entries = unzipSync(new Uint8Array(first.body));
  assert.ok("sample-deck.vdapp/manifest.json" in entries, Object.keys(entries).join(", "));
  assert.ok("sample-deck.vdapp/app.js" in entries);

  await writeProjectFile(dev.projectRoot, "src/App.tsx", "export default function App() {\n  return 'changed';\n}\n");
  await dev.waitFor(/\] installed sample-deck\.vdapp 1\.2\.3/u, 2);
  const last = stub.uploads.at(-1);
  assert.ok(last);
  assert.match(
    Buffer.from(unzipSync(new Uint8Array(last.body))["sample-deck.vdapp/app.js"] ?? []).toString(),
    /changed/u,
  );

  assert.equal(await dev.stop(), 0, dev.output());
});

void test("dev retries once when the listener is busy", async (t) => {
  const busy = { status: 409, body: { ok: false, code: "upload_in_progress", message: "busy" } };
  const stub = await startStub(t, [busy, INSTALLED]);
  const dev = await startDev(t, stub.address);

  await dev.waitFor(/\] installed sample-deck\.vdapp 1\.2\.3/u);
  assert.equal(stub.uploads.length, 2);
  assert.equal(await dev.stop(), 0, dev.output());
});

void test("dev prints the status, code and message of a rejected upload", async (t) => {
  const rejected = { status: 422, body: { ok: false, code: "invalid_package", message: "manifest.json is missing" } };
  const stub = await startStub(t, [rejected]);
  const dev = await startDev(t, stub.address);

  await dev.waitFor(/\] 422 invalid_package: manifest\.json is missing/u);
  assert.equal(await dev.stop(), 0, dev.output());
});

void test("dev explains a refused connection", async (t) => {
  const server = createServer();
  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  const { port } = server.address() as AddressInfo;
  server.close();
  await once(server, "close");
  const dev = await startDev(t, `http://127.0.0.1:${port}/`);

  await dev.waitFor(/Open the VitaDeck Shell and confirm Upload/u);
  assert.equal(await dev.stop(), 0, dev.output());
});
