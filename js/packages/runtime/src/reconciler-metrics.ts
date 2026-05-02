type InternalMetricEntry = {
  count: number;
  totalMs: number;
  minMs: number;
  maxMs: number;
  lastMs: number;
};

export type MetricSummary = InternalMetricEntry & {
  avgMs: number;
};

function now(): number {
  if (typeof getTime === "function") {
    return getTime() * 1000;
  }
  if (typeof Date !== "undefined" && typeof Date.now === "function") {
    return Date.now();
  }
  return 0;
}

function isPromiseLike<T = unknown>(value: unknown): value is PromiseLike<T> {
  return value !== null && typeof value === "object" && typeof (value as PromiseLike<T>).then === "function";
}

export class ReconcilerMetrics {
  private readonly entries = new Map<string, InternalMetricEntry>();

  constructor(public readonly label: string) {}

  reset(methodName?: string) {
    if (!methodName) {
      this.entries.clear();
      return;
    }
    this.entries.delete(methodName);
  }

  getSummary(): Record<string, MetricSummary> {
    const summary: Record<string, MetricSummary> = {};
    for (const [methodName, entry] of this.entries.entries()) {
      summary[methodName] = {
        ...entry,
        avgMs: entry.totalMs / entry.count,
      };
    }
    return summary;
  }

  getSummaryFor(methodName: string): MetricSummary | undefined {
    const entry = this.entries.get(methodName);
    if (!entry) return undefined;
    return {
      ...entry,
      avgMs: entry.totalMs / entry.count,
    };
  }

  getMethodNames(): string[] {
    return Array.from(this.entries.keys());
  }

  record(methodName: string, durationMs: number) {
    if (!Number.isFinite(durationMs)) {
      return;
    }
    const existing = this.entries.get(methodName);
    if (!existing) {
      this.entries.set(methodName, {
        count: 1,
        totalMs: durationMs,
        minMs: durationMs,
        maxMs: durationMs,
        lastMs: durationMs,
      });
      return;
    }

    existing.count += 1;
    existing.totalMs += durationMs;
    existing.lastMs = durationMs;
    if (durationMs < existing.minMs) existing.minMs = durationMs;
    if (durationMs > existing.maxMs) existing.maxMs = durationMs;
  }

  time<F extends (...args: unknown[]) => unknown>(
    methodName: string,
    fn: F,
    thisArg: unknown,
    args: Parameters<F>,
  ): ReturnType<F> {
    const start = now();
    try {
      const result = fn.apply(thisArg, args) as ReturnType<F>;
      if (isPromiseLike(result)) {
        const promise = Promise.resolve(result);
        return promise.then(
          (value) => {
            this.record(methodName, now() - start);
            return value;
          },
          (error) => {
            this.record(methodName, now() - start);
            throw error;
          },
        ) as ReturnType<F>;
      }
      this.record(methodName, now() - start);
      return result;
    } catch (error) {
      this.record(methodName, now() - start);
      throw error;
    }
  }
}

type AnyRecord = Record<string, unknown>;

export function instrumentHostConfig<T extends AnyRecord>(
  hostConfig: T,
  metrics: ReconcilerMetrics,
  prefix?: string,
): T {
  const instrumented: AnyRecord = { ...hostConfig };
  for (const key of Object.keys(hostConfig) as Array<keyof T>) {
    const value = hostConfig[key];
    if (typeof value !== "function") continue;
    const methodName = prefix ? `${prefix}.${String(key)}` : String(key);
    const original = value as (...fnArgs: unknown[]) => unknown;
    instrumented[key as string] = function instrumentedMethod(this: unknown, ...fnArgs: unknown[]) {
      return metrics.time(methodName, original, this, fnArgs);
    };
  }
  return instrumented as T;
}
