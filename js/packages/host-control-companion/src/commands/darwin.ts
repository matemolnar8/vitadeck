import { execFile } from "node:child_process";
import { promisify } from "node:util";

import type { CommandHandler } from "../gateway/types.js";

const execFileAsync = promisify(execFile);

export const darwinCommandHandlers: CommandHandler[] = [
  {
    command: "host.power.sleep_displays",
    handle: async () => {
      await execFileAsync("pmset", ["displaysleepnow"]);
      return {};
    },
  },
];
