import type { HostControlWireErrorCode } from "./protocol";

declare const resultType: unique symbol;

/**
 * One Host Control command: a guard the Host Companion runs on the payload, and a result type that exists only
 * for TypeScript. There is no schema library, since it would ship inside every Deck App bundle.
 */
export type HostCommand<P, R extends object> = {
  readonly isPayload: (value: unknown) => value is P;
  readonly [resultType]?: R;
};

export type HostCommandMap = Record<string, HostCommand<unknown, object>>;

/**
 * A **Host Control Plugin**: the commands a Deck App imports to call a computer, and the Host Companion loads
 * an implementation for. `id` namespaces its commands on the wire.
 */
export type HostPlugin<C extends HostCommandMap = HostCommandMap> = {
  readonly id: string;
  readonly commands: C;
};

export type PayloadOf<T> = T extends HostCommand<infer P, object> ? P : never;
export type ResultOf<T> = T extends HostCommand<unknown, infer R> ? R : never;

export function command<P, R extends object>(isPayload: (value: unknown) => value is P): HostCommand<P, R> {
  return { isPayload };
}

export function defineHostPlugin<const C extends HostCommandMap>(id: string, commands: C): HostPlugin<C> {
  return { id, commands };
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

/** Always loaded by every Host Companion. */
export const corePlugin = defineHostPlugin("core", {
  capabilities: command<undefined, HostCapabilities>(isAbsent),
  echo: command<{ text: string }, { text: string }>(
    (value): value is { text: string } => isRecord(value) && typeof value.text === "string",
  ),
});

export type HostPluginImplementations<C extends HostCommandMap> = {
  readonly [K in keyof C]: (payload: PayloadOf<C[K]>) => Promise<ResultOf<C[K]>> | ResultOf<C[K]>;
};

/**
 * A Host Control Plugin with its implementation, as a Host Plugin File default-exports it. The Host Companion
 * checks this shape rather than a class, since every Host Plugin File bundles its own copy of this module.
 */
export type ImplementedHostPlugin = {
  readonly id: string;
  readonly commands: ReadonlyMap<string, ImplementedHostCommand>;
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
export function implementHostPlugin<const C extends HostCommandMap>(
  plugin: HostPlugin<C>,
  implementations: HostPluginImplementations<C>,
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
  return { id: plugin.id, commands };
}
