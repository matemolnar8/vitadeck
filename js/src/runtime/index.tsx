import React, { type ReactNode } from "react";
import { useTheme, type Theme, type ThemeContextValue, type ThemeName } from "./theme";

type TextChild = string | number | boolean | null | undefined;

/** PSVita panel resolution; JS layout matches native `SCREEN_WIDTH` / `SCREEN_HEIGHT`. */
export const VITA_SCREEN = { width: 960, height: 544 } as const;

/** Outer padding for deck apps (full canvas; no shell top bar). */
export const VITA_INSET = 20;

export function insetContent(): { x: number; y: number; width: number; height: number } {
  return {
    x: VITA_INSET,
    y: VITA_INSET,
    width: VITA_SCREEN.width - VITA_INSET * 2,
    height: VITA_SCREEN.height - VITA_INSET * 2,
  };
}

export type ScreenProps = {
  children?: ReactNode;
  color?: Color;
};

export function Screen({ children, color }: ScreenProps) {
  const { theme } = useTheme();
  return (
    <vita-rect x={0} y={0} width={VITA_SCREEN.width} height={VITA_SCREEN.height} color={color ?? theme.background}>
      {children}
    </vita-rect>
  );
}

export type RectProps = {
  x: number;
  y: number;
  width: number;
  height: number;
  variant?: "fill" | "outline";
  color?: Color;
  borderColor?: Color;
  borderRadius?: number;
  children?: ReactNode;
};

export function Rect(props: RectProps) {
  return <vita-rect {...props} />;
}

export type TextProps = {
  children: TextChild | TextChild[];
  color?: Color;
  border?: boolean;
  fontSize?: number;
};

export function Text(props: TextProps) {
  return <vita-text {...props} />;
}

export type ButtonProps = {
  x: number;
  y: number;
  width: number;
  height: number;
  color?: Color;
  textColor?: Color;
  borderRadius?: number;
  label: string;
  onPress?: () => void;
  onPressStart?: () => void;
  onPressEnd?: () => void;
};

export function Button({ onPress, onPressStart, onPressEnd, ...props }: ButtonProps) {
  return <vita-button {...props} onPress={onPress} onPressStart={onPressStart} onPressEnd={onPressEnd} />;
}

const runtimeColors = globalThis.Colors;
export { runtimeColors as Colors, useTheme };
export type { Theme, ThemeContextValue, ThemeName };
