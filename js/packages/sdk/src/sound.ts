import type { SoundName } from "./types";

// Declared for the host in the runtime's runtime-native.d.ts; kept off the author-visible globals.
const host = globalThis as unknown as {
  nativePlaySound(name: string, loop: boolean): void;
  nativeStopSound(name: string): void;
};

export type PlaySoundOptions = {
  /** Repeat until `stopSound`. */
  loop?: boolean;
};

/**
 * Plays a declared **Deck App Sound** from its start, restarting it if it is already playing. Different sounds play
 * at the same time. Throws a `RangeError` for a name the package does not declare.
 */
export function playSound(name: SoundName, options: PlaySoundOptions = {}): void {
  host.nativePlaySound(name, options.loop ?? false);
}

/** Stops a declared **Deck App Sound**; stopping one that is not playing does nothing. */
export function stopSound(name: SoundName): void {
  host.nativeStopSound(name);
}
