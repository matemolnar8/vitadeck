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

export default implementHostPlugin(systemStatsPlugin, {
  read: async () => {
    const memoryTotalBytes = os.totalmem();
    return {
      cpuPercent: await cpuPercent(),
      memoryUsedPercent: roundTenth(((memoryTotalBytes - os.freemem()) / memoryTotalBytes) * 100),
      memoryTotalBytes,
      uptimeSeconds: Math.floor(os.uptime()),
    };
  },
});
