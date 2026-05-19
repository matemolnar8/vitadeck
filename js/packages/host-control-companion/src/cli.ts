#!/usr/bin/env node
import { createHostActionProvider } from "./actions.js";
import { startHostControlServer } from "./server.js";

const actions = createHostActionProvider();

try {
  const { url } = await startHostControlServer(actions);
  console.log("VitaDeck Host Control Companion");
  console.log(`Host Control Console Address: ${url}`);
  console.log("Endpoint: POST /v1/command");
  console.log(`Supported OS commands: ${actions.supportedCommands().join(", ") || "none"}`);
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exitCode = 1;
}
