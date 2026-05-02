import React from "react";
import { Screen, VITA_INSET, insetContent } from "@vitadeck/runtime";
import { Counter } from "../shared/Counter";

const COL_W = 180;
const COUNTER_H = 90;

export default function CountersDeckApp() {
  const { width: cw, height: ch } = insetContent();
  const colGap = (cw - COL_W * 3) / 2;
  const xs = [VITA_INSET, VITA_INSET + COL_W + colGap, VITA_INSET + (COL_W + colGap) * 2] as const;
  const rowGap = (ch - COUNTER_H * 2) / 3;
  const y0 = VITA_INSET + rowGap;
  const y1 = y0 + COUNTER_H + rowGap;

  return (
    <Screen>
      <Counter x={xs[0]} y={y0} />
      <Counter x={xs[1]} y={y0} />
      <Counter x={xs[2]} y={y0} />

      <Counter x={xs[0]} y={y1} />
      <Counter x={xs[1]} y={y1} />
      <Counter x={xs[2]} y={y1} />
    </Screen>
  );
}
