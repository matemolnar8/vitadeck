// Prints a Benchmark Mode run (scripts/bench.sh) as one table per scenario, next to a baseline run when there is one.
// usage: bench-report <run dir> [baseline dir]
import { existsSync } from "node:fs";
import { readFile } from "node:fs/promises";
import path from "node:path";

type Stats = { mean: number; p50: number; p95: number; p99: number; max: number };

// The shape src/core/bench.c writes.
type Scenario = {
  name: string;
  error?: string;
  instances: number;
  // null where the platform has no per-thread CPU time (Vita3K).
  cpu: { ui_pct: number; js_pct: number | null } | null;
  frames: {
    fps: number;
    dropped: number;
    work: Stats | null;
    begin: Stats | null;
    layout: Stats | null;
    glyphs: Stats | null;
    draw: Stats | null;
    present: Stats | null;
  };
  js: {
    startup_ms: number;
    heap_used_kb: number;
    ms_per_s: { busy: number };
    busy_iteration: Stats | null;
    inputs: number;
    input_latency: Stats | null;
    input_handler: Stats | null;
    commits: number;
    mutations: number;
    commit: Stats | null;
    swap: Stats | null;
  } | null;
};

type Result = { platform: string; clock_overhead_ns: number; measure_s: number; scenarios: Scenario[] };
type Meta = { target: string; commit: string; dirty: boolean; date: string; machine: string };
type Run = { result: Result; meta: Meta | null };

// Lower is better unless noted. A change smaller than noise, in the metric's unit, is not called better or worse.
type Metric = {
  label: string;
  value: (scenario: Scenario) => number | undefined;
  higherIsBetter?: boolean;
  noise?: number;
};

const metrics: Metric[] = [
  { label: "FPS", value: (s) => s.frames.fps, higherIsBetter: true },
  { label: "Dropped frames", value: (s) => s.frames.dropped, noise: 3 },
  { label: "UI work p50 ms", value: (s) => s.frames.work?.p50 },
  { label: "UI work p95 ms", value: (s) => s.frames.work?.p95 },
  { label: "  layout mean ms", value: (s) => s.frames.layout?.mean },
  { label: "  glyphs mean ms", value: (s) => s.frames.glyphs?.mean },
  { label: "  draw mean ms", value: (s) => s.frames.draw?.mean },
  { label: "Begin + present mean ms", value: (s) => sum(s.frames.begin?.mean, s.frames.present?.mean) },
  { label: "UI thread CPU %", value: (s) => s.cpu?.ui_pct },
  { label: "JS thread CPU %", value: (s) => s.cpu?.js_pct ?? undefined },
  { label: "JS busy ms/s", value: (s) => s.js?.ms_per_s.busy, noise: 5 },
  { label: "JS busy iteration p95 ms", value: (s) => s.js?.busy_iteration?.p95 },
  { label: "Inputs", value: (s) => s.js?.inputs },
  { label: "Input latency p50 ms", value: (s) => s.js?.input_latency?.p50 },
  { label: "Input latency p95 ms", value: (s) => s.js?.input_latency?.p95 },
  { label: "Input handler p50 ms", value: (s) => s.js?.input_handler?.p50 },
  { label: "Input handler p95 ms", value: (s) => s.js?.input_handler?.p95 },
  { label: "Commits", value: (s) => s.js?.commits },
  { label: "Commit p95 ms", value: (s) => s.js?.commit?.p95 },
  { label: "Native mutations", value: (s) => s.js?.mutations },
  { label: "Swap p95 ms", value: (s) => s.js?.swap?.p95 },
  { label: "JS startup ms", value: (s) => s.js?.startup_ms },
  { label: "JS heap KB", value: (s) => s.js?.heap_used_kb },
  { label: "Instances", value: (s) => s.instances },
];

function sum(a: number | undefined, b: number | undefined): number | undefined {
  return a === undefined || b === undefined ? undefined : a + b;
}

async function readJson<T>(file: string): Promise<T | null> {
  return existsSync(file) ? (JSON.parse(await readFile(file, "utf8")) as T) : null;
}

async function readRun(dir: string): Promise<Run | null> {
  const result = await readJson<Result>(path.join(dir, "result.json"));
  return result && { result, meta: await readJson<Meta>(path.join(dir, "meta.json")) };
}

function format(value: number | undefined): string {
  if (value === undefined) return "-";
  return Number.isInteger(value) ? String(value) : value.toFixed(value >= 100 ? 1 : value >= 10 ? 2 : 3);
}

function delta(metric: Metric, value: number | undefined, base: number | undefined): string {
  if (value === undefined || base === undefined) return "";
  if (base === 0) return value === 0 ? "" : "new";
  const percent = ((value - base) / base) * 100;
  const change = `${percent > 0 ? "+" : ""}${percent.toFixed(0)}%`;
  // Back-to-back Vita3K runs differ by up to about 10%, and by tens of microseconds on sub-millisecond spans.
  if (Math.abs(percent) < 10 || Math.abs(value - base) < (metric.noise ?? 0.05)) return change;
  const better = metric.higherIsBetter ? percent > 0 : percent < 0;
  return `${change} ${better ? "better" : "worse"}`;
}

function describe(run: Run): string {
  const meta = run.meta;
  const where = meta
    ? `${meta.target} @ ${meta.commit}${meta.dirty ? "+dirty" : ""}, ${meta.date}, ${meta.machine}`
    : "";
  return `${where} (${run.result.measure_s} s per scenario, clock read ${run.result.clock_overhead_ns} ns)`;
}

function table(rows: string[][]): string {
  const widths = rows[0]?.map((_, column) => Math.max(...rows.map((row) => row[column]?.length ?? 0))) ?? [];
  return rows
    .map((row) =>
      row
        .map((cell, column) => cell.padEnd(widths[column] ?? 0))
        .join("  ")
        .trimEnd(),
    )
    .join("\n");
}

const [runDir, baselineDir] = process.argv.slice(2);
if (!runDir) {
  console.error("usage: bench-report <run dir> [baseline dir]");
  process.exit(2);
}
const run = await readRun(runDir);
if (!run) {
  console.error(`No result.json in ${runDir}`);
  process.exit(1);
}
const baseline = baselineDir ? await readRun(baselineDir) : null;

console.log(`Run:      ${describe(run)}`);
if (baseline) {
  console.log(`Baseline: ${describe(baseline)}`);
  if (baseline.meta?.machine !== run.meta?.machine && run.meta?.target !== "vita") {
    console.log("Warning: the baseline ran on another machine, so emulator and host numbers do not compare.");
  }
}

for (const scenario of run.result.scenarios) {
  console.log(`\n${scenario.name}`);
  if (scenario.error) {
    console.log(`  error: ${scenario.error}`);
    continue;
  }
  const base = baseline?.result.scenarios.find((candidate) => candidate.name === scenario.name && !candidate.error);
  const header = base ? ["", "run", "baseline", "change"] : ["", "run"];
  const rows = metrics.map((metric) => {
    const value = metric.value(scenario);
    if (!base) return [`  ${metric.label}`, format(value)];
    const baseValue = metric.value(base);
    return [`  ${metric.label}`, format(value), format(baseValue), delta(metric, value, baseValue)];
  });
  console.log(table([header, ...rows]));
}
