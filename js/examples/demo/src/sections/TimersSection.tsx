import { Button, Rect, Stack, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useState } from "react";
import { Label, Section } from "../ui";

const pad = (value: number) => String(value).padStart(2, "0");

function formatClock(date: Date) {
  return `${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
}

function sample() {
  return { clock: formatClock(new Date()), uptime: getTime() };
}

/** Wall-clock time and getTime() (seconds since VitaDeck opened its window), refreshed every second. */
function useNow() {
  const [now, setNow] = useState(sample);
  useEffect(() => {
    const id = setInterval(() => {
      setNow(sample());
    }, 1000);
    return () => {
      clearInterval(id);
    };
  }, []);
  return now;
}

export function TimersSection() {
  const { theme } = useTheme();
  const now = useNow();
  const [toastsShown, setToastsShown] = useState(0);
  const [toast, setToast] = useState<number | null>(null);

  useEffect(() => {
    const id = setTimeout(() => {
      setToast(null);
    }, 2000);
    return () => {
      clearTimeout(id);
    };
  }, [toast]);

  const showToast = () => {
    const next = toastsShown + 1;
    setToastsShown(next);
    setToast(next);
  };

  return (
    <Section
      title="Timers"
      blurb="setTimeout, setInterval and their clear functions work as on the web. Clear them in the effect cleanup so they stop when the component unmounts."
    >
      <Stack direction="row" gap={24} align="center">
        <Label width={200} fontSize={44} font="mono" color={theme.success}>
          {now.clock}
        </Label>
        <Label width={420} fontSize={14} wrap color={theme.outline}>
          {`setInterval ticks every second. getTime() says VitaDeck opened its window ${now.uptime.toFixed(0)} s ago.`}
        </Label>
      </Stack>
      <Button
        width={240}
        height={48}
        label="Show a toast for 2 s"
        color={theme.buttonBackground}
        textColor={theme.buttonText}
        borderRadius={8}
        autoFocus
        onPress={showToast}
      />
      {toast === null ? (
        <Rect width={420} height={44} variant="outline" color={theme.outline} borderRadius={22}>
          <Label y={10} width={420} align="center" color={theme.outline}>
            No toast right now
          </Label>
        </Rect>
      ) : (
        <Rect width={420} height={44} color={theme.accent} borderRadius={22}>
          <Label
            y={10}
            width={420}
            align="center"
            color={theme.background}
          >{`Toast #${toast}: setTimeout hides me`}</Label>
        </Rect>
      )}
    </Section>
  );
}
