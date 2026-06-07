import * as vitadeckSdk from "@vitadeck/sdk";
import { ThemeProvider } from "@vitadeck/sdk/internal";
import React, { StrictMode, type ComponentType } from "react";
import * as reactCompilerRuntime from "react-compiler-runtime";
import { installFetch } from "./fetch";
import { onInputEventFromNative } from "./input";
import type { MetricSummary } from "./reconciler-metrics";
import { getMetrics, render, resetMetrics } from "./vitadeck-react-reconciler-mutation";

type DeckAppPackageManifest = {
  schemaVersion: number;
  name: string;
  version: string;
  entry: string;
  fonts?: Record<string, string>;
};

let deckAppComponent: ComponentType | null = null;
let packageLoaded = false;

globalThis.React = React;
globalThis.reactCompilerRuntime = reactCompilerRuntime;
globalThis.vitadeckSdk = vitadeckSdk;
installFetch();
globalThis.vitadeckPackage = {
  register(component) {
    deckAppComponent = component;
  },
};

function readManifest(): DeckAppPackageManifest {
  const packagePath = nativeGetActiveDeckAppPath();
  const raw = nativeReadTextFile(`${packagePath}/manifest.json`);
  const manifest = JSON.parse(raw) as Partial<DeckAppPackageManifest>;
  if (manifest.schemaVersion !== 1) {
    throw new Error(`Unsupported Deck App package schema: ${String(manifest.schemaVersion)}`);
  }
  if (typeof manifest.name !== "string" || manifest.name.length === 0) {
    throw new Error("Deck App package manifest must define a name.");
  }
  if (typeof manifest.version !== "string" || manifest.version.length === 0) {
    throw new Error("Deck App package manifest must define a version.");
  }
  if (manifest.entry !== "app.js") {
    throw new Error("Deck App package manifest entry must be app.js.");
  }
  return manifest as DeckAppPackageManifest;
}

function loadDeckApp(): ComponentType {
  if (deckAppComponent) return deckAppComponent;
  if (packageLoaded) throw new Error("Deck App package did not register a component.");

  const manifest = readManifest();
  packageLoaded = true;
  nativeEvalFile(`${nativeGetActiveDeckAppPath()}/${manifest.entry}`);
  if (!deckAppComponent) {
    throw new Error(`Deck App package "${manifest.name}" did not register a component.`);
  }
  return deckAppComponent;
}

export function updateContainer() {
  const DeckApp = loadDeckApp();
  render(
    <StrictMode>
      <ThemeProvider>
        <DeckApp />
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

console.debug("runtime.tsx loaded");
