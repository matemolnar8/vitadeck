import { useEffect } from "react";

/** Vita buttons a Deck App can listen to. The D-pad and Cross drive **Focus**; Start and Select are System Input. */
export type PadButton = "circle" | "triangle" | "square" | "l" | "r";

export type PadButtonPhase = "pressstart" | "pressend";

export type PadButtonHandlers = {
  onPressStart?: () => void;
  onPressEnd?: () => void;
};

const subscribers = new Set<{ button: PadButton; handlers: PadButtonHandlers }>();

// A handler's setState can re-run usePadButton's effect mid-dispatch, which subscribes again. Iterating a copy keeps
// that new subscriber for the next press; iterating the Set itself would visit it and press again, forever.
export function dispatchPadButton(button: PadButton, phase: PadButtonPhase): void {
  for (const subscriber of Array.from(subscribers)) {
    if (subscriber.button !== button) continue;
    if (phase === "pressstart") subscriber.handlers.onPressStart?.();
    else subscriber.handlers.onPressEnd?.();
  }
}

/**
 * Calls `onPressStart` when `button` goes down and `onPressEnd` when it comes back up, while the
 * calling component is mounted. Desktop host keys are listed under Host mappings in CONTEXT.md.
 */
export function usePadButton(button: PadButton, { onPressStart, onPressEnd }: PadButtonHandlers): void {
  useEffect(() => {
    const subscriber = { button, handlers: { onPressStart, onPressEnd } };
    subscribers.add(subscriber);
    return () => {
      subscribers.delete(subscriber);
    };
  }, [button, onPressStart, onPressEnd]);
}
