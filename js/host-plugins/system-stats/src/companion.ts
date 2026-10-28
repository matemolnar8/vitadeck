import { execFile } from "node:child_process";
import os from "node:os";
import { setTimeout as sleep } from "node:timers/promises";
import { implementHostPlugin } from "@vitadeck/host-companion";
import { systemStatsPlugin } from "./index";

type CpuSample = { idle: number; total: number };

function sampleCpus(): CpuSample {
  let idle = 0;
  let total = 0;
  for (const { times } of os.cpus()) {
    idle += times.idle;
    total += times.user + times.nice + times.sys + times.idle + times.irq;
  }
  return { idle, total };
}

const roundTenth = (value: number) => Math.round(value * 10) / 10;

let previous: CpuSample | undefined;

/** Busy share of CPU time since the previous read; the first read samples twice, 200 ms apart. */
async function cpuPercent(): Promise<number> {
  if (!previous) {
    previous = sampleCpus();
    await sleep(200);
  }
  const current = sampleCpus();
  const total = current.total - previous.total;
  const idle = current.idle - previous.idle;
  previous = current;
  return total > 0 ? roundTenth(((total - idle) / total) * 100) : 0;
}

// Node counts macOS's reclaimable file cache as used, so a Mac would always read near 100%. vm_stat's free, inactive
// and speculative pages are what Activity Monitor treats as available.
async function availableMemoryBytes(): Promise<number> {
  if (process.platform !== "darwin") return os.freemem();
  const stdout = await new Promise<string>((resolve, reject) => {
    execFile("vm_stat", (error, output) => {
      if (error) reject(new Error(`vm_stat failed: ${error.message}`));
      else resolve(output);
    });
  });
  const pageSize = Number(/page size of (\d+) bytes/u.exec(stdout)?.[1] ?? 4096);
  const pages = (label: string) => Number(new RegExp(`${label}:\\s+(\\d+)`, "u").exec(stdout)?.[1] ?? 0);
  return (pages("Pages free") + pages("Pages inactive") + pages("Pages speculative")) * pageSize;
}

export default implementHostPlugin(systemStatsPlugin, {
  read: async () => {
    const memoryTotalBytes = os.totalmem();
    return {
      cpuPercent: await cpuPercent(),
      memoryUsedPercent: roundTenth(((memoryTotalBytes - (await availableMemoryBytes())) / memoryTotalBytes) * 100),
      memoryTotalBytes,
      uptimeSeconds: Math.floor(os.uptime()),
    };
  },
});
