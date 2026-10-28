import { createHostClient } from "./client";
import { corePlugin } from "./plugin";

export { createHostClient, linkedHosts } from "./client";
export type { HostClient, HostClientOptions, HostTransport, LinkedHost } from "./client";
export { command, corePlugin, defineHostPlugin, isAbsent, isRecord } from "./plugin";
export type { HostCapabilities, HostCommand, HostCommandMap, HostPlugin, PayloadOf, ResultOf } from "./plugin";
export { HOST_CONTROL_PROTOCOL_VERSION, HOST_CONTROL_WIRE_ERRORS, isWireErrorCode } from "./protocol";
export type {
  HostCommandRequest,
  HostControlClientErrorCode,
  HostControlErrorCode,
  HostControlLinkRequest,
  HostControlWireErrorCode,
  HostFailure,
  HostResult,
} from "./protocol";

/** The core plugin on the most recently linked host: `capabilities` and `echo`. */
export const host = createHostClient(corePlugin);
