import React, { useEffect, useRef, useState } from "react";
import { Button, Rect, Screen, Text, useTheme } from "@vitadeck/runtime";

export default function TimersDeckApp() {
  const { theme } = useTheme();
  const [timeoutMsg, setTimeoutMsg] = useState("Idle");
  const timeoutIdRef = useRef<number | undefined>();
  const [timeoutActive, setTimeoutActive] = useState(false);
  const [ticks, setTicks] = useState(0);
  const intervalIdRef = useRef<number | undefined>();
  const [intervalRunning, setIntervalRunning] = useState(false);

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
          setTicks((ticks) => ticks + 1);
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

  return (
    <Screen>
      <Rect x={20} y={20} width={920} height={200} color={theme.surface} borderRadius={0.15}>
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
          borderRadius={0.2}
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
          borderRadius={0.2}
        />
      </Rect>

      <Rect x={20} y={240} width={920} height={200} color={theme.surfaceAlt} borderRadius={0.15}>
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
          borderRadius={0.2}
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
          borderRadius={0.2}
        />
      </Rect>
    </Screen>
  );
}
