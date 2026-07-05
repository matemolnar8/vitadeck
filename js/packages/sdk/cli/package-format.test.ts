import assert from "node:assert/strict";
import { execFile, type ExecFileOptions } from "node:child_process";
import { mkdtemp, mkdir, readFile, rm, symlink, writeFile } from "node:fs/promises";
import { createRequire } from "node:module";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";

type BuildResult = {
  ok: boolean;
  stdout: string;
  stderr: string;
};

type TestDeckAppConfig = {
  name: string;
  entry: string;
  outDir: string;
  version?: string;
  fonts?: Record<string, string>;
  images?: Record<string, string>;
};

const cliPath = fileURLToPath(new URL("cli.ts", import.meta.url));
const jsRoot = fileURLToPath(new URL("../../../", import.meta.url));
const requireFromSdk = createRequire(import.meta.url);
const tsxBin = path.join(jsRoot, "node_modules", ".bin", process.platform === "win32" ? "tsx.CMD" : "tsx");

function execFileText(
  file: string,
  args: string[],
  options: ExecFileOptions,
): Promise<{ stdout: string; stderr: string }> {
  return new Promise((resolve, reject) => {
    execFile(file, args, options, (error, stdout, stderr) => {
      if (error) {
        reject(
          Object.assign(new Error(error.message, { cause: error }), { stdout: String(stdout), stderr: String(stderr) }),
        );
        return;
      }
      resolve({ stdout: String(stdout), stderr: String(stderr) });
    });
  });
}

async function writeProjectFile(projectRoot: string, relPath: string, contents: string): Promise<void> {
  const filePath = path.join(projectRoot, relPath);
  await mkdir(path.dirname(filePath), { recursive: true });
  await writeFile(filePath, contents);
}

async function linkDependency(projectRoot: string, packageName: string): Promise<void> {
  const packageRoot = path.dirname(requireFromSdk.resolve(`${packageName}/package.json`));
  const linkPath = path.join(projectRoot, "node_modules", ...packageName.split("/"));
  await mkdir(path.dirname(linkPath), { recursive: true });
  await symlink(packageRoot, linkPath, process.platform === "win32" ? "junction" : "dir");
}

async function linkSdkBuildDependencies(projectRoot: string): Promise<void> {
  await Promise.all(
    ["@babel/preset-react", "@babel/preset-typescript", "babel-plugin-react-compiler"].map(async (packageName) => {
      await linkDependency(projectRoot, packageName);
    }),
  );
}

async function makeDeckAppProject(overrides: Partial<TestDeckAppConfig> = {}): Promise<string> {
  const projectRoot = await mkdtemp(path.join(os.tmpdir(), "vitadeck-sdk-package-"));
  await writeProjectFile(
    projectRoot,
    "package.json",
    `${JSON.stringify({ name: "vitadeck-sdk-test-deck", version: "1.2.3", type: "module" }, null, 2)}\n`,
  );
  await writeProjectFile(
    projectRoot,
    "node_modules/react/package.json",
    `${JSON.stringify({ name: "react", version: "18.3.1" }, null, 2)}\n`,
  );
  await linkSdkBuildDependencies(projectRoot);
  await writeProjectFile(
    projectRoot,
    "src/App.tsx",
    `export default function App() {
  return null;
}
`,
  );

  const config: TestDeckAppConfig = {
    name: "Sample Deck",
    entry: "src/App.tsx",
    outDir: "dist",
    ...overrides,
  };
  await writeProjectFile(projectRoot, "vitadeck.config.json", `${JSON.stringify(config, null, 2)}\n`);
  return projectRoot;
}

async function runBuild(projectRoot: string): Promise<BuildResult> {
  try {
    const { stdout, stderr } = await execFileText(tsxBin, [cliPath, "build", "--no-zip"], {
      cwd: projectRoot,
      env: { ...process.env, NODE_ENV: "test" },
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
