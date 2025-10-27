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
        <vita-button x={20} y={10} width={200} height={40} label={"Hello"} onClick={showHello} />
        <vita-button x={240} y={10} width={200} height={40} label={"Counters"} onClick={showCounters} />
        <vita-button x={460} y={10} width={200} height={40} label={"Timers"} onClick={showTimers} />
        <vita-button x={680} y={10} width={200} height={40} label={"Minesweeper"} onClick={showGame} />
        <vita-button
          x={900}
          y={10}
          width={40}
          height={40}
          color={theme.buttonBackground}
          label={"T"}
          onClick={cycleTheme}
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
