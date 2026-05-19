import { execFile } from "node:child_process";
import { promisify } from "node:util";

import type { HostControlCommand } from "@vitadeck/sdk/host-control";

const execFileAsync = promisify(execFile);

export type HostActionProvider = {
  supportedCommands(): HostControlCommand[];
  run(command: HostControlCommand): Promise<void>;
};

const MACOS_COMMANDS = ["host.power.sleep_displays"] satisfies HostControlCommand[];

class MacOSActionProvider implements HostActionProvider {
  supportedCommands(): HostControlCommand[] {
    return [...MACOS_COMMANDS];
  }

  async run(command: HostControlCommand): Promise<void> {
    if (command !== "host.power.sleep_displays") {
      throw new Error(`Unsupported macOS command ${command}`);
    }
    await execFileAsync("pmset", ["displaysleepnow"]);
  }
}

class UnsupportedActionProvider implements HostActionProvider {
  supportedCommands(): HostControlCommand[] {
    return [];
  }

  async run(command: HostControlCommand): Promise<void> {
    throw new Error(`Host Control command ${command} is not supported on ${process.platform}`);
  }
}

export function createHostActionProvider(): HostActionProvider {
  return process.platform === "darwin" ? new MacOSActionProvider() : new UnsupportedActionProvider();
}
