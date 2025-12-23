import React, { useState } from "react";
import { Counters } from "./pages/Counters";
import { Hello } from "./pages/Hello";
import { GameMinesweeper } from "./pages/Minesweeper";
import { Timers } from "./pages/Timers";
import { useTheme } from "./theme";

type Page = "hello" | "counters" | "timers" | "game";

export const App = () => {
  const [page, setPage] = useState<Page>("hello");
  const { theme, cycleTheme } = useTheme();

  const showHello = () => setPage("hello");
  const showCounters = () => setPage("counters");
  const showTimers = () => setPage("timers");
  const showGame = () => setPage("game");

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={theme.background}>
      {/* Top navigation bar */}
      <vita-rect x={0} y={0} width={960} height={60} color={theme.navBackground}>
        <vita-button
          x={20}
          y={10}
          width={200}
          height={40}
          label={"Hello"}
          onClick={showHello}
          color={theme.primary}
          textColor={theme.navText}
          borderRadius={0.25}
        />
        <vita-button
          x={240}
          y={10}
          width={200}
          height={40}
          label={"Counters"}
          onClick={showCounters}
          color={theme.primary}
          textColor={theme.navText}
          borderRadius={0.25}
        />
        <vita-button
          x={460}
          y={10}
          width={200}
          height={40}
          label={"Timers"}
          onClick={showTimers}
          color={theme.primary}
          textColor={theme.navText}
          borderRadius={0.25}
        />
        <vita-button
          x={680}
          y={10}
          width={200}
          height={40}
          label={"Minesweeper"}
          onClick={showGame}
          color={theme.primary}
          textColor={theme.navText}
          borderRadius={0.25}
        />
        <vita-button
          x={900}
          y={10}
          width={40}
          height={40}
          color={theme.accent}
          textColor={theme.navText}
          label={"T"}
          onClick={cycleTheme}
          borderRadius={0.4}
        />
      </vita-rect>

      {/* Content area */}
      <vita-rect x={0} y={60} width={960} height={484} color={theme.background}>
        {page === "hello" && <Hello />}
        {page === "counters" && <Counters />}
        {page === "timers" && <Timers />}
        {page === "game" && <GameMinesweeper />}
      </vita-rect>
    </vita-rect>
  );
};
