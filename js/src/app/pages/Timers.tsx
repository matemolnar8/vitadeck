import React, { useEffect, useRef, useState } from "react";

export const Timers = () => {
  // setTimeout demo
  const [timeoutMsg, setTimeoutMsg] = useState("Idle");
  const timeoutIdRef = useRef<number | undefined>();

  const startTimeout = () => {
    if (timeoutIdRef.current) return;
    setTimeoutMsg("Waiting 2s...");
    timeoutIdRef.current = setTimeout(() => {
      setTimeoutMsg("Timeout fired!");
      timeoutIdRef.current = undefined;
    }, 2000);
  };

  const cancelTimeout = () => {
    if (timeoutIdRef.current) {
      clearTimeout(timeoutIdRef.current);
      timeoutIdRef.current = undefined;
      setTimeoutMsg("Cancelled");
    }
  };

  useEffect(() => {
    return () => {
      if (timeoutIdRef.current) clearTimeout(timeoutIdRef.current);
    };
  }, []);

  // setInterval demo
  const [ticks, setTicks] = useState(0);
  const intervalIdRef = useRef<number | undefined>();

  const startInterval = () => {
    if (intervalIdRef.current) return;
    intervalIdRef.current = setInterval(() => {
      setTicks((t) => t + 1);
    }, 1000);
  };

  const stopInterval = () => {
    if (!intervalIdRef.current) return;
    clearInterval(intervalIdRef.current);
    intervalIdRef.current = undefined;
  };

  useEffect(() => {
    return () => {
      if (intervalIdRef.current) clearInterval(intervalIdRef.current);
    };
  }, []);

  return (
    <>
      <vita-rect x={20} y={20} width={920} height={200} color={Colors.DARKGREEN}>
        <vita-text fontSize={24} color={Colors.RAYWHITE}>
          setTimeout demo: {timeoutMsg}
        </vita-text>
        <vita-button x={20} y={50} width={180} height={40} label={"Start 2s"} onClick={startTimeout} />
        <vita-button x={210} y={50} width={180} height={40} label={"Cancel"} onClick={cancelTimeout} />
      </vita-rect>

      <vita-rect x={20} y={240} width={920} height={200} color={Colors.DARKBLUE}>
        <vita-text fontSize={24} color={Colors.RAYWHITE}>
          setInterval demo: ticks = {ticks}
        </vita-text>
        <vita-button x={20} y={50} width={180} height={40} label={"Start"} onClick={startInterval} />
        <vita-button x={210} y={50} width={180} height={40} label={"Stop"} onClick={stopInterval} />
      </vita-rect>
    </>
  );
};
