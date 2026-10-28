import { execFile, type ExecFileOptions } from "node:child_process";
import { mkdtemp, mkdir, symlink, writeFile } from "node:fs/promises";
import { createRequire } from "node:module";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

type TestDeckAppConfig = {
  name: string;
  entry: string;
  outDir: string;
  version?: string;
  fonts?: Record<string, string>;
  images?: Record<string, string>;
};

export const cliPath = fileURLToPath(new URL("cli.ts", import.meta.url));
const sdkRoot = fileURLToPath(new URL("../", import.meta.url));
const jsRoot = fileURLToPath(new URL("../../../", import.meta.url));
const requireFromSdk = createRequire(import.meta.url);
export const tsxBin = path.join(jsRoot, "node_modules", ".bin", process.platform === "win32" ? "tsx.CMD" : "tsx");

export function execFileText(
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

export async function writeProjectFile(projectRoot: string, relPath: string, contents: string): Promise<void> {
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

/** Links this SDK checkout as the project's `@vitadeck/sdk`, for subpaths a Deck App bundles. Needs a built `dist`. */
export async function linkSdk(projectRoot: string): Promise<void> {
  const linkPath = path.join(projectRoot, "node_modules", "@vitadeck", "sdk");
  await mkdir(path.dirname(linkPath), { recursive: true });
  await symlink(sdkRoot, linkPath, process.platform === "win32" ? "junction" : "dir");
}

async function linkSdkBuildDependencies(projectRoot: string): Promise<void> {
  await Promise.all(
    ["@babel/preset-react", "@babel/preset-typescript", "babel-plugin-react-compiler"].map(async (packageName) => {
      await linkDependency(projectRoot, packageName);
    }),
  );
}

export async function makeDeckAppProject(overrides: Partial<TestDeckAppConfig> = {}): Promise<string> {
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
