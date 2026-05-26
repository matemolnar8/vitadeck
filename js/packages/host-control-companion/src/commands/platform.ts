import type { CommandHandler } from "../gateway/types.js";
import { darwinCommandHandlers } from "./darwin.js";

export function createPlatformHandlers(): CommandHandler[] {
  if (process.platform === "darwin") return darwinCommandHandlers;
  return [];
}
