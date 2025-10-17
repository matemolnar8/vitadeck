import { useEffect, useRef, useState } from "react";

export type TickerState = {
  tick: number;
  elapsedMs: number;
  beatIndex: number; // cycles 0..7 roughly every 550ms
  cursorOn: boolean; // blinks ~420ms
};

export function useTicker(stepMs: number = 33): TickerState {
  const [tick, setTick] = useState(0);
  const startTimeRef = useRef<number>(Date.now());
  const intervalRef = useRef<number | null>(null);

  useEffect(() => {
    if (intervalRef.current !== null) return;
    intervalRef.current = setInterval(() => setTick((t) => t + 1), stepMs);
    return () => {
      if (intervalRef.current !== null) clearInterval(intervalRef.current);
      intervalRef.current = null;
    };
  }, [stepMs]);

  const elapsedMs = Date.now() - startTimeRef.current;
  const beatIndex = Math.floor(elapsedMs / 550) % 8;
  const cursorOn = Math.floor(elapsedMs / 420) % 2 === 0;

  return { tick, elapsedMs, beatIndex, cursorOn };
}


