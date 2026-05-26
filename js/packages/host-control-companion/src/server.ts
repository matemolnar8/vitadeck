export { createDefaultGateway } from "./commands/index.js";
export { createHostControlGateway } from "./gateway/registry.js";
export type { CommandHandler, HostControlGateway } from "./gateway/types.js";
export { startHostControlServer, type HostControlServer } from "./gateway/http-server.js";
