import fs from "node:fs";
import os from "node:os";
import path from "node:path";

export type HostControlCompanionConfig = {
  vitaUrl: string;
};

const CONFIG_DIR = path.join(os.homedir(), ".vitadeck");
const CONFIG_PATH = path.join(CONFIG_DIR, "host-control.json");

export function loadConfig(): HostControlCompanionConfig | null {
  try {
    const raw = fs.readFileSync(CONFIG_PATH, "utf8");
    const parsed = JSON.parse(raw) as unknown;
    if (!parsed || typeof parsed !== "object") return null;
    const vitaUrl = (parsed as { vitaUrl?: unknown }).vitaUrl;
    if (typeof vitaUrl !== "string" || vitaUrl.length === 0) return null;
    return { vitaUrl: vitaUrl.replace(/\/+$/, "") };
  } catch {
    return null;
  }
}

export function saveConfig(config: HostControlCompanionConfig): void {
  fs.mkdirSync(CONFIG_DIR, { recursive: true });
  fs.writeFileSync(CONFIG_PATH, JSON.stringify({ vitaUrl: config.vitaUrl }, null, 2) + "\n");
}

export function resolveVitaUrl(cliVita?: string): string {
  if (cliVita) return cliVita.replace(/\/+$/, "");
  const file = loadConfig();
  if (!file) {
    throw new Error(
      "No Vita URL configured. Pass --vita http://IP:PORT or save ~/.vitadeck/host-control.json",
    );
  }
  return file.vitaUrl;
}

export function configPath(): string {
  return CONFIG_PATH;
}
