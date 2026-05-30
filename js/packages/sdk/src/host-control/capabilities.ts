import type { HostCapabilitiesResult, HostControlCommand } from "./registry.js";
import type { VitaDeckLanJsonResult } from "./lan-json.js";

/** Whether a successful capabilities response includes `command`. */
export function isHostCommandAvailable(
  capabilities: VitaDeckLanJsonResult<HostCapabilitiesResult>,
  command: HostControlCommand,
): boolean {
  if (!capabilities.ok) return false;
  return capabilities.commands.includes(command);
}

/** Commands the companion reports for this host (empty if capabilities failed). */
export function availableHostCommands(
  capabilities: VitaDeckLanJsonResult<HostCapabilitiesResult>,
): readonly string[] {
  if (!capabilities.ok) return [];
  return capabilities.commands;
}
