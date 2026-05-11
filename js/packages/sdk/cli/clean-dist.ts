import { rmSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const pkgRoot = fileURLToPath(new URL("..", import.meta.url));
rmSync(path.join(pkgRoot, "dist"), { recursive: true, force: true });
