import type { ReactNode } from "react";
import type { Color, FontName } from "./types";

type TextChild = string | number | boolean | null | undefined;
type WithKey<T> = T & { key?: string };

export type TextAlign = "left" | "center" | "right";
export type TextWrap = "none" | "word";

export type VitaTextProps = WithKey<{
  children: TextChild | TextChild[];
  color?: Color;
  border?: boolean;
  font?: FontName;
  fontSize?: number;
  x?: number;
  y?: number;
  width?: number;
  align?: TextAlign;
  wrap?: TextWrap;
  lineHeight?: number;
}>;

export type VitaRectProps = WithKey<{
  x: number;
  y: number;
  width: number;
  height: number;
  variant?: "fill" | "outline";
  color?: Color;
  borderColor?: Color;
  /** Corner radius in pixels (CSS `border-radius` semantics). */
  borderRadius?: number;
  children?: ReactNode | ReactNode[];
}>;

export type VitaButtonProps = WithKey<{
  x: number;
  y: number;
  width: number;
  height: number;
  color?: Color;
  textColor?: Color;
  /** Corner radius in pixels (CSS `border-radius` semantics). */
  borderRadius?: number;
  label: string;
  onPress?: () => void;
  onPressStart?: () => void;
  onPressEnd?: () => void;
}>;

export type VitaHostPropsByType = {
  "vita-text": VitaTextProps;
  "vita-rect": VitaRectProps;
  "vita-button": VitaButtonProps;
};

export type VitaHostType = keyof VitaHostPropsByType;
