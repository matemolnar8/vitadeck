import { Button, Rect, Screen, Stack, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useRef, useState } from "react";

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
    void (async () => {
      setAsyncResult(await timeoutPromise);
    })();

    const timeoutId = setTimeout(() => {
      setAsyncResult("Hello from timeout");
    }, 1000);
    return () => {
      clearTimeout(timeoutId);
    };
  }, []);

  useEffect(() => {
    if (timeoutActive) {
      timeoutIdRef.current ??= setTimeout(() => {
        setTimeoutMsg("Timeout fired!");
        timeoutIdRef.current = undefined;
        setTimeoutActive(false);
      }, 2000);
    } else if (timeoutIdRef.current) {
      clearTimeout(timeoutIdRef.current);
      timeoutIdRef.current = undefined;
    }
  }, [timeoutActive]);

  useEffect(() => {
    if (intervalRunning) {
      intervalIdRef.current ??= setInterval(() => {
        setTicks((prev) => prev + 1);
      }, 1000);
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
      <Stack x={insetX} y={insetY} gap={stackGap}>
        <Rect width={cw} height={rowH} color={theme.surface} borderRadius={6}>
          <Text fontSize={24} color={theme.text}>
            setTimeout demo: {timeoutMsg}
          </Text>
          <Stack x={20} y={50} direction="row" gap={10}>
            <Button
              width={180}
              height={40}
              label="Start 2s"
              onPress={() => {
                setTimeoutMsg("Waiting 2s...");
                setTimeoutActive(true);
              }}
              color={theme.success}
              textColor={theme.navText}
              borderRadius={4}
            />
            <Button
              width={180}
              height={40}
              label="Cancel"
              onPress={() => {
                if (timeoutActive) setTimeoutMsg("Cancelled");
                setTimeoutActive(false);
              }}
              color={theme.danger}
              textColor={theme.navText}
              borderRadius={4}
            />
          </Stack>
        </Rect>

        <Rect width={cw} height={rowH} color={theme.surfaceAlt} borderRadius={6}>
          <Text fontSize={24} color={theme.text}>
            setInterval demo: ticks = {ticks}
          </Text>
          <Stack x={20} y={50} direction="row" gap={10}>
            <Button
              width={180}
              height={40}
              label="Start"
              onPress={() => {
                setIntervalRunning(true);
              }}
              color={theme.success}
              textColor={theme.navText}
              borderRadius={4}
            />
            <Button
              width={180}
              height={40}
              label="Stop"
              onPress={() => {
                setIntervalRunning(false);
              }}
              color={theme.danger}
              textColor={theme.navText}
              borderRadius={4}
            />
          </Stack>
        </Rect>

        <Rect width={cw} height={rowH} color={theme.surface} borderRadius={6}>
          <Text fontSize={24} color={theme.text}>
            Promise vs setTimeout: {asyncResult || "…"}
          </Text>
          <Text fontSize={18} color={theme.outline}>
            Timeout at 1s, promise at 2s — final text should be from the promise
          </Text>
        </Rect>
      </Stack>
    </Screen>
  );
}
