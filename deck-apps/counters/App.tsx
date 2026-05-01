import React from "react";
import { Screen } from "@vitadeck/runtime";
import { Counter } from "../shared/Counter";

export default function CountersDeckApp() {
  return (
    <Screen>
      <Counter x={30} y={30} />
      <Counter x={360} y={30} />
      <Counter x={690} y={30} />

      <Counter x={30} y={270} />
      <Counter x={360} y={270} />
      <Counter x={690} y={270} />
    </Screen>
  );
}
