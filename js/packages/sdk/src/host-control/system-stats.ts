import { command, defineHostPlugin, isAbsent } from "./plugin";

export type SystemStats = {
  cpuPercent: number;
  memoryUsedPercent: number;
  memoryTotalBytes: number;
  uptimeSeconds: number;
};

/** Built into every Host Companion: read-only gauges of the computer. */
export const systemStatsPlugin = defineHostPlugin("systemStats", {
  read: command<undefined, SystemStats>(isAbsent),
});
