import React, { useCallback, useEffect, useMemo, useRef, useState } from "react";
import {
  getActiveReconciler,
  getAvailableReconcilers,
  getReconcilerMetrics,
  type ReconcilerDescriptor,
  type ReconcilerId,
  resetMetrics,
  setActiveReconciler,
} from "../reconciler-manager";
import type { MetricSummary } from "../reconciler-metrics";
import { App } from "./App";
import { Benchmark } from "./pages/Benchmark";
import { useTheme } from "./theme";

type LauncherMode = "demo" | "benchmark";

type MetricsByReconciler = Record<ReconcilerId, Record<string, MetricSummary>>;

const PANEL_WIDTH = 260;
const PANEL_HEIGHT = 504;
const PANEL_X = 680;
const PANEL_Y = 20;
const METRICS_REFRESH_MS = 1500;
const METRICS_LOG_LIMIT = 10;

export const MainLauncher = () => {
  const { theme } = useTheme();
  const [mode, setMode] = useState<LauncherMode>("demo");
  const availableReconcilers = useMemo<ReconcilerDescriptor[]>(() => getAvailableReconcilers(), []);
  const [activeReconcilerId, setActiveReconcilerId] = useState<ReconcilerId>(() => {
    return getActiveReconciler().id;
  });

  const collectMetrics = useCallback((): MetricsByReconciler => {
    const summary: Partial<MetricsByReconciler> = {};
    for (const { id } of availableReconcilers) {
      summary[id] = getReconcilerMetrics(id).getSummary();
    }
    return summary as MetricsByReconciler;
  }, [availableReconcilers]);

  const [metrics, setMetrics] = useState<MetricsByReconciler>(() => collectMetrics());
  const loggedSignatureRef = useRef<Record<ReconcilerId, string>>({} as Record<ReconcilerId, string>);

  useEffect(() => {
    const interval = setInterval(() => {
      setMetrics(collectMetrics());
    }, METRICS_REFRESH_MS);
    return () => {
      clearInterval(interval);
    };
  }, [collectMetrics]);

  useEffect(() => {
    const signatureStore = loggedSignatureRef.current;
    for (const descriptor of availableReconcilers) {
      const summary = metrics[descriptor.id];
      if (!summary) continue;
      const pairs: Array<{ method: string; stats: MetricSummary }> = [];
      for (const method in summary) {
        const stats = summary[method];
        if (!stats) continue;
        pairs.push({ method, stats });
      }
      if (pairs.length === 0) continue;
      pairs.sort((left, right) => right.stats.totalMs - left.stats.totalMs);
      const trimmed = pairs.slice(0, METRICS_LOG_LIMIT);
      let signature = "";
      for (let i = 0; i < trimmed.length; i++) {
        const entry = trimmed[i];
        if (!entry) continue;
        signature +=
          entry.method +
          ":" +
          entry.stats.totalMs.toFixed(3) +
          ":" +
          entry.stats.count +
          ":" +
          entry.stats.avgMs.toFixed(3) +
          "|";
      }
      if (signatureStore[descriptor.id] === signature) continue;
      signatureStore[descriptor.id] = signature;

      console.log("----------------------------------------");
      console.log(`[ReconcilerMetrics][${descriptor.label}] hotspots (top ${trimmed.length}, sorted by total time)`);
      for (let i = 0; i < trimmed.length; i++) {
        const entry = trimmed[i];
        if (!entry) continue;
        console.log(
          "  " +
            entry.method +
            " :: count=" +
            entry.stats.count +
            " total=" +
            entry.stats.totalMs.toFixed(3) +
            "ms avg=" +
            entry.stats.avgMs.toFixed(3) +
            "ms min=" +
            entry.stats.minMs.toFixed(3) +
            "ms max=" +
            entry.stats.maxMs.toFixed(3) +
            "ms",
        );
      }
      console.log("----------------------------------------");
    }
  }, [metrics, availableReconcilers]);

  const handleModeChange = useCallback((nextMode: LauncherMode) => {
    setMode(nextMode);
  }, []);

  const handleSwitchReconciler = useCallback(
    (nextId: ReconcilerId) => {
      if (nextId === activeReconcilerId) return;
      setActiveReconciler(nextId);
      setActiveReconcilerId(nextId);
      setMetrics(collectMetrics());
    },
    [activeReconcilerId, collectMetrics],
  );

  const handleResetMetrics = useCallback(
    (targetId: ReconcilerId) => {
      resetMetrics(targetId);
      setMetrics(collectMetrics());
    },
    [collectMetrics],
  );

  const content = mode === "demo" ? <App /> : <Benchmark />;

  const resetButtonOffsetY = 220 + availableReconcilers.length * 50;
  const metricsPanelOffsetY = 270 + availableReconcilers.length * 50;
  const metricsPanelHeight = Math.max(120, PANEL_HEIGHT - metricsPanelOffsetY - 20);

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={theme.background}>
      {content}

      <vita-rect
        x={PANEL_X}
        y={PANEL_Y}
        width={PANEL_WIDTH}
        height={PANEL_HEIGHT}
        color={theme.surface}
        borderColor={theme.navBackground}
      >
        <vita-text fontSize={24} color={theme.text}>
          Launcher
        </vita-text>
        <vita-text fontSize={18} color={theme.text}>
          Mode
        </vita-text>
        <vita-button
          x={20}
          y={70}
          width={100}
          height={40}
          color={mode === "demo" ? theme.buttonBackground : Colors.DARKGRAY}
          label={"Demo"}
          onClick={() => handleModeChange("demo")}
        />
        <vita-button
          x={140}
          y={70}
          width={100}
          height={40}
          color={mode === "benchmark" ? theme.buttonBackground : Colors.DARKGRAY}
          label={"Benchmark"}
          onClick={() => handleModeChange("benchmark")}
        />

        <vita-text fontSize={18} color={theme.text}>
          Reconciler
        </vita-text>
        {availableReconcilers.map(({ id, label }, index) => (
          <vita-button
            key={id}
            x={20}
            y={140 + index * 50}
            width={PANEL_WIDTH - 40}
            height={40}
            color={activeReconcilerId === id ? theme.buttonBackground : Colors.DARKGRAY}
            label={label}
            onClick={() => handleSwitchReconciler(id)}
          />
        ))}

        <vita-button
          x={20}
          y={resetButtonOffsetY}
          width={PANEL_WIDTH - 40}
          height={36}
          color={Colors.DARKBLUE}
          label={"Reset Metrics"}
          onClick={() => handleResetMetrics(activeReconcilerId)}
        />

        <vita-rect
          x={20}
          y={metricsPanelOffsetY}
          width={PANEL_WIDTH - 40}
          height={metricsPanelHeight}
          color={theme.background}
          borderColor={theme.navBackground}
        >
          <vita-text fontSize={18} color={theme.text}>
            Metrics ({activeReconcilerId})
          </vita-text>
          <vita-text fontSize={14} color={theme.text}>
            Hot spot metrics are logged to the console.
          </vita-text>
          <vita-text fontSize={12} color={theme.text}>
            Logs refresh every {METRICS_REFRESH_MS / 1000} seconds and include top {METRICS_LOG_LIMIT} methods.
          </vita-text>
        </vita-rect>
      </vita-rect>
    </vita-rect>
  );
};
