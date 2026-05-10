import babel from "@rollup/plugin-babel";
import { mkdir, readdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import { rolldown } from "rolldown";

type DeckAppConfig = {
  name: string;
  entry: string;
  outDir: string;
  version?: string;
};

const PACKAGE_SCHEMA_VERSION = 1;
const SUPPORTED_REACT_PREFIX = "18.3.";
/** Must match `name` / copy in `cli/templates/scaffold` so the template is a valid workspace package and typechecks; replaced with the author's slug on `create`. */
const SCAFFOLD_TEMPLATE_LABEL = "vitadeck-scaffold-template";
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
  const config: DeckAppConfig = {
    name: requireString(raw.name, "name", configPath),
    entry: requireString(raw.entry, "entry", configPath),
    outDir: requireString(raw.outDir, "outDir", configPath),
  };
  if (raw.version !== undefined) {
    config.version = requireString(raw.version, "version", configPath);
  }
  return config;
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

function validatePackageVersion(version: string, sourcePath: string): void {
  if (!/^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$/.test(version)) {
    throw new Error(`${sourcePath} must define "version" as a semver string.`);
  }
}

async function readPackageVersion(projectRoot: string, config: DeckAppConfig): Promise<string> {
  if (config.version) {
    validatePackageVersion(config.version, "vitadeck.config.json");
    return config.version;
  }

  const packagePath = path.join(projectRoot, "package.json");
  const raw = requireObject(await readJson(packagePath), packagePath);
  const version = requireString(raw.version, "version", packagePath);
  validatePackageVersion(version, packagePath);
  return version;
}

function isScaffoldCopySkippedDir(name: string): boolean {
  return name === "node_modules" || name === "dist";
}

async function copyScaffold(templateRoot: string, targetRoot: string, slug: string): Promise<void> {
  const stack: string[] = [""];
  while (stack.length > 0) {
    const rel = stack.pop()!;
    const absDir = path.join(templateRoot, rel);
    const entries = await readdir(absDir, { withFileTypes: true });
    for (const ent of entries) {
      if (ent.isDirectory() && isScaffoldCopySkippedDir(ent.name)) continue;
      const childRel = rel ? path.join(rel, ent.name) : ent.name;
      const src = path.join(templateRoot, childRel);
      const dest = path.join(targetRoot, childRel);
      if (ent.isDirectory()) {
        await mkdir(dest, { recursive: true });
        stack.push(childRel);
      } else if (ent.isFile()) {
        await mkdir(path.dirname(dest), { recursive: true });
        const body = (await readFile(src, "utf8"))
          .replaceAll("{{slug}}", slug)
          .replaceAll(SCAFFOLD_TEMPLATE_LABEL, slug);
        await writeFile(dest, body);
      }
    }
  }
}

async function build(projectRoot = process.cwd()): Promise<void> {
  const config = await readConfig(projectRoot);
  await validateReact(projectRoot);
  const version = await readPackageVersion(projectRoot, config);

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
    JSON.stringify({ schemaVersion: PACKAGE_SCHEMA_VERSION, name: config.name, version, entry: "app.js" }, null, 2) + "\n",
  );
  await rm(generatedDir, { recursive: true, force: true });

  console.log(`Built ${path.relative(projectRoot, packageDir)}`);
}

async function create(projectName: string): Promise<void> {
  const targetDir = path.resolve(process.cwd(), projectName);
  const slug = packageNameFor(projectName).replace(/\.vdapp$/, "");
  const templateRoot = fileURLToPath(new URL("./templates/scaffold", import.meta.url));
  await mkdir(targetDir, { recursive: true });
  await copyScaffold(templateRoot, targetDir, slug);
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
