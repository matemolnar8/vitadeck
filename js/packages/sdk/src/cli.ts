#!/usr/bin/env node
import babel from "@rollup/plugin-babel";
import { mkdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";
import { rolldown } from "rolldown";

type DeckAppConfig = {
  name: string;
  entry: string;
  outDir: string;
};

const PACKAGE_SCHEMA_VERSION = 1;
const SUPPORTED_REACT_PREFIX = "18.3.";
const requireFromHere = createRequire(import.meta.url);

async function readJson(filePath: string): Promise<unknown> {
  try {
    return JSON.parse(await readFile(filePath, "utf8"));
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    throw new Error(`Could not read ${filePath}: ${message}`, { cause: error });
  }
}

function requireObject(value: unknown, sourcePath: string): Record<string, unknown> {
  if (typeof value !== "object" || value === null || Array.isArray(value)) {
    throw new Error(`${sourcePath} must contain a JSON object.`);
  }
  return value as Record<string, unknown>;
}

function requireString(value: unknown, field: string, sourcePath: string): string {
  if (typeof value !== "string" || value.trim() === "") {
    throw new Error(`${sourcePath} must define a non-empty "${field}" string.`);
  }
  return value;
}

async function readConfig(projectRoot: string): Promise<DeckAppConfig> {
  const configPath = path.join(projectRoot, "vitadeck.config.json");
  const raw = requireObject(await readJson(configPath), configPath);
  return {
    name: requireString(raw.name, "name", configPath),
    entry: requireString(raw.entry, "entry", configPath),
    outDir: requireString(raw.outDir, "outDir", configPath),
  };
}

function packageNameFor(displayName: string): string {
  const slug = displayName
    .trim()
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "");
  return `${slug || "deck-app"}.vdapp`;
}

function toImportPath(fromFile: string, toFile: string): string {
  let relativePath = path.relative(path.dirname(fromFile), toFile).replaceAll(path.sep, "/");
  if (!relativePath.startsWith(".")) relativePath = `./${relativePath}`;
  return relativePath;
}

async function validateReact(projectRoot: string): Promise<void> {
  const reactPackagePath = requireFromHere.resolve("react/package.json", { paths: [projectRoot] });
  const reactPackage = requireObject(await readJson(reactPackagePath), reactPackagePath);
  const version = requireString(reactPackage.version, "version", reactPackagePath);
  if (!version.startsWith(SUPPORTED_REACT_PREFIX)) {
    throw new Error(`Deck Apps must resolve react ${SUPPORTED_REACT_PREFIX}x; found ${version}.`);
  }
}

async function build(projectRoot = process.cwd()): Promise<void> {
  const config = await readConfig(projectRoot);
  await validateReact(projectRoot);

  const entryPath = path.resolve(projectRoot, config.entry);
  const outDir = path.resolve(projectRoot, config.outDir);
  const packageDir = path.join(outDir, packageNameFor(config.name));
  const generatedDir = path.join(projectRoot, ".vitadeck");
  const generatedEntry = path.join(generatedDir, "package-entry.tsx");

  await rm(packageDir, { recursive: true, force: true });
  await mkdir(packageDir, { recursive: true });
  await mkdir(generatedDir, { recursive: true });

  await writeFile(
    generatedEntry,
    `import DeckApp from "${toImportPath(generatedEntry, entryPath)}";

globalThis.vitadeckPackage.register(DeckApp);
`,
  );

  const bundle = await rolldown({
    input: generatedEntry,
    external: ["react", "react-compiler-runtime", "@vitadeck/sdk"],
    tsconfig: false,
    transform: {
      define: {
        "process.env.NODE_ENV": JSON.stringify(process.env.NODE_ENV || "development"),
      },
    },
    plugins: [
      babel({
        babelHelpers: "bundled",
        extensions: [".ts", ".tsx", ".js", ".jsx"],
        presets: ["@babel/preset-react", ["@babel/preset-typescript", { isTSX: true, allExtensions: true }]],
        plugins: [["babel-plugin-react-compiler", { target: "18" }]],
      }),
    ],
  });

  await bundle.write({
    file: path.join(packageDir, "app.js"),
    format: "iife",
    name: "vitadeckDeckApp",
    globals: {
      react: "React",
      "react-compiler-runtime": "reactCompilerRuntime",
      "@vitadeck/sdk": "vitadeckSdk",
    },
  });
  await bundle.close();

  await writeFile(
    path.join(packageDir, "manifest.json"),
    JSON.stringify({ schemaVersion: PACKAGE_SCHEMA_VERSION, name: config.name, entry: "app.js" }, null, 2) + "\n",
  );
  await rm(generatedDir, { recursive: true, force: true });

  console.log(`Built ${path.relative(projectRoot, packageDir)}`);
}

async function create(projectName: string): Promise<void> {
  const targetDir = path.resolve(process.cwd(), projectName);
  await mkdir(path.join(targetDir, "src"), { recursive: true });
  await writeFile(
    path.join(targetDir, "package.json"),
    JSON.stringify(
      {
        name: packageNameFor(projectName).replace(/\.vdapp$/, ""),
        version: "0.1.0",
        private: true,
        type: "module",
        scripts: {
          build: "vitadeck build",
          tsc: "tsgo",
        },
        dependencies: {
          "@vitadeck/sdk": "latest",
          react: "^18.3.1",
        },
        devDependencies: {
          "@types/react": "^18.3.28",
          "@typescript/native-preview": "7.0.0-dev.20260501.1",
        },
      },
      null,
      2,
    ) + "\n",
  );
  await writeFile(
    path.join(targetDir, "vitadeck.config.json"),
    JSON.stringify({ name: projectName, entry: "./src/App.tsx", outDir: "./dist" }, null, 2) + "\n",
  );
  await writeFile(
    path.join(targetDir, "tsconfig.json"),
    JSON.stringify({ extends: "@vitadeck/sdk/tsconfig", include: ["src"] }, null, 2) + "\n",
  );
  await writeFile(
    path.join(targetDir, "src", "vitadeck-env.d.ts"),
    '/// <reference types="@vitadeck/sdk/deck-app-env" />\n',
  );
  await writeFile(
    path.join(targetDir, "src", "App.tsx"),
    `import React from "react";
import { Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";

export default function App() {
  const { theme } = useTheme();
  const { x, y } = insetContent();

  return (
    <Screen>
      <Text fontSize={28} color={theme.text}>
        Hello from ${projectName}
      </Text>
    </Screen>
  );
}
`,
  );
  console.log(`Created ${targetDir}`);
}

async function main(): Promise<void> {
  const [, , command, arg] = process.argv;
  if (command === "build" || command === undefined) {
    await build();
    return;
  }
  if (command === "create") {
    if (!arg) throw new Error("Usage: vitadeck create <project-name>");
    await create(arg);
    return;
  }
  throw new Error(`Unknown command: ${command}`);
}

main().catch((error: unknown) => {
  const message = error instanceof Error ? error.message : String(error);
  console.error(message);
  process.exit(1);
});
