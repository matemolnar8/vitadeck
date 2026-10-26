import { Button, Rect, Screen, Scroll, Stack, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useRef, useState } from "react";

const timeoutPromise = new Promise<string>((resolve) => {
  setTimeout(() => {
    resolve("Hello from promise");
  }, 2000);
});

const MAX_LOGGED_TICKS = 100;

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
      setTimeoutMsg("Waiting 2s...");
      timeoutIdRef.current ??= setTimeout(() => {
        setTimeoutMsg("Timeout fired!");
        timeoutIdRef.current = undefined;
        setTimeoutActive(false);
      }, 2000);
    } else if (timeoutIdRef.current) {
      clearTimeout(timeoutIdRef.current);
      timeoutIdRef.current = undefined;
      setTimeoutMsg("Cancelled");
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
  const rowH = 96;
  const logPadding = 8;
  const loggedTicks = Array.from({ length: Math.min(ticks, MAX_LOGGED_TICKS) }, (_, i) => ticks - i);

  return (
    <Screen>
      <Stack x={insetX} y={insetY} width={cw} height={ch} gap={12}>
        <Rect width={cw} height={rowH} color={theme.surface} borderRadius={6}>
          <Text x={8} y={8} fontSize={24} color={theme.text}>
            setTimeout demo: {timeoutMsg}
          </Text>
          <Stack x={20} y={50} direction="row" gap={10}>
            <Button
              width={180}
              height={40}
              label="Start 2s"
              onPress={() => {
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
                setTimeoutActive(false);
              }}
              color={theme.danger}
              textColor={theme.navText}
              borderRadius={4}
            />
          </Stack>
        </Rect>

        <Rect width={cw} height={rowH} color={theme.surfaceAlt} borderRadius={6}>
          <Text x={8} y={8} fontSize={24} color={theme.text}>
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

        <Rect width={cw} height={64} color={theme.surface} borderRadius={6}>
          <Text x={8} y={8} fontSize={24} color={theme.text}>
            Promise vs setTimeout: {asyncResult || "…"}
          </Text>
          <Text x={8} y={8} fontSize={18} color={theme.outline}>
            Timeout at 1s, promise at 2s — final text should be from the promise
          </Text>
        </Rect>

        <Scroll width={cw} color={theme.surfaceAlt} gap={4} padding={logPadding}>
          {loggedTicks.length === 0 ? (
            <Text x={8} y={8} fontSize={18} color={theme.outline}>
              Start the interval to log ticks here
            </Text>
          ) : (
            loggedTicks.map((tick) => (
              <Rect key={tick} width={cw - 2 * logPadding} height={28} color={theme.surface} borderRadius={4}>
                <Text x={8} y={4} fontSize={18} color={theme.text}>
                  Tick {tick}
                </Text>
              </Rect>
            ))
          )}
        </Scroll>
      </Stack>
    </Screen>
  );
}
