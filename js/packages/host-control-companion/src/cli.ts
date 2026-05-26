#!/usr/bin/env node
import { createDefaultGateway, startHostControlServer } from "./server.js";

const gateway = createDefaultGateway();

try {
  const { url } = await startHostControlServer(gateway);
  console.log("VitaDeck Host Control Companion");
  console.log(`Host Control Console Address: ${url}`);
  console.log("Endpoint: POST /v1/command");
  const osCommands = gateway
    .availableCommands()
    .filter((command) => command !== "host.capabilities" && command !== "host.echo");
  console.log(`Supported OS commands: ${osCommands.join(", ") || "none"}`);
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exitCode = 1;
}
