import React, { StrictMode } from "react";
import type { MetricSummary } from "./reconciler-metrics";
import { App } from "./app/App";
import { onInputEventFromNative } from "./input";
import { ThemeProvider } from "./app/theme";
import { getMetrics, render, resetMetrics } from "./vitadeck-react-reconciler-mutation";

export function updateContainer() {
  render(
    <StrictMode>
      <ThemeProvider>
        <App />
      </ThemeProvider>
    </StrictMode>,
  );
}

export const input = {
  onInputEventFromNative,
};

const METRICS_REFRESH_MS = 1500;
const METRICS_LOG_LIMIT = 10;
let metricsLogSignature = "";
let metricsIntervalId: number | null = null;

function logMetrics() {
  const summary = getMetrics().getSummary();
  const pairs: Array<{ method: string; stats: MetricSummary }> = [];
  for (const method in summary) {
    const stats = summary[method];
    if (stats) pairs.push({ method, stats });
  }
  if (pairs.length === 0) return;

  pairs.sort((a, b) => b.stats.totalMs - a.stats.totalMs);
  const trimmed = pairs.slice(0, METRICS_LOG_LIMIT);

  let signature = "";
  for (const entry of trimmed) {
    signature += `${entry.method}:${entry.stats.totalMs.toFixed(3)}:${entry.stats.count}|`;
  }
  if (metricsLogSignature === signature) return;
  metricsLogSignature = signature;

  console.log("----------------------------------------");
  console.log(`[ReconcilerMetrics] hotspots (top ${trimmed.length}, sorted by total time)`);
  for (const entry of trimmed) {
    console.log(
      `  ${entry.method} :: count=${entry.stats.count} total=${entry.stats.totalMs.toFixed(3)}ms avg=${entry.stats.avgMs.toFixed(3)}ms min=${entry.stats.minMs.toFixed(3)}ms max=${entry.stats.maxMs.toFixed(3)}ms`,
    );
  }
  console.log("----------------------------------------");
}

function startMetricsLogging() {
  if (metricsIntervalId !== null) return;
  metricsIntervalId = setInterval(logMetrics, METRICS_REFRESH_MS) as unknown as number;
}

function stopMetricsLogging() {
  if (metricsIntervalId === null) return;
  clearInterval(metricsIntervalId);
  metricsIntervalId = null;
  metricsLogSignature = "";
}

export const metrics = {
  get: getMetrics,
  reset: resetMetrics,
  startLogging: startMetricsLogging,
  stopLogging: stopMetricsLogging,
  isLogging: () => metricsIntervalId !== null,
};

console.debug("main.tsx loaded");
