import type { HostControlWireErrorCode } from "./protocol";

declare const resultType: unique symbol;
declare const dataType: unique symbol;

/**
 * One Host Control command: a guard the Host Companion runs on the payload, and a result type that exists only
 * for TypeScript. There is no schema library, since it would ship inside every Deck App bundle.
 */
export type HostCommand<P, R extends object> = {
  readonly isPayload: (value: unknown) => value is P;
  readonly [resultType]?: R;
};

export type HostCommandMap = Record<string, HostCommand<unknown, object>>;

/** One **Host Control Event** a plugin's implementation emits; its data type exists only for TypeScript. */
export type HostEvent<D> = { readonly [dataType]?: D };

export type HostEventMap = Record<string, HostEvent<unknown>>;

/**
 * A **Host Control Plugin**: the commands a Deck App imports to call a computer and the events it can watch,
 * which the Host Companion loads an implementation for. `id` namespaces both on the wire.
 */
export type HostPlugin<C extends HostCommandMap = HostCommandMap, E extends HostEventMap = HostEventMap> = {
  readonly id: string;
  readonly commands: C;
  readonly events: E;
};

export type PayloadOf<T> = T extends HostCommand<infer P, object> ? P : never;
export type ResultOf<T> = T extends HostCommand<unknown, infer R> ? R : never;
export type DataOf<T> = T extends HostEvent<infer D> ? D : never;

/** An event as `watchHostEvents` delivers it: `name` narrows `data`. */
export type HostEventOf<E extends HostEventMap> = {
  [K in keyof E & string]: { seq: number; name: K; data: DataOf<E[K]> };
}[keyof E & string];

export function command<P, R extends object>(isPayload: (value: unknown) => value is P): HostCommand<P, R> {
  return { isPayload };
}

export function event<D>(): HostEvent<D> {
  return {};
}

// A plugin without events has an empty event map, so `watchHostEvents` offers it no event names.
// oxlint-disable-next-line typescript/no-generated-empty-object-type
export function defineHostPlugin<const C extends HostCommandMap, const E extends HostEventMap = Record<never, never>>(
  id: string,
  commands: C,
  events?: E,
): HostPlugin<C, E> {
  return { id, commands, events: events ?? ({} as E) };
}

/** Guard for commands that take no payload. */
export function isAbsent(value: unknown): value is undefined {
  return value === undefined;
}

export function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

export type HostCapabilities = {
  protocolVersion: number;
  name: string;
  platform: string;
  /** Plugins the companion loaded. It may list plugins and commands this Deck App does not know. */
  plugins: { id: string; commands: string[] }[];
};

/** Events a plugin emitted after `after`, and the sequence number to pass as `after` next time. */
export type HostEventBatch = { events: { seq: number; name: string; data: unknown }[]; next: number };

const hasText = (value: unknown): value is { text: string } => isRecord(value) && typeof value.text === "string";

/**
 * Always loaded by every Host Companion. `events` waits up to 25 s for a plugin's events, which is how events
 * reach the Vita (see `watchHostEvents`); `notify` emits a `notice`, which `vitadeck-host-companion notify` sends.
 */
export const corePlugin = defineHostPlugin(
  "core",
  {
    capabilities: command<undefined, HostCapabilities>(isAbsent),
    echo: command<{ text: string }, { text: string }>(hasText),
    events: command<{ plugin: string; after?: number }, HostEventBatch>(
      (value): value is { plugin: string; after?: number } =>
        isRecord(value) &&
        typeof value.plugin === "string" &&
        (value.after === undefined || (Number.isSafeInteger(value.after) && Number(value.after) >= 0)),
    ),
    notify: command<{ text: string }, object>(hasText),
  },
  { notice: event<{ text: string }>() },
);

export const HOST_PLUGIN_SCHEMA_VERSION = 1;

/**
 * `manifest.json` of a Host Plugin Package, the `<slug>.vdplugin` folder `vitadeck build` writes. `entry` is the
 * bundled implementation; the package may hold other files next to it, which the code finds through
 * `import.meta.url`.
 */
export type HostPluginManifest = {
  schemaVersion: typeof HOST_PLUGIN_SCHEMA_VERSION;
  name: string;
  version: string;
  entry: string;
};

export type HostPluginImplementations<C extends HostCommandMap> = {
  readonly [K in keyof C]: (payload: PayloadOf<C[K]>) => Promise<ResultOf<C[K]>> | ResultOf<C[K]>;
};

/**
 * A Host Control Plugin with its implementation, as a Host Plugin Package's entry default-exports it. The Host
 * Companion checks this shape rather than a class, since every package bundles its own copy of this module.
 */
export type ImplementedHostPlugin = {
  readonly id: string;
  readonly commands: ReadonlyMap<string, ImplementedHostCommand>;
  readonly start?: (emit: (name: string, data: unknown) => void) => (() => void) | void;
};

export type HostPluginEmit<E extends HostEventMap> = <K extends keyof E & string>(name: K, data: DataOf<E[K]>) => void;

/** Started when the companion loads the plugin; the returned function runs when it unloads or reloads. */
export type HostPluginLifecycle<E extends HostEventMap> = {
  start(emit: HostPluginEmit<E>): (() => void) | void;
};

type ImplementedHostCommand = {
  readonly isPayload: (value: unknown) => boolean;
  readonly run: (payload: unknown) => Promise<object> | object;
};

/** Thrown by an implementation to fail with a specific wire code. Any other throw answers `command_failed`. */
export class HostCommandError extends Error {
  readonly code: HostControlWireErrorCode;

  constructor(code: HostControlWireErrorCode, message: string) {
    super(message);
    this.name = "HostCommandError";
    this.code = code;
  }
}

/** The computer side of a plugin: the default export of the file `hostPlugin` names in `vitadeck.config.json`. */
export function implementHostPlugin<const C extends HostCommandMap, const E extends HostEventMap>(
  plugin: HostPlugin<C, E>,
  implementations: HostPluginImplementations<C>,
  lifecycle?: HostPluginLifecycle<E>,
): ImplementedHostPlugin {
  // The companion runs the guard first, so each implementation only sees its own payload type.
  const runs = implementations as unknown as Record<
    string,
    ((payload: unknown) => object | Promise<object>) | undefined
  >;
  const commands = new Map<string, ImplementedHostCommand>();
  for (const [name, definition] of Object.entries(plugin.commands)) {
    const run = runs[name];
    if (typeof run !== "function") throw new Error(`Plugin ${plugin.id} has no implementation for ${name}.`);
    commands.set(name, { isPayload: (value) => definition.isPayload(value), run });
  }
  if (!lifecycle) return { id: plugin.id, commands };
  return { id: plugin.id, commands, start: (emit) => lifecycle.start(emit) };
}
