import React, { createContext, useContext, useState } from "react";
import type { Color } from "./types";

function hex(value: string): Color {
  const normalized = value.replace("#", "");
  return {
    r: parseInt(normalized.slice(0, 2), 16),
    g: parseInt(normalized.slice(2, 4), 16),
    b: parseInt(normalized.slice(4, 6), 16),
    a: 255,
  };
}

const palette = {
  brand10: hex("#f7fbfd"),
  brand20: hex("#ebf5f9"),
  brand30: hex("#d0e7f1"),
  brand40: hex("#a8d3e6"),
  brand50: hex("#75b9d7"),
  brand60: hex("#41a0c8"),
  brand70: hex("#2e7c9e"),
  brand80: hex("#20576f"),
  brand90: hex("#153847"),
  brand100: hex("#071318"),
  gray10: hex("#fafafa"),
  gray20: hex("#f2f2f3"),
  gray30: hex("#dfe1e2"),
  gray40: hex("#c4c7c9"),
  gray50: hex("#a2a7aa"),
  gray60: hex("#80868a"),
  gray70: hex("#62676a"),
  gray80: hex("#45484a"),
  gray90: hex("#2c2e2f"),
  gray100: hex("#0f0f10"),
  accent: hex("#f7733b"),
  success: hex("#2bca75"),
  warning: hex("#eeda2b"),
  danger: hex("#d96226"),
};

export type Theme = {
  name: ThemeName;
  background: Color;
  navBackground: Color;
  navText: Color;
  surface: Color;
  surfaceAlt: Color;
  primary: Color;
  text: Color;
  outline: Color;
  buttonBackground: Color;
  buttonText: Color;
  accent: Color;
  success: Color;
  warning: Color;
  danger: Color;
};

export type ThemeName = "dark" | "light";

const THEMES: Record<ThemeName, Theme> = {
  dark: {
    name: "dark",
    background: palette.gray100,
    navBackground: palette.gray90,
    navText: palette.gray10,
    surface: palette.gray90,
    surfaceAlt: palette.gray80,
    primary: palette.brand80,
    text: palette.gray10,
    outline: palette.gray70,
    buttonBackground: palette.brand80,
    buttonText: palette.gray10,
    accent: palette.accent,
    success: palette.success,
    warning: palette.warning,
    danger: palette.danger,
  },
  light: {
    name: "light",
    background: palette.gray30,
    navBackground: palette.brand90,
    navText: palette.gray10,
    surface: palette.gray10,
    surfaceAlt: palette.gray40,
    primary: palette.brand70,
    text: palette.gray100,
    outline: palette.gray50,
    buttonBackground: palette.brand70,
    buttonText: palette.gray10,
    accent: palette.accent,
    success: palette.success,
    warning: palette.warning,
    danger: palette.danger,
  },
};

export type ThemeContextValue = {
  theme: Theme;
  themeName: ThemeName;
};

const ThemeContext = createContext<ThemeContextValue | null>(null);

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [themeName] = useState<ThemeName>("dark");
  const theme = THEMES[themeName];

  return <ThemeContext.Provider value={{ theme, themeName }}>{children}</ThemeContext.Provider>;
};

export function useTheme(): ThemeContextValue {
  const ctx = useContext(ThemeContext);
  if (!ctx) throw new Error("useTheme must be used within VitaDeck runtime bootstrap");
  return ctx;
}
