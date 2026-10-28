import path from "node:path";
import { pathToFileURL } from "node:url";
import { isRecord } from "@vitadeck/sdk/host-control";
import type {
  HostCommandMap,
  HostControlWireErrorCode,
  HostPlugin,
  PayloadOf,
  ResultOf,
} from "@vitadeck/sdk/host-control";

export type HostPluginImplementations<C extends HostCommandMap> = {
  readonly [K in keyof C]: (payload: PayloadOf<C[K]>) => Promise<ResultOf<C[K]>> | ResultOf<C[K]>;
};

type ImplementedCommand = {
  readonly isPayload: (value: unknown) => boolean;
  readonly run: (payload: unknown) => Promise<object> | object;
};

/** A Host Control Plugin with its implementation, ready for the Host Companion to serve. */
export type ImplementedHostPlugin = {
  readonly id: string;
  readonly commands: ReadonlyMap<string, ImplementedCommand>;
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

export function implementHostPlugin<const C extends HostCommandMap>(
  plugin: HostPlugin<C>,
  implementations: HostPluginImplementations<C>,
): ImplementedHostPlugin {
  // The server runs the guard first, so each implementation only sees its own payload type.
  const runs = implementations as unknown as Record<
    string,
    ((payload: unknown) => object | Promise<object>) | undefined
  >;
  const commands = new Map<string, ImplementedCommand>();
  for (const [name, command] of Object.entries(plugin.commands)) {
    const run = runs[name];
    if (typeof run !== "function") throw new Error(`Plugin ${plugin.id} has no implementation for ${name}.`);
    commands.set(name, { isPayload: (value) => command.isPayload(value), run });
  }
  return { id: plugin.id, commands };
}

function isImplementedHostPlugin(value: unknown): value is ImplementedHostPlugin {
  return isRecord(value) && typeof value.id === "string" && value.commands instanceof Map;
}

function isPathSpecifier(specifier: string): boolean {
  return specifier.startsWith(".") || path.isAbsolute(specifier);
}

/**
 * Loads a plugin implementation. A path is resolved against `baseDir` and imported as is; a bare package name
 * imports its `./companion` export, resolved from this package.
 */
export async function loadHostPlugin(specifier: string, baseDir: string): Promise<ImplementedHostPlugin> {
  const target = isPathSpecifier(specifier)
    ? pathToFileURL(path.resolve(baseDir, specifier)).href
    : `${specifier}/companion`;
  const module: unknown = await import(target);
  if (!isRecord(module) || !isImplementedHostPlugin(module.default)) {
    throw new Error(`${specifier} does not default-export implementHostPlugin(...).`);
  }
  return module.default;
}
