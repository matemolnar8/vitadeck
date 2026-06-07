import babel from "@rollup/plugin-babel";
import { zipSync } from "fflate";
import { access, copyFile, mkdir, readdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";
import { fileURLToPath } from "node:url";
import { rolldown } from "rolldown";

type DeckAppConfig = {
  name: string;
  entry: string;
  outDir: string;
  version?: string;
  fonts?: Record<string, string>;
};

const PACKAGE_SCHEMA_VERSION = 1;
const SUPPORTED_REACT_PREFIX = "18.3.";
const RESERVED_FONT_NAMES = new Set(["default"]);
const SUPPORTED_FONT_EXTENSIONS = new Set([".ttf", ".otf", ".fnt", ".bdf"]);
const FONT_NAME_PATTERN = /^[A-Za-z][A-Za-z0-9_-]{0,62}$/;
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

function readFonts(value: unknown, sourcePath: string): Record<string, string> | undefined {
  if (value === undefined) return undefined;
  const raw = requireObject(value, `${sourcePath} "fonts"`);
  const fonts: Record<string, string> = {};
  for (const [name, fontPath] of Object.entries(raw)) {
    if (!FONT_NAME_PATTERN.test(name)) {
      throw new Error(`${sourcePath} font name "${name}" must match ${FONT_NAME_PATTERN.toString()}.`);
    }
    if (RESERVED_FONT_NAMES.has(name)) {
      throw new Error(`${sourcePath} font name "${name}" is reserved by VitaDeck.`);
    }
    fonts[name] = requireString(fontPath, `fonts.${name}`, sourcePath);
  }
  return fonts;
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
  config.fonts = readFonts(raw.fonts, configPath);
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

function validateFontSourcePath(fontPath: string, sourcePath: string): void {
  if (path.isAbsolute(fontPath)) {
    throw new Error(`${sourcePath} font path "${fontPath}" must be relative to the Deck App project.`);
  }
  const extension = path.extname(fontPath).toLowerCase();
  if (!SUPPORTED_FONT_EXTENSIONS.has(extension)) {
    throw new Error(`${sourcePath} font path "${fontPath}" must use a supported font extension.`);
  }
}

async function assertFileExists(filePath: string, sourcePath: string): Promise<void> {
  try {
    await access(filePath);
  } catch (error) {
    throw new Error(`${sourcePath} font file is missing: ${filePath}`, { cause: error });
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

async function writeRuntimeUploadArchive(packageDir: string, zipOutPath: string): Promise<void> {
  const rootName = path.basename(packageDir);
  const filesForZip: Record<string, Uint8Array> = {};

  async function walk(relInsidePackage: string): Promise<void> {
    const abs = path.join(packageDir, relInsidePackage);
    const entries = await readdir(abs, { withFileTypes: true });
    for (const ent of entries) {
      const childRel = relInsidePackage ? `${relInsidePackage}/${ent.name}` : ent.name;
      const zipEntryPath = `${rootName}/${childRel}`.replaceAll("\\", "/");
      if (ent.isDirectory()) {
        await walk(childRel);
      } else if (ent.isFile()) {
        const buf = await readFile(path.join(abs, ent.name));
        filesForZip[zipEntryPath] = new Uint8Array(buf.buffer, buf.byteOffset, buf.byteLength);
      }
    }
  }

  await walk("");
  const zipped = zipSync(filesForZip, { level: 6 });
  await writeFile(zipOutPath, zipped);
}

async function writeFontTypes(generatedDir: string, fonts: Record<string, string> | undefined): Promise<void> {
  const names = Object.keys(fonts ?? {}).sort();
  const declarations =
    names.length === 0 ? "" : names.map((name) => `    ${JSON.stringify(name)}: true;`).join("\n") + "\n";
  await writeFile(
    path.join(generatedDir, "font-names.d.ts"),
    `import "@vitadeck/sdk/types";

declare module "@vitadeck/sdk/types" {
  interface VitaDeckFontMap {
${declarations}  }
}
`,
  );
}

async function copyFonts(
  projectRoot: string,
  packageDir: string,
  fonts: Record<string, string> | undefined,
): Promise<Record<string, string> | undefined> {
  const entries = Object.entries(fonts ?? {});
  if (entries.length === 0) return undefined;

  const manifestFonts: Record<string, string> = {};
  const outFontsDir = path.join(packageDir, "fonts");
  await mkdir(outFontsDir, { recursive: true });

  for (const [name, sourceRel] of entries) {
    validateFontSourcePath(sourceRel, "vitadeck.config.json");
    const sourceAbs = path.resolve(projectRoot, sourceRel);
    await assertFileExists(sourceAbs, "vitadeck.config.json");
    const packageRel = `fonts/${name}${path.extname(sourceRel).toLowerCase()}`;
    await copyFile(sourceAbs, path.join(packageDir, packageRel));
    manifestFonts[name] = packageRel;
  }

  return manifestFonts;
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

type BuildOptions = { noZip?: boolean };

async function build(projectRoot = process.cwd(), options: BuildOptions = {}): Promise<void> {
  const config = await readConfig(projectRoot);
  await validateReact(projectRoot);
  const version = await readPackageVersion(projectRoot, config);

  const entryPath = path.resolve(projectRoot, config.entry);
  const outDir = path.resolve(projectRoot, config.outDir);
  const packageFolderName = packageNameFor(config.name);
  const packageDir = path.join(outDir, packageFolderName);
  const archivePath = path.join(outDir, `${packageFolderName}.zip`);
  const generatedDir = path.join(projectRoot, ".vitadeck");
  const generatedEntry = path.join(generatedDir, "package-entry.tsx");

  await rm(packageDir, { recursive: true, force: true });
  await rm(archivePath, { force: true });
  await mkdir(packageDir, { recursive: true });
  await mkdir(generatedDir, { recursive: true });
  await writeFontTypes(generatedDir, config.fonts);

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
  const manifestFonts = await copyFonts(projectRoot, packageDir, config.fonts);

  const manifest = {
    schemaVersion: PACKAGE_SCHEMA_VERSION,
    name: config.name,
    version,
    entry: "app.js",
    ...(manifestFonts ? { fonts: manifestFonts } : {}),
  };
  await writeFile(
    path.join(packageDir, "manifest.json"),
    JSON.stringify(manifest, null, 2) + "\n",
  );
  await rm(generatedEntry, { force: true });

  console.log(`Built ${path.relative(projectRoot, packageDir)}`);
  if (!options.noZip) {
    await writeRuntimeUploadArchive(packageDir, archivePath);
    console.log(`Wrote ${path.relative(projectRoot, archivePath)}`);
  }
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
  const rawArgs = process.argv.slice(2);
  const flags = new Set(rawArgs.filter((a) => a.startsWith("--")));
  const positional = rawArgs.filter((a) => !a.startsWith("--"));
  const unknownFlags = [...flags].filter((f) => f !== "--no-zip");
  if (unknownFlags.length > 0) {
    throw new Error(`Unknown flag(s): ${unknownFlags.join(", ")}`);
  }
  const noZip = flags.has("--no-zip");
  const [command, arg] = positional;

  if (command === undefined || command === "build") {
    await build(process.cwd(), { noZip });
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
