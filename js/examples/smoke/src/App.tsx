import React, { useEffect, useState } from "react";
import { Button, Image, Rect, Screen, Scroll, Text, insetContent, type Color } from "@vitadeck/sdk";

const BG_COLOR: Color = { r: 13, g: 27, b: 42, a: 255 };
const SURFACE: Color = { r: 27, g: 38, b: 59, a: 255 };
const SURFACE_ALT: Color = { r: 65, g: 90, b: 119, a: 255 };
const OUTLINE: Color = { r: 119, g: 141, b: 169, a: 255 };
const STATUS_COLOR: Color = { r: 224, g: 225, b: 221, a: 255 };
const ACCENT: Color = { r: 233, g: 196, b: 106, a: 255 };
const BUTTON_TEXT: Color = { r: 13, g: 27, b: 42, a: 255 };

const SCROLL_ROWS = [
  {
    title: "Fill rect + wrapped text",
    body: "Default font with word wrap exercises the SDK text layout path across multiple lines in a rounded fill rect.",
    variant: "fill" as const,
    font: undefined,
    align: "left" as const,
    border: false,
  },
  {
    title: "Outline rect + bordered text",
    body: "Center aligned copy inside an outline variant rect with bordered text styling.",
    variant: "outline" as const,
    font: undefined,
    align: "center" as const,
    border: true,
  },
  {
    title: "Custom font rendering",
    body: "SMOKE_MONO 0123456789 ABCD",
    variant: "fill" as const,
    font: "smokeMono" as const,
    align: "left" as const,
    border: false,
  },
];

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

  const inset = insetContent();
  const headerHeight = 56;
  const footerHeight = 32;
  const scrollY = inset.y + headerHeight + 6;
  const scrollHeight = inset.height - headerHeight - footerHeight - 12;
  const rowWidth = inset.width - 20;
  const rowHeight = 64;
  const imageRowHeight = 72;

  return (
    <Screen color={BG_COLOR}>
      <Rect x={inset.x} y={inset.y} width={inset.width} height={headerHeight} color={SURFACE} borderRadius={8}>
        <Image x={inset.width - 82} y={4} image="smokeLogo" height={48} />
        <Text x={14} y={10} fontSize={26} color={STATUS_COLOR}>
          {status}
        </Text>
        <Text x={14} y={38} fontSize={14} color={OUTLINE}>
          SDK / runtime / timers / render smoke
        </Text>
      </Rect>

      <Scroll
        x={inset.x}
        y={scrollY}
        width={inset.width}
        height={scrollHeight}
        color={SURFACE_ALT}
        gap={8}
        padding={10}
      >
        {SCROLL_ROWS.map((row, index) => (
          <Rect
            key={row.title}
            x={0}
            y={0}
            width={rowWidth}
            height={rowHeight}
            variant={row.variant}
            color={index % 2 === 0 ? SURFACE : SURFACE_ALT}
            borderColor={OUTLINE}
            borderRadius={6}
          >
            <Text x={10} y={6} fontSize={16} color={ACCENT} align="left">
              {row.title}
            </Text>
            <Text
              x={10}
              y={26}
              width={rowWidth - 20}
              fontSize={13}
              color={STATUS_COLOR}
              align={row.align}
              wrap="word"
              border={row.border}
              font={row.font}
              lineHeight={15}
            >
              {row.body}
            </Text>
          </Rect>
        ))}

        <Rect
          x={0}
          y={0}
          width={rowWidth}
          height={imageRowHeight}
          color={SURFACE_ALT}
          borderColor={OUTLINE}
          borderRadius={6}
        >
          <Text x={10} y={6} fontSize={16} color={ACCENT} align="left">
            Image sizing
          </Text>
          <Text x={10} y={26} fontSize={13} color={STATUS_COLOR}>
            width-only and stretch inside viewport
          </Text>
          <Image x={rowWidth - 168} y={8} image="smokeLogo" width={88} />
          <Image x={rowWidth - 72} y={14} image="smokeLogo" width={64} height={32} />
        </Rect>

        <Button
          x={0}
          y={0}
          width={220}
          height={36}
          label="Static button"
          color={ACCENT}
          textColor={BUTTON_TEXT}
          borderRadius={8}
        />
        <Button
          x={0}
          y={0}
          width={220}
          height={36}
          label="Rounded button"
          color={SURFACE}
          textColor={STATUS_COLOR}
          borderRadius={20}
        />
      </Scroll>

      <Rect
        x={inset.x}
        y={scrollY + scrollHeight + 6}
        width={inset.width}
        height={footerHeight}
        variant="outline"
        borderColor={OUTLINE}
        borderRadius={6}
      >
        <Text x={14} y={9} fontSize={14} color={OUTLINE} font="smokeMono">
          All smoke UI elements fit inside this screenshot viewport.
        </Text>
      </Rect>
    </Screen>
  );
}
