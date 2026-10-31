import { watch } from "node:fs";
import { access, copyFile, cp, mkdir, readdir, readFile, rename, rm, stat, writeFile } from "node:fs/promises";
import { createRequire } from "node:module";
import path from "node:path";
import { setTimeout as delay } from "node:timers/promises";
import { fileURLToPath } from "node:url";
import babel from "@rollup/plugin-babel";
import { zipSync } from "fflate";
import { rolldown } from "rolldown";
import { HOST_PLUGIN_SCHEMA_VERSION, type HostPluginManifest } from "../src/host-control/plugin";
import type { DeckAppPackageManifest } from "../src/package-manifest";

type DeckAppConfig = {
  name: string;
  /** The Deck App's root component. A project without one builds only its Host Control Plugin. */
  entry?: string;
  outDir: string;
  version?: string;
  fonts?: Record<string, string>;
  images?: Record<string, string>;
  /** A Host Control Plugin, built into the `<outDir>/<slug>.vdplugin` package. */
  hostPlugin?: { entry: string; files?: string };
};

const PACKAGE_SCHEMA_VERSION = 1;
const SUPPORTED_REACT_PREFIX = "18.3.";
const RESERVED_FONT_NAMES = new Set(["default"]);
const SUPPORTED_FONT_EXTENSIONS = new Set([".ttf", ".otf", ".fnt", ".bdf"]);
const SUPPORTED_IMAGE_EXTENSIONS = new Set([
  ".png",
  ".jpg",
  ".jpeg",
  ".bmp",
  ".tga",
  ".gif",
  ".psd",
  ".hdr",
  ".pic",
  ".qoi",
]);
const SUPPORTED_HOST_PLUGIN_EXTENSIONS = new Set([".ts", ".mts", ".js", ".mjs"]);
const FONT_NAME_PATTERN = /^[A-Za-z][A-Za-z0-9_-]{0,62}$/u;
const NATIVE_DISPLAY_NAME_MAX_BYTES = 127;
const NATIVE_PACKAGE_NAME_MAX_BYTES = 127;
const NATIVE_PACKAGE_VERSION_MAX_BYTES = 63;
const NATIVE_PACKAGE_ASSET_MAX = 32;
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
  validateAssetCount(raw, "fonts", sourcePath);
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

function readImages(value: unknown, sourcePath: string): Record<string, string> | undefined {
  if (value === undefined) return undefined;
  const raw = requireObject(value, `${sourcePath} "images"`);
  validateAssetCount(raw, "images", sourcePath);
  const images: Record<string, string> = {};
  for (const [name, imagePath] of Object.entries(raw)) {
    if (!FONT_NAME_PATTERN.test(name)) {
      throw new Error(`${sourcePath} image name "${name}" must match ${FONT_NAME_PATTERN.toString()}.`);
    }
    images[name] = requireString(imagePath, `images.${name}`, sourcePath);
  }
  return images;
}

async function readConfig(projectRoot: string): Promise<DeckAppConfig> {
  const configPath = path.join(projectRoot, "vitadeck.config.json");
  const raw = requireObject(await readJson(configPath), configPath);
  const config: DeckAppConfig = {
    name: requireString(raw.name, "name", configPath),
    outDir: requireString(raw.outDir, "outDir", configPath),
  };
  if (raw.entry !== undefined) config.entry = requireString(raw.entry, "entry", configPath);
  if (raw.version !== undefined) {
    config.version = requireString(raw.version, "version", configPath);
  }
  config.fonts = readFonts(raw.fonts, configPath);
  config.images = readImages(raw.images, configPath);
  if (raw.hostPlugin !== undefined) {
    const hostPlugin = requireObject(raw.hostPlugin, `${configPath} "hostPlugin"`);
    config.hostPlugin = { entry: requireString(hostPlugin.entry, "hostPlugin.entry", configPath) };
    if (hostPlugin.files !== undefined) {
      config.hostPlugin.files = requireString(hostPlugin.files, "hostPlugin.files", configPath);
    }
  }
  if (config.entry === undefined && config.hostPlugin === undefined) {
    throw new Error(`${configPath} must define "entry" (a Deck App), "hostPlugin", or both.`);
  }
  return config;
}

function packageNameFor(displayName: string): string {
  const slug = displayName
    .trim()
    .toLowerCase()
    .replaceAll(/[^a-z0-9]+/gu, "-")
    .replaceAll(/^-+|-+$/gu, "");
  const packageName = `${slug || "deck-app"}.vdapp`;
  validatePackageName(packageName);
  return packageName;
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
  if (!/^\d+\.\d+\.\d+(?:[-+][0-9A-Za-z.-]+)?$/u.test(version)) {
    throw new Error(`${sourcePath} must define "version" as a semver string.`);
  }
  if (utf8ByteLength(version) > NATIVE_PACKAGE_VERSION_MAX_BYTES) {
    throw new Error(
      `${sourcePath} "version" must be at most ${NATIVE_PACKAGE_VERSION_MAX_BYTES} bytes for VitaDeck packages.`,
    );
  }
}

function validateDisplayName(displayName: string, sourcePath: string): void {
  if (utf8ByteLength(displayName) > NATIVE_DISPLAY_NAME_MAX_BYTES) {
    throw new Error(
      `${sourcePath} "name" must be at most ${NATIVE_DISPLAY_NAME_MAX_BYTES} bytes for VitaDeck packages.`,
    );
  }
}

function validatePackageName(packageName: string): void {
  if (
    packageName === "" ||
    !packageName.endsWith(".vdapp") ||
    packageName.includes("/") ||
    packageName.includes("\\") ||
    packageName.includes("..")
  ) {
    throw new Error(`Generated Deck App Package Name "${packageName}" is not native-safe.`);
  }
  if (utf8ByteLength(packageName) > NATIVE_PACKAGE_NAME_MAX_BYTES) {
    throw new Error(
      `Generated Deck App Package Name "${packageName}" must be at most ${NATIVE_PACKAGE_NAME_MAX_BYTES} bytes.`,
    );
  }
}

function validateAssetCount(raw: Record<string, unknown>, label: "fonts" | "images", sourcePath: string): void {
  const count = Object.keys(raw).length;
  if (count > NATIVE_PACKAGE_ASSET_MAX) {
    throw new Error(`${sourcePath} declares ${count} ${label}; VitaDeck packages support at most 32.`);
  }
}

function utf8ByteLength(value: string): number {
  return Buffer.byteLength(value, "utf8");
}

function resolveProjectPath(projectRoot: string, relPath: string, sourcePath: string, label: string): string {
  if (path.isAbsolute(relPath) || path.win32.isAbsolute(relPath)) {
    throw new Error(`${sourcePath} ${label} path "${relPath}" must be relative to the Deck App project.`);
  }
  const normalizedRel = path.posix.normalize(relPath.replaceAll("\\", "/"));
  if (normalizedRel === ".." || normalizedRel.startsWith("../")) {
    throw new Error(`${sourcePath} ${label} path "${relPath}" must stay inside the Deck App project.`);
  }
  return path.resolve(projectRoot, relPath);
}

function resolveAssetSourcePath(
  projectRoot: string,
  assetPath: string,
  sourcePath: string,
  assetLabel: "font" | "image" | "hostPlugin.entry",
  supportedExtensions: Set<string>,
): string {
  resolveProjectPath(projectRoot, assetPath, sourcePath, assetLabel);
  const extension = path.extname(assetPath).toLowerCase();
  if (!supportedExtensions.has(extension)) {
    throw new Error(`${sourcePath} ${assetLabel} path "${assetPath}" must use a supported ${assetLabel} extension.`);
  }
  return path.resolve(projectRoot, assetPath);
}

async function assertFileExists(filePath: string, sourcePath: string, assetLabel = "font"): Promise<void> {
  try {
    await access(filePath);
  } catch (error) {
    throw new Error(`${sourcePath} ${assetLabel} file is missing: ${filePath}`, { cause: error });
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

/** Zips a package folder as the archive's one top-level folder: a Runtime Upload Archive or a `.vdplugin.zip`. */
async function writePackageArchive(packageDir: string, zipOutPath: string): Promise<void> {
  const rootName = path.basename(packageDir);
  const filesForZip: Record<string, Uint8Array> = {};

  async function walk(relInsidePackage: string): Promise<void> {
    const abs = path.join(packageDir, relInsidePackage);
    const entries = await readdir(abs, { withFileTypes: true });
    await Promise.all(
      entries.map(async (ent) => {
        const childRel = relInsidePackage ? `${relInsidePackage}/${ent.name}` : ent.name;
        const zipEntryPath = `${rootName}/${childRel}`.replaceAll("\\", "/");
        if (ent.isDirectory()) {
          await walk(childRel);
        } else if (ent.isFile()) {
          const buf = await readFile(path.join(abs, ent.name));
          filesForZip[zipEntryPath] = new Uint8Array(buf.buffer, buf.byteOffset, buf.byteLength);
        }
      }),
    );
  }

  await walk("");
  const zipped = zipSync(filesForZip, { level: 6 });
  await writeFile(zipOutPath, zipped);
}

async function writeFontTypes(generatedDir: string, fonts: Record<string, string> | undefined): Promise<void> {
  const names = Object.keys(fonts ?? {}).toSorted();
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

async function writeImageTypes(generatedDir: string, images: Record<string, string> | undefined): Promise<void> {
  const names = Object.keys(images ?? {}).toSorted();
  const declarations =
    names.length === 0 ? "" : names.map((name) => `    ${JSON.stringify(name)}: true;`).join("\n") + "\n";
  await writeFile(
    path.join(generatedDir, "image-names.d.ts"),
    `import "@vitadeck/sdk/types";

declare module "@vitadeck/sdk/types" {
  interface VitaDeckImageMap {
${declarations}  }
}
`,
  );
}

async function copyImages(
  projectRoot: string,
  packageDir: string,
  images: Record<string, string> | undefined,
): Promise<Record<string, string> | undefined> {
  const entries = Object.entries(images ?? {});
  if (entries.length === 0) return undefined;

  const manifestImages: Record<string, string> = {};
  const outImagesDir = path.join(packageDir, "images");
  await mkdir(outImagesDir, { recursive: true });

  await Promise.all(
    entries.map(async ([name, sourceRel]) => {
      const sourceAbs = resolveAssetSourcePath(
        projectRoot,
        sourceRel,
        "vitadeck.config.json",
        "image",
        SUPPORTED_IMAGE_EXTENSIONS,
      );
      await assertFileExists(sourceAbs, "vitadeck.config.json", "image");
      const packageRel = `images/${name}${path.extname(sourceRel).toLowerCase()}`;
      await copyFile(sourceAbs, path.join(packageDir, packageRel));
      manifestImages[name] = packageRel;
    }),
  );

  return manifestImages;
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

  await Promise.all(
    entries.map(async ([name, sourceRel]) => {
      const sourceAbs = resolveAssetSourcePath(
        projectRoot,
        sourceRel,
        "vitadeck.config.json",
        "font",
        SUPPORTED_FONT_EXTENSIONS,
      );
      await assertFileExists(sourceAbs, "vitadeck.config.json");
      const packageRel = `fonts/${name}${path.extname(sourceRel).toLowerCase()}`;
      await copyFile(sourceAbs, path.join(packageDir, packageRel));
      manifestFonts[name] = packageRel;
    }),
  );

  return manifestFonts;
}

async function copyScaffoldDir(templateRoot: string, targetRoot: string, slug: string, rel = ""): Promise<void> {
  const absDir = path.join(templateRoot, rel);
  const entries = await readdir(absDir, { withFileTypes: true });
  await Promise.all(
    entries.map(async (ent) => {
      if (ent.isDirectory() && isScaffoldCopySkippedDir(ent.name)) return;
      const childRel = rel ? path.join(rel, ent.name) : ent.name;
      const src = path.join(templateRoot, childRel);
      const dest = path.join(targetRoot, childRel);
      if (ent.isDirectory()) {
        await mkdir(dest, { recursive: true });
        await copyScaffoldDir(templateRoot, targetRoot, slug, childRel);
      } else if (ent.isFile()) {
        await mkdir(path.dirname(dest), { recursive: true });
        await writeFile(dest, fillTemplate(await readFile(src, "utf8"), slug));
      }
    }),
  );
}

function fillTemplate(body: string, slug: string): string {
  return body.replaceAll("{{slug}}", slug).replaceAll(SCAFFOLD_TEMPLATE_LABEL, slug);
}

/** Copies an add-on template over a scaffolded project. Its JSON files are merged into existing ones, one level deep. */
async function applyTemplateOverlay(overlayRoot: string, targetRoot: string, slug: string): Promise<void> {
  const entries = await readdir(overlayRoot, { recursive: true, withFileTypes: true });
  await Promise.all(
    entries
      .filter((ent) => ent.isFile())
      .map(async (ent) => {
        const rel = path.relative(overlayRoot, path.join(ent.parentPath, ent.name));
        const dest = path.join(targetRoot, rel);
        const body = fillTemplate(await readFile(path.join(overlayRoot, rel), "utf8"), slug);
        await mkdir(path.dirname(dest), { recursive: true });
        if (!ent.name.endsWith(".json") || !(await fileExists(dest))) {
          await writeFile(dest, body);
          return;
        }
        const merged = requireObject(await readJson(dest), dest);
        for (const [key, value] of Object.entries(requireObject(JSON.parse(body), rel))) {
          const current = merged[key];
          merged[key] = isPlainObject(current) && isPlainObject(value) ? { ...current, ...value } : value;
        }
        await writeFile(dest, `${JSON.stringify(merged, null, 2)}\n`);
      }),
  );
}

function isPlainObject(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

async function fileExists(filePath: string): Promise<boolean> {
  try {
    await access(filePath);
    return true;
  } catch {
    return false;
  }
}

type BuildOptions = { noZip?: boolean; dev?: boolean };
type BuildOutput = {
  packageDir?: string;
  archivePath?: string;
  hostPluginDir?: string;
  hostPluginArchivePath?: string;
};

const HOST_PLUGIN_ENTRY = "companion.mjs";

/**
 * Writes the Host Plugin Package: the implementation bundled with everything it imports except Node built-ins,
 * the optional `files` folder, and `manifest.json`.
 */
async function buildHostPlugin(
  plugin: { entry: string; files?: string },
  outDir: string,
  manifest: Omit<HostPluginManifest, "schemaVersion" | "entry">,
): Promise<void> {
  // Written beside the old package and swapped in at the end, so a failed build leaves the last good one.
  const staging = `${outDir}.tmp`;
  await rm(staging, { recursive: true, force: true });
  await mkdir(staging, { recursive: true });
  try {
    const bundle = await rolldown({ input: plugin.entry, platform: "node", tsconfig: false });
    await bundle.write({ file: path.join(staging, HOST_PLUGIN_ENTRY), format: "esm", codeSplitting: false });
    await bundle.close();
    if (plugin.files !== undefined) {
      // Links are copied as the files they point to, so the zip, which holds only files, matches the folder.
      await cp(plugin.files, path.join(staging, "files"), { recursive: true, dereference: true });
    }
    const full: HostPluginManifest = {
      schemaVersion: HOST_PLUGIN_SCHEMA_VERSION,
      ...manifest,
      entry: HOST_PLUGIN_ENTRY,
    };
    await writeFile(path.join(staging, "manifest.json"), `${JSON.stringify(full, null, 2)}\n`);
    await rm(outDir, { recursive: true, force: true });
    await rename(staging, outDir);
  } finally {
    await rm(staging, { recursive: true, force: true });
  }
}

async function resolveHostPlugin(
  projectRoot: string,
  plugin: { entry: string; files?: string },
): Promise<{ entry: string; files?: string }> {
  const entry = resolveAssetSourcePath(
    projectRoot,
    plugin.entry,
    "vitadeck.config.json",
    "hostPlugin.entry",
    SUPPORTED_HOST_PLUGIN_EXTENSIONS,
  );
  await assertFileExists(entry, "vitadeck.config.json", "hostPlugin.entry");
  if (plugin.files === undefined) return { entry };
  const files = resolveProjectPath(projectRoot, plugin.files, "vitadeck.config.json", "hostPlugin.files");
  const isFolder = await stat(files).then(
    (info) => info.isDirectory(),
    () => false,
  );
  if (files === projectRoot || !isFolder) {
    throw new Error(`vitadeck.config.json "hostPlugin.files" must name a folder inside the project, not ${files}.`);
  }
  return { entry, files };
}

async function buildDeckApp(
  projectRoot: string,
  config: DeckAppConfig & { entry: string },
  version: string,
  packageDir: string,
  options: BuildOptions,
): Promise<void> {
  await validateReact(projectRoot);
  const entryPath = path.resolve(projectRoot, config.entry);
  const generatedDir = path.join(projectRoot, ".vitadeck");
  const generatedEntry = path.join(generatedDir, "package-entry.tsx");

  await rm(packageDir, { recursive: true, force: true });
  await rm(`${packageDir}.zip`, { force: true });
  await mkdir(packageDir, { recursive: true });
  await mkdir(generatedDir, { recursive: true });
  await writeFontTypes(generatedDir, config.fonts);
  await writeImageTypes(generatedDir, config.images);

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
        "process.env.NODE_ENV": JSON.stringify(options.dev ? "development" : "production"),
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
  const manifestImages = await copyImages(projectRoot, packageDir, config.images);

  const manifest = {
    schemaVersion: PACKAGE_SCHEMA_VERSION,
    name: config.name,
    version,
    entry: "app.js",
    ...(manifestFonts ? { fonts: manifestFonts } : {}),
    ...(manifestImages ? { images: manifestImages } : {}),
  } satisfies DeckAppPackageManifest;
  await writeFile(path.join(packageDir, "manifest.json"), JSON.stringify(manifest, null, 2) + "\n");
  await rm(generatedEntry, { force: true });
}

/** Builds the project's Deck App, its Host Control Plugin, or both, with their archives unless `noZip`. */
async function build(projectRoot: string, options: BuildOptions = {}): Promise<BuildOutput> {
  const config = await readConfig(projectRoot);
  validateDisplayName(config.name, "vitadeck.config.json");
  const version = await readPackageVersion(projectRoot, config);
  const outDir = path.resolve(projectRoot, config.outDir);
  const packageFolderName = packageNameFor(config.name);
  const hostPlugin = config.hostPlugin && (await resolveHostPlugin(projectRoot, config.hostPlugin));
  const output: BuildOutput = {};

  const { entry } = config;
  if (entry !== undefined) {
    output.packageDir = path.join(outDir, packageFolderName);
    await buildDeckApp(projectRoot, { ...config, entry }, version, output.packageDir, options);
  }
  if (hostPlugin) {
    output.hostPluginDir = path.join(outDir, `${packageFolderName.replace(/\.vdapp$/u, "")}.vdplugin`);
    await buildHostPlugin(hostPlugin, output.hostPluginDir, { name: config.name, version });
  }
  if (options.noZip) return output;
  if (output.packageDir) output.archivePath = `${output.packageDir}.zip`;
  if (output.hostPluginDir) output.hostPluginArchivePath = `${output.hostPluginDir}.zip`;
  await Promise.all(
    [output.packageDir, output.hostPluginDir]
      .filter((dir) => dir !== undefined)
      .map((dir) => writePackageArchive(dir, `${dir}.zip`)),
  );
  return output;
}

const DEFAULT_UPLOAD_PORT = "8787";
const WATCH_QUIET_MS = 150;
const BUSY_RETRY_MS = 500;
const UPLOAD_TIMEOUT_MS = 30_000;
const WATCH_IGNORED_DIRS = ["node_modules", ".vitadeck", ".git"];

type UploadReply = { ok: true; packageName: string; version: string } | { ok: false; code: string; message: string };
type UploadAttempt = { kind: "reply"; status: number; reply: UploadReply } | { kind: "busy" };

function parseRuntimeUploadUrl(raw: string): URL {
  const withScheme = /^[a-z][a-z0-9+.-]*:\/\//iu.test(raw) ? raw : `http://${raw}`;
  let url: URL;
  try {
    url = new URL(withScheme);
  } catch (error) {
    throw new Error(`Invalid Runtime Upload URL: ${raw}`, { cause: error });
  }
  if (url.protocol !== "http:") throw new Error(`Runtime Upload URL must use http://, got ${raw}`);
  if (url.port === "") url.port = DEFAULT_UPLOAD_PORT;
  return new URL("/upload", url);
}

function parseUploadReply(status: number, body: string): UploadReply {
  try {
    const json = requireObject(JSON.parse(body), "reply");
    if (json.ok === true && typeof json.packageName === "string" && typeof json.version === "string") {
      return { ok: true, packageName: json.packageName, version: json.version };
    }
    if (json.ok === false && typeof json.code === "string" && typeof json.message === "string") {
      return { ok: false, code: json.code, message: json.message };
    }
  } catch {
    // Not a Runtime Upload HTTP Contract reply; report the raw body below.
  }
  return { ok: false, code: "unexpected_reply", message: body.trim() || `HTTP ${status} with an empty body` };
}

function errorCode(error: unknown): string | undefined {
  for (let current = error; current instanceof Error; current = current.cause) {
    if ("code" in current && typeof current.code === "string") return current.code;
    if (current instanceof AggregateError) return errorCode(current.errors[0]);
  }
  return undefined;
}

async function postArchive(uploadUrl: URL, archive: Uint8Array, signal: AbortSignal): Promise<UploadAttempt> {
  let response: Response;
  try {
    response = await fetch(uploadUrl, {
      method: "POST",
      headers: { "Content-Type": "application/zip" },
      body: archive,
      signal: AbortSignal.any([signal, AbortSignal.timeout(UPLOAD_TIMEOUT_MS)]),
    });
  } catch (error) {
    if (signal.aborted) throw error;
    const code = errorCode(error);
    if (code === "ECONNREFUSED") {
      throw new Error(
        `Nothing is listening at ${uploadUrl.origin}. Check the address under Upload and link in the VitaDeck Shell, and that it is on.`,
        {
          cause: error,
        },
      );
    }
    // The listener may close the socket on a rejected upload without reading the body (#79).
    if (code === "ECONNRESET" || code === "EPIPE" || code === "UND_ERR_SOCKET") return { kind: "busy" };
    if (error instanceof DOMException && error.name === "TimeoutError") {
      throw new Error(`No reply from ${uploadUrl.origin} within ${UPLOAD_TIMEOUT_MS / 1000}s.`, { cause: error });
    }
    const reason = code ?? (error instanceof Error ? error.message : String(error));
    throw new Error(`Could not reach ${uploadUrl.origin} (${reason}).`, { cause: error });
  }
  let body = "";
  try {
    body = await response.text();
  } catch {
    // Reply cut short; fall through to the status code alone.
  }
  if (response.status === 409) return { kind: "busy" };
  return { kind: "reply", status: response.status, reply: parseUploadReply(response.status, body) };
}

async function uploadArchive(uploadUrl: URL, archive: Uint8Array, signal: AbortSignal): Promise<string> {
  let attempt = await postArchive(uploadUrl, archive, signal);
  if (attempt.kind === "busy") {
    await delay(BUSY_RETRY_MS, undefined, { signal });
    attempt = await postArchive(uploadUrl, archive, signal);
  }
  if (attempt.kind === "busy") return "busy: another Runtime Upload is in progress, retrying on the next change";
  const { status, reply } = attempt;
  return reply.ok ? `installed ${reply.packageName} ${reply.version}` : `${status} ${reply.code}: ${reply.message}`;
}

function logCycle(line: string): void {
  console.log(`[${new Date().toLocaleTimeString("en-GB")}] ${line}`);
}

async function dev(projectRoot: string, rawUrl: string | undefined): Promise<void> {
  const hasDeckApp = (await readConfig(projectRoot)).entry !== undefined;
  if (hasDeckApp !== (rawUrl !== undefined)) {
    throw new Error(
      hasDeckApp
        ? "Usage: vitadeck dev <runtime-upload-url>"
        : "This project has no Deck App to upload; run vitadeck dev without a URL.",
    );
  }
  const uploadUrl = rawUrl === undefined ? undefined : parseRuntimeUploadUrl(rawUrl);
  const session = new AbortController();
  let outDirRel: string | undefined;
  let running = false;
  let pending = false;
  let quietTimer: NodeJS.Timeout | undefined;

  async function cycle(): Promise<void> {
    running = true;
    try {
      // Known before build() writes into it, so the build's own output does not retrigger the watcher.
      outDirRel = path.relative(projectRoot, path.resolve(projectRoot, (await readConfig(projectRoot)).outDir));
      const output = await build(projectRoot, { dev: true });
      if (output.hostPluginDir) logCycle(`built ${path.relative(projectRoot, output.hostPluginDir)}`);
      if (uploadUrl && output.archivePath) {
        logCycle(await uploadArchive(uploadUrl, await readFile(output.archivePath), session.signal));
      }
    } catch (error) {
      if (!session.signal.aborted) logCycle(error instanceof Error ? error.message : String(error));
    } finally {
      running = false;
    }
    if (pending && !session.signal.aborted) {
      pending = false;
      await cycle();
    }
  }

  function isIgnored(filename: string): boolean {
    const ignored = outDirRel === undefined ? WATCH_IGNORED_DIRS : [...WATCH_IGNORED_DIRS, outDirRel];
    return ignored.some((dir) => filename === dir || filename.startsWith(`${dir}${path.sep}`));
  }

  const watcher = watch(projectRoot, { recursive: true, signal: session.signal }, (_event, filename) => {
    if (filename !== null && isIgnored(filename)) return;
    clearTimeout(quietTimer);
    quietTimer = setTimeout(() => {
      if (running) pending = true;
      else void cycle();
    }, WATCH_QUIET_MS);
  });
  let watchError: Error | undefined;
  watcher.on("error", (error) => {
    watchError = error;
    session.abort();
  });

  const stopped = new Promise<void>((resolve) => {
    session.signal.addEventListener(
      "abort",
      () => {
        clearTimeout(quietTimer);
        resolve();
      },
      { once: true },
    );
  });
  const stop = (): void => {
    session.abort();
  };
  process.once("SIGINT", stop);
  process.once("SIGTERM", stop);

  console.log(`Watching ${projectRoot}${uploadUrl ? `, uploading to ${uploadUrl.href}` : ""} (Ctrl-C to stop)`);
  void cycle();
  await stopped;
  if (watchError) throw new Error(`Watching ${projectRoot} failed: ${watchError.message}`, { cause: watchError });
}

async function isNonEmptyDir(dir: string): Promise<boolean> {
  try {
    return (await readdir(dir)).length > 0;
  } catch (error) {
    if (errorCode(error) === "ENOENT") return false;
    throw error;
  }
}

async function create(
  projectName: string,
  options: { force: boolean; hostPlugin: boolean; hostPluginOnly: boolean },
): Promise<void> {
  const targetDir = path.resolve(process.cwd(), projectName);
  const slug = packageNameFor(projectName).replace(/\.vdapp$/u, "");
  const templateRoot = fileURLToPath(new URL("./templates/scaffold", import.meta.url));
  if (!options.force && (await isNonEmptyDir(targetDir))) {
    throw new Error(`${targetDir} is not empty. Pick another name or pass --force to write into it.`);
  }
  await mkdir(targetDir, { recursive: true });
  const hostPluginRoot = fileURLToPath(new URL("./templates/host-plugin", import.meta.url));
  if (options.hostPluginOnly) {
    // A plugin without a Deck App: the same host/ folder, and a package.json that exports its commands.
    await copyScaffoldDir(fileURLToPath(new URL("./templates/host-plugin-only", import.meta.url)), targetDir, slug);
    await copyScaffoldDir(hostPluginRoot, targetDir, slug, "host");
  } else {
    await copyScaffoldDir(templateRoot, targetDir, slug);
    if (options.hostPlugin) await applyTemplateOverlay(hostPluginRoot, targetDir, slug);
  }
  console.log(`Created ${targetDir}`);
}

const USAGE = `Usage: vitadeck <command> [flags]

Commands:
  build                     Build the Deck App Package, .vdapp.zip and any .vdplugin.zip (default command)
    --no-zip                Skip the .zip archives
    --dev                   Bundle with NODE_ENV=development
  dev <runtime-upload-url>  Watch, rebuild with --dev and upload to a running VitaDeck; a project with only a
                            Host Plugin takes no URL and just rebuilds
  create <project-name>     Scaffold a new Deck App project
    --force                 Write into a non-empty directory
    --host-plugin           Add a Host Plugin: commands the Deck App sends to a computer
    --host-plugin-only      Scaffold a Host Plugin without a Deck App, for Deck Apps to depend on

Flags:
  --help, -h                Show this help
  --version, -v             Print the SDK version`;

const COMMAND_FLAGS = {
  build: ["--no-zip", "--dev"],
  dev: [],
  create: ["--force", "--host-plugin", "--host-plugin-only"],
} as const satisfies Record<string, readonly string[]>;

type Command = keyof typeof COMMAND_FLAGS;

function isCommand(value: string): value is Command {
  return Object.hasOwn(COMMAND_FLAGS, value);
}

async function sdkVersion(): Promise<string> {
  const packagePath = fileURLToPath(new URL("../package.json", import.meta.url));
  return requireString(requireObject(await readJson(packagePath), packagePath).version, "version", packagePath);
}

async function main(): Promise<void> {
  const rawArgs = process.argv.slice(2);
  if (rawArgs.includes("--help") || rawArgs.includes("-h")) {
    console.log(USAGE);
    return;
  }
  if (rawArgs.includes("--version") || rawArgs.includes("-v")) {
    console.log(await sdkVersion());
    return;
  }
  const flags = new Set(rawArgs.filter((a) => a.startsWith("-")));
  const [command = "build", arg] = rawArgs.filter((a) => !a.startsWith("-"));
  if (!isCommand(command)) throw new Error(`Unknown command: ${command}\n\n${USAGE}`);
  const knownFlags: readonly string[] = COMMAND_FLAGS[command];
  const unknownFlags = [...flags].filter((f) => !knownFlags.includes(f));
  if (unknownFlags.length > 0) throw new Error(`Unknown flag(s) for ${command}: ${unknownFlags.join(", ")}`);

  const projectRoot = process.cwd();
  switch (command) {
    case "build": {
      const output = await build(projectRoot, { noZip: flags.has("--no-zip"), dev: flags.has("--dev") });
      for (const [verb, file] of [
        ["Built", output.packageDir],
        ["Wrote", output.archivePath],
        ["Built", output.hostPluginDir],
        ["Wrote", output.hostPluginArchivePath],
      ] as const) {
        if (file !== undefined) console.log(`${verb} ${path.relative(projectRoot, file)}`);
      }
      return;
    }
    case "dev": {
      await dev(projectRoot, arg);
      return;
    }
    case "create": {
      if (flags.has("--host-plugin") && flags.has("--host-plugin-only")) {
        throw new Error("Pass --host-plugin or --host-plugin-only, not both.");
      }
      if (!arg) throw new Error("Usage: vitadeck create <project-name>");
      await create(arg, {
        force: flags.has("--force"),
        hostPlugin: flags.has("--host-plugin"),
        hostPluginOnly: flags.has("--host-plugin-only"),
      });
    }
  }
}

try {
  await main();
} catch (error: unknown) {
  const message = error instanceof Error ? error.message : String(error);
  console.error(message);
  process.exit(1);
}
