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
