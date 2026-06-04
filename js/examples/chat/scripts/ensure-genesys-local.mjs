import { copyFileSync, existsSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = join(dirname(fileURLToPath(import.meta.url)), "..");
const target = join(root, "src/genesys/genesys.local.ts");
const template = join(root, "src/genesys/genesys.local.example.ts");

if (!existsSync(target)) {
  copyFileSync(template, target);
}
