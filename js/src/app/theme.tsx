import React, { createContext, useContext, useState } from "react";

export type Theme = {
  name: ThemeName;
  background: Color;
  navBackground: Color;
  surface: Color;
  surfaceAlt: Color;
  primary: Color;
  text: Color;
  outline: Color;
  buttonBackground: Color;
  buttonText: Color;
};

export type ThemeName = "dark" | "light" | "purple";

const THEMES: Record<ThemeName, Theme> = {
  dark: {
    name: "dark",
    background: Colors.BLACK,
    navBackground: Colors.DARKBLUE,
    surface: Colors.DARKGRAY,
    surfaceAlt: Colors.GRAY,
    primary: Colors.SKYBLUE,
    text: Colors.RAYWHITE,
    outline: Colors.BLACK,
    buttonBackground: Colors.DARKBLUE,
    buttonText: Colors.RAYWHITE,
  },
  light: {
    name: "light",
    background: Colors.RAYWHITE,
    navBackground: Colors.SKYBLUE,
    surface: Colors.LIGHTGRAY,
    surfaceAlt: Colors.GRAY,
    primary: Colors.BLUE,
    text: Colors.BLACK,
    outline: Colors.DARKGRAY,
    buttonBackground: Colors.BLUE,
    buttonText: Colors.RAYWHITE,
  },
  purple: {
    name: "purple",
    background: Colors.DARKPURPLE,
    navBackground: Colors.PURPLE,
    surface: Colors.DARKBLUE,
    surfaceAlt: Colors.VIOLET,
    primary: Colors.MAGENTA,
    text: Colors.RAYWHITE,
    outline: Colors.BLACK,
    buttonBackground: Colors.PURPLE,
    buttonText: Colors.RAYWHITE,
  },
};

export type ThemeContextValue = {
  theme: Theme;
  setTheme: (name: ThemeName) => void;
  themeName: ThemeName;
  cycleTheme: () => void;
};

const ThemeContext = createContext<ThemeContextValue | null>(null);

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [themeName, setThemeName] = useState<ThemeName>("dark");

  const theme = THEMES[themeName];

  const setTheme = (name: ThemeName) => {
    setThemeName(name);
  };

  const cycleTheme = () => {
    const nextMap: Record<ThemeName, ThemeName> = {
      dark: "light",
      light: "purple",
      purple: "dark",
    };
    setThemeName(nextMap[themeName]);
  };

  const value = { theme, setTheme, themeName, cycleTheme };

  return <ThemeContext.Provider value={value}>{children}</ThemeContext.Provider>;
};

export function useTheme(): ThemeContextValue {
  const ctx = useContext(ThemeContext);
  if (!ctx) throw new Error("useTheme must be used within ThemeProvider");
  return ctx;
}
