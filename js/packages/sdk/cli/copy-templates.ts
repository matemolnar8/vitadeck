import { copyFile, mkdir, readFile, readdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const pkgRoot = fileURLToPath(new URL("..", import.meta.url));
const srcRoot = path.join(pkgRoot, "cli/templates");
const destRoot = path.join(pkgRoot, "dist/templates");

const sdkPkg = JSON.parse(await readFile(path.join(pkgRoot, "package.json"), "utf8")) as { version: string };
const sdkVersion = sdkPkg.version;
const companionPkg = JSON.parse(await readFile(path.join(pkgRoot, "../host-companion/package.json"), "utf8")) as {
  version: string;
};
const publishedVersions = { "@vitadeck/sdk": sdkVersion, "@vitadeck/host-companion": companionPkg.version };

/** Template packages depend on the workspace; a scaffolded project gets the published versions. */
function withPublishedVersions(json: string): string {
  const pkg = JSON.parse(json) as Partial<Record<"dependencies" | "devDependencies", Record<string, string>>>;
  for (const deps of [pkg.dependencies, pkg.devDependencies]) {
    for (const [name, version] of Object.entries(publishedVersions)) {
      if (deps?.[name] === "workspace:*") deps[name] = `^${version}`;
    }
  }
  return `${JSON.stringify(pkg, null, 2)}\n`;
}

function skipTemplateDir(name: string): boolean {
  return name === "node_modules" || name === "dist";
}

async function copyTree(rel = ""): Promise<void> {
  const srcDir = path.join(srcRoot, rel);
  const entries = await readdir(srcDir, { withFileTypes: true });
  await Promise.all(
    entries.map(async (ent) => {
      if (ent.isDirectory() && skipTemplateDir(ent.name)) return;
      const childRel = rel ? path.join(rel, ent.name) : ent.name;
      const src = path.join(srcRoot, childRel);
      const dest = path.join(destRoot, childRel);
      if (ent.isDirectory()) {
        await mkdir(dest, { recursive: true });
        await copyTree(childRel);
      } else if (ent.isFile()) {
        await mkdir(path.dirname(dest), { recursive: true });
        if (ent.name === "package.json") {
          await writeFile(dest, withPublishedVersions(await readFile(src, "utf8")));
        } else {
          await copyFile(src, dest);
        }
      }
    }),
  );
}

await rm(destRoot, { recursive: true, force: true });
await copyTree();

await copyFile(path.join(pkgRoot, "tsconfig.deck-app.json"), path.join(pkgRoot, "dist", "tsconfig.deck-app.json"));
await copyFile(path.join(pkgRoot, "src", "deck-app-globals.d.ts"), path.join(pkgRoot, "dist", "deck-app-globals.d.ts"));
