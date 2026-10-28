import { command, defineHostPlugin, isAbsent, isRecord } from "@vitadeck/sdk/host-control";
import { HostCommandError, implementHostPlugin } from "../../src/index";

export const greeterPlugin = defineHostPlugin("greeter", {
  greet: command<{ name: string }, { greeting: string }>(
    (value): value is { name: string } => isRecord(value) && typeof value.name === "string",
  ),
  crash: command<undefined, object>(isAbsent),
  launch: command<undefined, object>(isAbsent),
});

export default implementHostPlugin(greeterPlugin, {
  greet: ({ name }) => ({ greeting: `Hello, ${name}!` }),
  crash: () => {
    throw new Error("The greeter fell over.");
  },
  launch: () => {
    throw new HostCommandError("unsupported_platform", "Launching is not supported on this computer.");
  },
});
