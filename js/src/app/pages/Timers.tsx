import React, { useEffect, useRef, useState } from "react";
import { useTheme } from "../theme";

export const Timers = () => {
  const { theme } = useTheme();
  // setTimeout demo
  const [timeoutMsg, setTimeoutMsg] = useState("Idle");
  const timeoutIdRef = useRef<number | undefined>();
  const [timeoutActive, setTimeoutActive] = useState(false);

  const startTimeout = () => {
    setTimeoutActive(true);
  };

  const cancelTimeout = () => {
    setTimeoutActive(false);
  };

  useEffect(() => {
    if (timeoutActive) {
      setTimeoutMsg("Waiting 2s...");
      if (!timeoutIdRef.current) {
        timeoutIdRef.current = setTimeout(() => {
          setTimeoutMsg("Timeout fired!");
          timeoutIdRef.current = undefined;
          setTimeoutActive(false);
        }, 2000);
      }
    } else {
      if (timeoutIdRef.current) {
        clearTimeout(timeoutIdRef.current);
        timeoutIdRef.current = undefined;
        setTimeoutMsg("Cancelled");
      }
    }
  }, [timeoutActive]);

  useEffect(() => {
    return () => {
      if (timeoutIdRef.current) clearTimeout(timeoutIdRef.current);
    };
  }, []);

  // setInterval demo
  const [ticks, setTicks] = useState(0);
  const intervalIdRef = useRef<number | undefined>();
  const [intervalRunning, setIntervalRunning] = useState(false);

  const startInterval = () => {
    setIntervalRunning(true);
  };

  const stopInterval = () => {
    setIntervalRunning(false);
  };

  useEffect(() => {
    if (intervalRunning) {
      if (!intervalIdRef.current) {
        intervalIdRef.current = setInterval(() => {
          setTicks((t) => t + 1);
        }, 1000);
      }
    } else {
      if (intervalIdRef.current) {
        clearInterval(intervalIdRef.current);
        intervalIdRef.current = undefined;
      }
    }
  }, [intervalRunning]);

  useEffect(() => {
    return () => {
      if (intervalIdRef.current) clearInterval(intervalIdRef.current);
    };
  }, []);

  return (
    <>
      <vita-rect x={20} y={20} width={920} height={200} color={theme.surface} borderRadius={0.15}>
        <vita-text fontSize={24} color={theme.text}>
          setTimeout demo: {timeoutMsg}
        </vita-text>
        <vita-button x={20} y={50} width={180} height={40} label={"Start 2s"} onClick={startTimeout} color={theme.success} textColor={theme.navText} borderRadius={0.2} />
        <vita-button x={210} y={50} width={180} height={40} label={"Cancel"} onClick={cancelTimeout} color={theme.danger} textColor={theme.navText} borderRadius={0.2} />
      </vita-rect>

      <vita-rect x={20} y={240} width={920} height={200} color={theme.surfaceAlt} borderRadius={0.15}>
        <vita-text fontSize={24} color={theme.text}>
          setInterval demo: ticks = {ticks}
        </vita-text>
        <vita-button x={20} y={50} width={180} height={40} label={"Start"} onClick={startInterval} color={theme.success} textColor={theme.navText} borderRadius={0.2} />
        <vita-button x={210} y={50} width={180} height={40} label={"Stop"} onClick={stopInterval} color={theme.danger} textColor={theme.navText} borderRadius={0.2} />
      </vita-rect>
    </>
  );
};
