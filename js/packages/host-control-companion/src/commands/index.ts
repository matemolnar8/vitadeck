import os from "node:os";

import { createHostControlGateway, hostControlCommands } from "@vitadeck/sdk/host-control";

import { hostControlImplementations } from "../implementations.js";

export function createDefaultGateway() {
  return createHostControlGateway(hostControlCommands, hostControlImplementations, {
    platform: process.platform,
    hostName: os.hostname(),
  });
}
