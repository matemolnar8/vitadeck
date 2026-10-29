import { createHostClient } from "./client";
import { corePlugin } from "./plugin";

export { createHostClient, linkedHosts, watchHostEvents } from "./client";
export type { HostClient, HostClientOptions, HostTransport, LinkedHost, WatchHostEventsOptions } from "./client";
export {
  HOST_PLUGIN_SCHEMA_VERSION,
  HostCommandError,
  command,
  corePlugin,
  defineHostPlugin,
  event,
  implementHostPlugin,
  isAbsent,
  isRecord,
} from "./plugin";
export type {
  DataOf,
  HostCapabilities,
  HostCommand,
  HostCommandMap,
  HostEvent,
  HostEventBatch,
  HostEventMap,
  HostEventOf,
  HostPlugin,
  HostPluginEmit,
  HostPluginImplementations,
  HostPluginLifecycle,
  HostPluginManifest,
  ImplementedHostPlugin,
  PayloadOf,
  ResultOf,
} from "./plugin";
export { HOST_CONTROL_PROTOCOL_VERSION, HOST_CONTROL_WIRE_ERRORS, isWireErrorCode } from "./protocol";
export { systemStatsPlugin } from "./system-stats";
export type { SystemStats } from "./system-stats";
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
