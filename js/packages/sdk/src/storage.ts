// Declared for the host in the runtime's runtime-native.d.ts; kept off the author-visible globals.
const host = globalThis as unknown as {
  nativeReadDeckAppStorage(): string;
  nativeWriteDeckAppStorage(json: string): void;
};

let items: Map<string, string> | null = null;

function load(): Map<string, string> {
  if (items) return items;
  const raw = host.nativeReadDeckAppStorage();
  const loaded = new Map<string, string>();
  try {
    const parsed: unknown = JSON.parse(raw);
    if (typeof parsed === "object" && parsed !== null) {
      for (const [key, value] of Object.entries(parsed)) {
        if (typeof value === "string") loaded.set(key, value);
      }
    }
  } catch (error) {
    console.warn(`Deck App Storage is unreadable and starts empty: ${String(error)}`);
  }
  items = loaded;
  return loaded;
}

function persist(next: Map<string, string>) {
  host.nativeWriteDeckAppStorage(JSON.stringify(Object.fromEntries(next)));
  items = next;
}

/**
 * **Deck App Storage**: string key-value pairs kept for the Active Deck App across restarts, up to
 * 1 MiB of JSON per Deck App. `setItem` and `removeItem` return once the change is on disk;
 * `setItem` throws a `RangeError` when the store would pass the limit and leaves it unchanged.
 */
export const storage = {
  getItem(key: string): string | null {
    return load().get(key) ?? null;
  },
  setItem(key: string, value: string): void {
    persist(new Map(load()).set(key, value));
  },
  removeItem(key: string): void {
    const current = load();
    if (!current.has(key)) return;
    const next = new Map(current);
    next.delete(key);
    persist(next);
  },
};
