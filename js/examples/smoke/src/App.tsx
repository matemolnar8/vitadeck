import React, { useEffect, useState } from "react";
import { Screen, Text, type Color } from "@vitadeck/sdk";

const BG_COLOR: Color = { r: 13, g: 27, b: 42, a: 255 };
const LABEL_COLOR: Color = { r: 119, g: 141, b: 169, a: 255 };
const STATUS_COLOR: Color = { r: 224, g: 225, b: 221, a: 255 };

export default function SmokeDeckApp() {
  const [status, setStatus] = useState("SMOKE_READY");

  useEffect(() => {
    let intervalTicks = 0;
    const intervalId = setInterval(() => {
      intervalTicks += 1;
    }, 16);

    const timeoutId = setTimeout(() => {
      clearInterval(intervalId);
      if (intervalTicks < 1) {
        setStatus("SMOKE_INTERVAL_FAIL");
        return;
      }
      setStatus("SMOKE_OK");
    }, 50);

    return () => {
      clearTimeout(timeoutId);
      clearInterval(intervalId);
    };
  }, []);

  return (
    <Screen color={BG_COLOR}>
      <Text fontSize={48} color={STATUS_COLOR} x={100} y={200}>
        {status}
      </Text>
      <Text fontSize={24} color={LABEL_COLOR} x={100} y={280}>
        VitaDeck CI Smoke
      </Text>
    </Screen>
  );
}
