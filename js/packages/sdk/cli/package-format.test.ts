import assert from "node:assert/strict";
import { readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import test from "node:test";
import { cliPath, execFileText, linkSdk, makeDeckAppProject, tsxBin, writeProjectFile } from "./test-project";

type BuildResult = {
  ok: boolean;
  stdout: string;
  stderr: string;
};

async function runBuild(projectRoot: string, extraArgs: string[] = []): Promise<BuildResult> {
  try {
    const { stdout, stderr } = await execFileText(tsxBin, [cliPath, "build", "--no-zip", ...extraArgs], {
      cwd: projectRoot,
      maxBuffer: 1024 * 1024,
      timeout: 30_000,
    });
    return { ok: true, stdout, stderr };
  } catch (error) {
    const failed = error as Partial<BuildResult>;
    return { ok: false, stdout: failed.stdout ?? "", stderr: failed.stderr ?? "" };
  }
}

function buildOutput(result: BuildResult): string {
  return `${result.stdout}\n${result.stderr}`;
}

void test("build writes native-compatible manifest asset paths", async (t) => {
  const projectRoot = await makeDeckAppProject({
    fonts: { body: "assets/fonts/body.OTF" },
    images: { logo: "assets/images/logo.JPEG" },
  });
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });
  await writeProjectFile(projectRoot, "assets/fonts/body.OTF", "font\n");
  await writeProjectFile(projectRoot, "assets/images/logo.JPEG", "image\n");

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, true, buildOutput(result));
  const manifestPath = path.join(projectRoot, "dist/sample-deck.vdapp/manifest.json");
  const manifest = JSON.parse(await readFile(manifestPath, "utf8")) as unknown;
  assert.deepEqual(manifest, {
    schemaVersion: 1,
    name: "Sample Deck",
    version: "1.2.3",
    entry: "app.js",
    fonts: { body: "fonts/body.otf" },
    images: { logo: "images/logo.jpeg" },
  });
});

for (const { args, mode } of [
  { args: [], mode: "production" },
  { args: ["--dev"], mode: "development" },
]) {
  void test(`build ${args.join(" ") || "(default)"} bundles NODE_ENV=${mode}`, async (t) => {
    const projectRoot = await makeDeckAppProject();
    t.after(async () => {
      await rm(projectRoot, { recursive: true, force: true });
    });
    await writeProjectFile(
      projectRoot,
      "src/App.tsx",
      `export default function App() {
  return process.env.NODE_ENV;
}
`,
    );

    const result = await runBuild(projectRoot, args);
    assert.equal(result.ok, true, buildOutput(result));
    const appJs = await readFile(path.join(projectRoot, "dist/sample-deck.vdapp/app.js"), "utf8");
    assert.match(appJs, new RegExp(`"${mode}"`, "u"));
    assert.doesNotMatch(appJs, /process\.env\.NODE_ENV/u);
  });
}

void test("build rejects display names that exceed native manifest storage", async (t) => {
  const projectRoot = await makeDeckAppProject({ name: "A".repeat(128) });
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, false, buildOutput(result));
  assert.match(buildOutput(result), /name.*127 bytes/u);
});

void test("build rejects generated package names that exceed native package storage", async (t) => {
  const projectRoot = await makeDeckAppProject({ name: "A".repeat(122) });
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, false, buildOutput(result));
  assert.match(buildOutput(result), /Package Name.*127 bytes/u);
});

void test("build rejects package versions that exceed native manifest storage", async (t) => {
  const projectRoot = await makeDeckAppProject({ version: `1.2.3-${"a".repeat(58)}` });
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, false, buildOutput(result));
  assert.match(buildOutput(result), /version.*63 bytes/u);
});

void test("build rejects more assets than the native manifest stores", async (t) => {
  const fonts = Object.fromEntries(
    Array.from({ length: 33 }, (_, index) => [`font${index}`, `assets/fonts/font${index}.ttf`]),
  );
  const projectRoot = await makeDeckAppProject({ fonts });
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, false, buildOutput(result));
  assert.match(buildOutput(result), /at most 32/u);
});

void test("build rejects asset source paths outside the Deck App project", async (t) => {
  const projectRoot = await makeDeckAppProject();
  const escapedFontPath = `../${path.basename(projectRoot)}-outside.ttf`;
  const outsidePath = path.resolve(projectRoot, "..", `${path.basename(projectRoot)}-outside.ttf`);
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
    await rm(outsidePath, { force: true });
  });
  await writeProjectFile(
    projectRoot,
    "vitadeck.config.json",
    `${JSON.stringify(
      {
        name: "Sample Deck",
        entry: "src/App.tsx",
        outDir: "dist",
        fonts: { body: escapedFontPath },
      },
      null,
      2,
    )}\n`,
  );
  await writeFile(outsidePath, "font\n");

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, false, buildOutput(result));
  assert.match(buildOutput(result), /must stay inside the Deck App project/u);
});

void test("build bundles @vitadeck/sdk/host-control into the Deck App", async (t) => {
  const projectRoot = await makeDeckAppProject();
  t.after(async () => {
    await rm(projectRoot, { recursive: true, force: true });
  });
  await linkSdk(projectRoot);
  await writeProjectFile(
    projectRoot,
    "src/App.tsx",
    `import { host } from "@vitadeck/sdk/host-control";
export default function App() {
  void host.echo({ text: "hi" });
  return null;
}
`,
  );

  const result = await runBuild(projectRoot);
  assert.equal(result.ok, true, buildOutput(result));
  const appJs = await readFile(path.join(projectRoot, "dist/sample-deck.vdapp/app.js"), "utf8");
  assert.match(appJs, /nativeGetHostControlLinks/u);
  assert.doesNotMatch(appJs, /@vitadeck\/sdk\/host-control/u);
});
