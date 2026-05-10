import { copyFile, mkdir, readdir, rm } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const pkgRoot = fileURLToPath(new URL("..", import.meta.url));
const srcRoot = path.join(pkgRoot, "cli/templates");
const destRoot = path.join(pkgRoot, "dist/templates");

function skipTemplateDir(name: string): boolean {
  return name === "node_modules" || name === "dist";
}

async function copyTree(rel = ""): Promise<void> {
  const srcDir = path.join(srcRoot, rel);
  const entries = await readdir(srcDir, { withFileTypes: true });
  for (const ent of entries) {
    if (ent.isDirectory() && skipTemplateDir(ent.name)) continue;
    const childRel = rel ? path.join(rel, ent.name) : ent.name;
    const src = path.join(srcRoot, childRel);
    const dest = path.join(destRoot, childRel);
    if (ent.isDirectory()) {
      await mkdir(dest, { recursive: true });
      await copyTree(childRel);
    } else if (ent.isFile()) {
      await mkdir(path.dirname(dest), { recursive: true });
      await copyFile(src, dest);
    }
  }
}

await rm(destRoot, { recursive: true, force: true });
await copyTree();
