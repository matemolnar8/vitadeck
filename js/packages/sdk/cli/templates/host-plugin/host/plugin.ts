import { command, defineHostPlugin, isRecord } from "@vitadeck/sdk/host-control";

// The commands your Deck App can send to a computer. This file is bundled into the Deck App, so it must not
// import Node.js modules; the implementation is in companion.ts.
export const hostPlugin = defineHostPlugin("vitadeck-scaffold-template", {
  greet: command<{ name: string }, { message: string }>(
    (value): value is { name: string } => isRecord(value) && typeof value.name === "string",
  ),
});
