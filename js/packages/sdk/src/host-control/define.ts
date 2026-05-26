import type { HostControlFailureCode, HostControlRequest, VitaDeckLanJsonResult } from "./lan-json.js";

/** Host platforms that may provide a command implementation. `default` runs when no platform-specific impl exists. */
export type HostControlPlatform = NodeJS.Platform | "default";

export type HostControlCommandContext = {
  platform: NodeJS.Platform;
  hostName: string;
  availableCommands: () => string[];
};

export type CommandContract<Payload, Result extends object> = {
  readonly $payload: Payload;
  readonly $result: Result;
  validatePayload(payload: unknown): payload is Payload;
  invalidPayloadMessage?: string;
};

export function cmd<Payload, Result extends object>(
  contract: Omit<CommandContract<Payload, Result>, "$payload" | "$result">,
): CommandContract<Payload, Result> {
  return {
    ...contract,
    $payload: undefined as unknown as Payload,
    $result: undefined as unknown as Result,
  };
}

export type CommandDefinitions = Record<string, CommandContract<unknown, object>>;

export type HostControlCommandRegistry<T extends CommandDefinitions> = {
  readonly definitions: T;
  readonly commandNames: readonly (keyof T & string)[];
  isKnownCommand(command: string): command is keyof T & string;
  validatePayload<C extends keyof T & string>(command: C, payload: unknown): string | null;
};

export function defineHostControlCommands<const T extends CommandDefinitions>(
  definitions: T,
): HostControlCommandRegistry<T> {
  const commandNames = Object.keys(definitions) as (keyof T & string)[];

  return {
    definitions,
    commandNames,
    isKnownCommand(command: string): command is keyof T & string {
      return Object.prototype.hasOwnProperty.call(definitions, command);
    },
    validatePayload<C extends keyof T & string>(command: C, payload: unknown): string | null {
      const contract = definitions[command] as CommandContract<unknown, object>;
      if (contract.validatePayload(payload)) return null;
      return contract.invalidPayloadMessage ?? `Invalid payload for ${command}.`;
    },
  };
}

export type PayloadFor<C extends CommandContract<unknown, object>> = C["$payload"];
export type ResultFor<C extends CommandContract<unknown, object>> = C["$result"];

export type HostControlPayloadsFor<T extends CommandDefinitions> = {
  [K in keyof T & string]: PayloadFor<T[K]>;
};

export type HostControlResultsFor<T extends CommandDefinitions> = {
  [K in keyof T & string]: ResultFor<T[K]>;
};

export type CommandImplementationFn<Payload, Result extends object> = (
  payload: Payload,
  context: HostControlCommandContext,
) => Promise<Result>;

export type CommandPlatformImplementations<Payload, Result extends object> = Partial<
  Record<HostControlPlatform, CommandImplementationFn<Payload, Result>>
>;

export type HostControlImplementationsFor<T extends CommandDefinitions> = {
  [K in keyof T & string]: CommandPlatformImplementations<PayloadFor<T[K]>, ResultFor<T[K]>>;
};

export function defineHostControlImplementations<
  const T extends CommandDefinitions,
  const I extends HostControlImplementationsFor<T>,
>(_registry: HostControlCommandRegistry<T>, implementations: I): I {
  return implementations;
}

export function resolvePlatformImplementation<Payload, Result extends object>(
  implementations: CommandPlatformImplementations<Payload, Result> | undefined,
  platform: NodeJS.Platform,
): CommandImplementationFn<Payload, Result> | null {
  if (!implementations) return null;
  const platformImpl = implementations[platform];
  if (platformImpl) return platformImpl;
  const defaultImpl = implementations.default;
  if (defaultImpl) return defaultImpl;
  return null;
}

export type HostControlJsonResponse = {
  status: number;
  body: VitaDeckLanJsonResult<object>;
};

export function hostControlFailure(
  status: number,
  code: HostControlFailureCode,
  message: string,
): HostControlJsonResponse {
  return { status, body: { ok: false, code, message } };
}

export function hostControlSuccess<T extends object>(body: T): HostControlJsonResponse {
  return { status: 200, body: { ok: true, ...body } };
}

export function createHostControlGateway<const T extends CommandDefinitions>(
  registry: HostControlCommandRegistry<T>,
  implementations: HostControlImplementationsFor<T>,
  context: Omit<HostControlCommandContext, "availableCommands">,
) {
  let gatewayRef: {
    handleCommand(request: HostControlRequest): Promise<HostControlJsonResponse>;
    availableCommands(): (keyof T & string)[];
  };

  const commandContext: HostControlCommandContext = {
    ...context,
    availableCommands: () => gatewayRef.availableCommands(),
  };

  function availableCommands(): (keyof T & string)[] {
    return registry.commandNames.filter((command) => {
      return resolvePlatformImplementation(implementations[command], context.platform) !== null;
    });
  }

  async function handleCommand(request: HostControlRequest): Promise<HostControlJsonResponse> {
    if (!registry.isKnownCommand(request.command)) {
      return hostControlFailure(400, "unknown_command", `Unknown Host Control command: ${request.command}`);
    }

    const command = request.command;
    const impl = resolvePlatformImplementation(implementations[command], context.platform);
    if (!impl) {
      return hostControlFailure(
        501,
        "unsupported_platform",
        `${command} is not supported on ${context.platform}.`,
      );
    }

    const payloadError = registry.validatePayload(command, request.payload);
    if (payloadError) {
      return hostControlFailure(422, "invalid_payload", payloadError);
    }

    try {
      const result = await impl(request.payload as PayloadFor<T[typeof command]>, commandContext);
      return hostControlSuccess(result);
    } catch (error) {
      return hostControlFailure(
        501,
        "command_failed",
        error instanceof Error ? error.message : "Host action failed.",
      );
    }
  }

  gatewayRef = { handleCommand, availableCommands };
  return gatewayRef;
}
