import React, { type ReactNode } from "react";
import { useTheme, type Theme, type ThemeContextValue, type ThemeName } from "./theme";

type TextChild = string | number | boolean | null | undefined;

export type ScreenProps = {
  children?: ReactNode;
  color?: Color;
};

export function Screen({ children, color }: ScreenProps) {
  const { theme } = useTheme();
  return (
    <vita-rect x={0} y={0} width={960} height={544} color={color ?? theme.background}>
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
