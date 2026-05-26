import { execFile } from "node:child_process";
import { promisify } from "node:util";

const execFileAsync = promisify(execFile);

export type MediaKeyAction =
  | "play_pause"
  | "next"
  | "previous"
  | "volume_up"
  | "volume_down"
  | "mute";

/** PROTOTYPE — simulated media keys; replace with a stable OS integration before shipping. */
const LINUX_KEYS: Record<MediaKeyAction, string> = {
  play_pause: "XF86AudioPlay",
  next: "XF86AudioNext",
  previous: "XF86AudioPrev",
  volume_up: "XF86AudioRaiseVolume",
  volume_down: "XF86AudioLowerVolume",
  mute: "XF86AudioMute",
};

/** macOS System Events key codes (best-effort; varies by keyboard layout). */
const DARWIN_KEY_CODES: Record<MediaKeyAction, number> = {
  play_pause: 49,
  next: 17,
  previous: 18,
  volume_up: 48,
  volume_down: 47,
  mute: 72,
};

export async function pressMediaKey(platform: NodeJS.Platform, action: MediaKeyAction): Promise<void> {
  if (platform === "linux") {
    await execFileAsync("xdotool", ["key", LINUX_KEYS[action]]);
    return;
  }
  if (platform === "darwin") {
    const code = DARWIN_KEY_CODES[action];
    await execFileAsync("osascript", ["-e", `tell application "System Events" to key code ${code}`]);
    return;
  }
  throw new Error(`Media keys are not implemented on ${platform}.`);
}
