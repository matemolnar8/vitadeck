import type { HostEventBatch } from "@vitadeck/sdk/host-control";

/** How long a `core.events` request waits for an event. The Vita's native fetch gives up at 30 s. */
export const EVENT_WAIT_MS = 25_000;
const KEPT_EVENTS = 100;

type Waiter = { plugin: string; wake: () => void };

/** The companion's recent events, numbered in order, and the requests waiting for the next one of a plugin. */
export function createEventLog(waitMs = EVENT_WAIT_MS) {
  const events: { seq: number; plugin: string; name: string; data: unknown }[] = [];
  const waiters = new Set<Waiter>();
  let seq = 0;
  let closed = false;

  const pick = (plugin: string, after: number) =>
    events
      .filter((event) => event.plugin === plugin && event.seq > after)
      .map(({ seq: n, name, data }) => ({ seq: n, name, data }));

  return {
    emit(plugin: string, name: string, data: unknown): void {
      events.push({ seq: ++seq, plugin, name, data });
      if (events.length > KEPT_EVENTS) events.shift();
      for (const waiter of waiters) if (waiter.plugin === plugin) waiter.wake();
    },

    /** Answers at once with events after `after`, or waits for the plugin's next one, up to `waitMs`. */
    async wait(plugin: string, after: number | undefined): Promise<HostEventBatch> {
      // Without `after`, or with one from before this companion started, watching starts now.
      if (after === undefined || after > seq) return { events: [], next: seq };
      if (pick(plugin, after).length === 0 && !closed) {
        await new Promise<void>((resolve) => {
          let timer: NodeJS.Timeout | undefined;
          const waiter: Waiter = {
            plugin,
            wake: () => {
              clearTimeout(timer);
              waiters.delete(waiter);
              resolve();
            },
          };
          timer = setTimeout(waiter.wake, waitMs);
          waiters.add(waiter);
        });
      }
      return { events: pick(plugin, after), next: seq };
    },

    /** Answers every waiting request, so the server can close. */
    close(): void {
      closed = true;
      for (const waiter of waiters) waiter.wake();
    },
  };
}

export type EventLog = ReturnType<typeof createEventLog>;
