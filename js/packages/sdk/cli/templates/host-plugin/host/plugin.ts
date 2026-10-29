import { command, defineHostPlugin, isRecord } from "@vitadeck/sdk/host-control";

// The commands a Deck App can send to a computer. Deck Apps bundle this file, so it must not import Node.js
// modules; the implementation is in companion.ts.
export const hostPlugin = defineHostPlugin("vitadeck-scaffold-template", {
  greet: command<{ name: string }, { message: string }>(
    (value): value is { name: string } => isRecord(value) && typeof value.name === "string",
  ),
});
