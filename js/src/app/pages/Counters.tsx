import React from "react";
import { Counter } from "../components/Counter";

export const Counters = () => {
  return (
    <>
      {/* First row of 3 counters */}
      <Counter x={30} y={30} />
      <Counter x={360} y={30} />
      <Counter x={690} y={30} />

      {/* Second row of 3 counters */}
      <Counter x={30} y={270} />
      <Counter x={360} y={270} />
      <Counter x={690} y={270} />
    </>
  );
};


