import { copyFileSync, existsSync } from "node:fs";
import { join } from "node:path";

const root = join(import.meta.dirname, "..");
const target = join(root, "src/genesys/genesys.local.ts");
const template = join(root, "src/genesys/genesys.local.example.ts");

if (!existsSync(target)) {
  copyFileSync(template, target);
}
