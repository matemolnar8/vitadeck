import type { ReactNode } from "react";
import type { Color, FontName, ImageName } from "./types";

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

export type VitaScrollProps = WithKey<{
  x: number;
  y: number;
  width: number;
  height: number;
  /** Optional background fill. */
  color?: Color;
  /** Vertical gap in pixels between stacked children. */
  gap?: number;
  /** Inner padding in pixels around the stacked content. */
  padding?: number;
  children?: ReactNode | ReactNode[];
}>;

export type VitaImageProps = WithKey<{
  x: number;
  y: number;
  image: ImageName;
}> & (
  | { width: number; height: number }
  | { width: number; height?: never }
  | { height: number; width?: never }
);

export type VitaHostPropsByType = {
  "vita-text": VitaTextProps;
  "vita-rect": VitaRectProps;
  "vita-button": VitaButtonProps;
  "vita-scroll": VitaScrollProps;
  "vita-image": VitaImageProps;
};

export type VitaHostType = keyof VitaHostPropsByType;
