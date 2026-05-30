#!/usr/bin/env node
import os from "node:os";

import { createDefaultGateway } from "./commands/index.js";
import { configPath, resolveVitaUrl, saveConfig } from "./config.js";
import { startHostControlServer } from "./http-server.js";
import { linkToVitaWithRetry } from "./link.js";

function parseArgs(argv: string[]): { vita?: string; save?: boolean; callbackHost?: string } {
  let vita: string | undefined;
  let save = false;
  let callbackHost: string | undefined;
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
    } else if (arg === "--callback-host" && argv[i + 1]) {
      callbackHost = argv[++i];
    } else if (arg?.startsWith("--callback-host=")) {
      callbackHost = arg.slice("--callback-host=".length);
    }
  }
  return { vita, save, callbackHost };
}

const { vita: cliVita, save, callbackHost } = parseArgs(process.argv);
const vitaUrl = resolveVitaUrl(cliVita);

if (save && cliVita) {
  saveConfig({ vitaUrl });
  console.log(`Saved Vita URL to ${configPath()}`);
}

const gateway = createDefaultGateway();
const { url: callbackUrl } = await startHostControlServer(gateway, { callbackHost });

console.log("VitaDeck Host Control Companion");
console.log(`Vita LAN HTTP URL: ${vitaUrl}`);
console.log(`Host Callback URL: ${callbackUrl}`);
console.log("Endpoint: POST /v1/command");
console.log("Linking to Vita (Ctrl+C to exit)...");

await linkToVitaWithRetry(vitaUrl, callbackUrl, os.hostname());

await new Promise(() => {});
