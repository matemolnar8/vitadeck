// Bundled into Deck Apps that run on QuickJS: no Node APIs in this file.
import { command, defineHostPlugin, isAbsent } from "@vitadeck/sdk/host-control";

export type SystemStats = {
  cpuPercent: number;
  memoryUsedPercent: number;
  memoryTotalBytes: number;
  uptimeSeconds: number;
};

/** Read-only gauges of the linked computer. */
export const systemStatsPlugin = defineHostPlugin("systemStats", {
  read: command<undefined, SystemStats>(isAbsent),
});
