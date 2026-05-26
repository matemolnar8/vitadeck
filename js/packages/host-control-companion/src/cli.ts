#!/usr/bin/env node
import { configPath, resolveVitaUrl, saveConfig } from "./config.js";
import { runVitaSession } from "./vita-session.js";

function parseArgs(argv: string[]): { vita?: string; save?: boolean } {
  let vita: string | undefined;
  let save = false;
  for (let i = 2; i < argv.length; i++) {
    const arg = argv[i];
    if (arg === "--vita" && argv[i + 1]) {
      vita = argv[++i];
      save = true;
    } else if (arg === "start") {
      continue;
    } else if (arg?.startsWith("--vita=")) {
      vita = arg.slice("--vita=".length);
      save = true;
    }
  }
  return { vita, save };
}

const { vita: cliVita, save } = parseArgs(process.argv);
const vitaUrl = resolveVitaUrl(cliVita);

if (save && cliVita) {
  saveConfig({ vitaUrl });
  console.log(`Saved Vita URL to ${configPath()}`);
}

console.log("VitaDeck Host Control Companion");
console.log(`Vita LAN HTTP URL: ${vitaUrl}`);
console.log("Long-poll session (Ctrl+C to exit)");

await runVitaSession(vitaUrl);
