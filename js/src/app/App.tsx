import React, { useState, useEffect } from "react";
import { Counter } from "./components/Counter";

export const App = () => {
  // Cause lag on PSVita
  // const [_, setRender] = useState(0);
  
  // useEffect(() => {
  //   const interval = setInterval(() => {
  //     setRender((render) => {
  //       console.log(`render: ${render}`);
  //       return render + 1;
  //     });
  //   }, 1000);
  //   return () => clearInterval(interval);
  // }, []);

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={Colors.BLACK}>
      {/* First row of 3 counters */}
      <Counter x={30} y={30} />
      <Counter x={360} y={30} />
      <Counter x={690} y={30} />

      {/* Second row of 3 counters */}
      <Counter x={30} y={270} />
      <Counter x={360} y={270} />
      <Counter x={690} y={270} />
    </vita-rect>
  );
};
