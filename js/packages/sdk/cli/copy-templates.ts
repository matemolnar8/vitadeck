import { copyFile, mkdir, readFile, readdir, rm, writeFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const pkgRoot = fileURLToPath(new URL("..", import.meta.url));
const srcRoot = path.join(pkgRoot, "cli/templates");
const destRoot = path.join(pkgRoot, "dist/templates");

const sdkPkg = JSON.parse(await readFile(path.join(pkgRoot, "package.json"), "utf8")) as { version: string };
const sdkVersion = sdkPkg.version;

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
        const relPosix = childRel.split(path.sep).join("/");
        const isScaffoldPkg = ent.name === "package.json" && relPosix === "scaffold/package.json";
        if (isScaffoldPkg) {
          const raw = await readFile(src, "utf8");
          const pkg = JSON.parse(raw) as { dependencies?: Record<string, string> };
          if (pkg.dependencies?.["@vitadeck/sdk"] === "workspace:*") {
            pkg.dependencies["@vitadeck/sdk"] = `^${sdkVersion}`;
          }
          await writeFile(dest, `${JSON.stringify(pkg, null, 2)}\n`);
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
