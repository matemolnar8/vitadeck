import React, { useEffect, useRef, useState } from "react";
import { Button, Rect, Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";

const timeoutPromise = new Promise<string>((resolve) => {
  setTimeout(() => {
    resolve("Hello from promise");
  }, 2000);
});

export default function TimersDeckApp() {
  const { theme } = useTheme();
  const [timeoutMsg, setTimeoutMsg] = useState("Idle");
  const timeoutIdRef = useRef<number | undefined>();
  const [timeoutActive, setTimeoutActive] = useState(false);
  const [ticks, setTicks] = useState(0);
  const intervalIdRef = useRef<number | undefined>();
  const [intervalRunning, setIntervalRunning] = useState(false);
  const [asyncResult, setAsyncResult] = useState("");

  useEffect(() => {
    timeoutPromise.then((result) => {
      setAsyncResult(result);
    });

    setTimeout(() => {
      setAsyncResult("Hello from timeout");
    }, 1000);
  }, []);

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
    } else if (timeoutIdRef.current) {
      clearTimeout(timeoutIdRef.current);
      timeoutIdRef.current = undefined;
      setTimeoutMsg("Cancelled");
    }
  }, [timeoutActive]);

  useEffect(() => {
    if (intervalRunning) {
      if (!intervalIdRef.current) {
        intervalIdRef.current = setInterval(() => {
          setTicks((prev) => prev + 1);
        }, 1000);
      }
    } else if (intervalIdRef.current) {
      clearInterval(intervalIdRef.current);
      intervalIdRef.current = undefined;
    }
  }, [intervalRunning]);

  useEffect(() => {
    return () => {
      if (timeoutIdRef.current) clearTimeout(timeoutIdRef.current);
      if (intervalIdRef.current) clearInterval(intervalIdRef.current);
    };
  }, []);

  const { x: insetX, y: insetY, width: cw, height: ch } = insetContent();
  const stackGap = 16;
  const rowH = (ch - 2 * stackGap) / 3;

  return (
    <Screen>
      <Rect x={insetX} y={insetY} width={cw} height={rowH} color={theme.surface} borderRadius={6}>
        <Text fontSize={24} color={theme.text}>
          setTimeout demo: {timeoutMsg}
        </Text>
        <Button
          x={20}
          y={50}
          width={180}
          height={40}
          label="Start 2s"
          onPress={() => setTimeoutActive(true)}
          color={theme.success}
          textColor={theme.navText}
          borderRadius={4}
        />
        <Button
          x={210}
          y={50}
          width={180}
          height={40}
          label="Cancel"
          onPress={() => setTimeoutActive(false)}
          color={theme.danger}
          textColor={theme.navText}
          borderRadius={4}
        />
      </Rect>

      <Rect
        x={insetX}
        y={insetY + rowH + stackGap}
        width={cw}
        height={rowH}
        color={theme.surfaceAlt}
        borderRadius={6}
      >
        <Text fontSize={24} color={theme.text}>
          setInterval demo: ticks = {ticks}
        </Text>
        <Button
          x={20}
          y={50}
          width={180}
          height={40}
          label="Start"
          onPress={() => setIntervalRunning(true)}
          color={theme.success}
          textColor={theme.navText}
          borderRadius={4}
        />
        <Button
          x={210}
          y={50}
          width={180}
          height={40}
          label="Stop"
          onPress={() => setIntervalRunning(false)}
          color={theme.danger}
          textColor={theme.navText}
          borderRadius={4}
        />
      </Rect>

      <Rect
        x={insetX}
        y={insetY + 2 * (rowH + stackGap)}
        width={cw}
        height={rowH}
        color={theme.surface}
        borderRadius={6}
      >
        <Text fontSize={24} color={theme.text}>
          Promise vs setTimeout: {asyncResult || "…"}
        </Text>
        <Text fontSize={18} color={theme.outline}>
          Timeout at 1s, promise at 2s — final text should be from the promise
        </Text>
      </Rect>
    </Screen>
  );
}
