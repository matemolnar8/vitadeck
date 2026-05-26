import os from "node:os";

import { createHostControlGateway } from "../gateway/registry.js";
import type { HostControlGateway } from "../gateway/types.js";
import { createBuiltinHandlers, withHostIdentity } from "./builtin.js";
import { createPlatformHandlers } from "./platform.js";

export function createDefaultGateway(): HostControlGateway {
  const context = { platform: process.platform, hostName: os.hostname() };
  const platformHandlers = createPlatformHandlers();
  let gateway: HostControlGateway;

  const builtin = withHostIdentity(
    createBuiltinHandlers(() => gateway.availableCommands()),
    context,
  );

  gateway = createHostControlGateway([...builtin, ...platformHandlers], context);
  return gateway;
}
