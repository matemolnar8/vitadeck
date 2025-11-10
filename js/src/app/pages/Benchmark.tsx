import React, { useCallback, useEffect, useMemo, useState } from "react";
import { useTheme } from "../theme";

const GRID_LEFT = 20;
const GRID_TOP = 140;
const GRID_WIDTH = 920;
const GRID_HEIGHT = 360;
const CONTROL_HEIGHT = 100;
const CELL_GAP = 2;
const MIN_DIMENSION = 1;
const MAX_DIMENSION = 80;
const DEFAULT_ROWS = 20;
const DEFAULT_COLUMNS = 30;

function clamp(value: number, min: number, max: number) {
  return Math.max(min, Math.min(max, value));
}

function buildGrid(rows: number, columns: number, tick: number) {
  const rects: React.ReactNode[] = [];
  const innerWidth = GRID_WIDTH - CELL_GAP * (columns - 1);
  const innerHeight = GRID_HEIGHT - CELL_GAP * (rows - 1);
  const rawCellWidth = innerWidth / columns;
  const rawCellHeight = innerHeight / rows;

  for (let row = 0; row < rows; row++) {
    for (let column = 0; column < columns; column++) {
      const width = Math.max(1, Math.round(rawCellWidth));
      const height = Math.max(1, Math.round(rawCellHeight));
      const x = Math.round(GRID_LEFT + column * (rawCellWidth + CELL_GAP));
      const y = Math.round(GRID_TOP + row * (rawCellHeight + CELL_GAP));
      const colorToggle = (row + column + tick) % 2 === 0;
      rects.push(
        <vita-rect
          key={`${row}-${column}`}
          x={x}
          y={y}
          width={width}
          height={height}
          color={colorToggle ? Colors.GRAY : Colors.DARKGRAY}
          borderColor={Colors.BLACK}
        />,
      );
    }
  }

  return rects;
}

export const Benchmark = () => {
  const { theme } = useTheme();
  const [rows, setRows] = useState(DEFAULT_ROWS);
  const [columns, setColumns] = useState(DEFAULT_COLUMNS);
  const [isAnimating, setIsAnimating] = useState(false);
  const [tick, setTick] = useState(0);

  useEffect(() => {
    if (!isAnimating) return;
    const interval = setInterval(() => {
      setTick((prev) => prev + 1);
    }, 16);
    return () => {
      clearInterval(interval);
    };
  }, [isAnimating]);

  const totalRects = rows * columns;

  const gridRects = useMemo(() => buildGrid(rows, columns, isAnimating ? tick : 0), [rows, columns, tick, isAnimating]);

  const adjustRows = useCallback((delta: number) => {
    setRows((prev) => clamp(prev + delta, MIN_DIMENSION, MAX_DIMENSION));
  }, []);

  const adjustColumns = useCallback((delta: number) => {
    setColumns((prev) => clamp(prev + delta, MIN_DIMENSION, MAX_DIMENSION));
  }, []);

  const toggleAnimation = useCallback(() => {
    setIsAnimating((prev) => !prev);
  }, []);

  const resetGrid = useCallback(() => {
    setRows(DEFAULT_ROWS);
    setColumns(DEFAULT_COLUMNS);
    setTick(0);
    setIsAnimating(false);
  }, []);

  return (
    <>
      <vita-rect x={0} y={0} width={960} height={544} color={theme.background}>
        <vita-rect x={20} y={20} width={920} height={CONTROL_HEIGHT} color={theme.surface}>
          <vita-text fontSize={24} color={theme.text}>
            Benchmark Mode
          </vita-text>
          <vita-text fontSize={18} color={theme.text}>
            {`Rows: ${rows}  Columns: ${columns}  Total Rects: ${totalRects}`}
          </vita-text>
          <vita-text fontSize={16} color={theme.text}>
            {`Animation: ${isAnimating ? "On" : "Off"}  Tick: ${tick}`}
          </vita-text>
        </vita-rect>

        <vita-button
          x={40}
          y={80}
          width={120}
          height={40}
          color={Colors.DARKBLUE}
          label={"Rows +"}
          onClick={() => adjustRows(1)}
        />
        <vita-button
          x={180}
          y={80}
          width={120}
          height={40}
          color={Colors.DARKBLUE}
          label={"Rows -"}
          onClick={() => adjustRows(-1)}
        />
        <vita-button
          x={340}
          y={80}
          width={120}
          height={40}
          color={Colors.DARKBLUE}
          label={"Cols +"}
          onClick={() => adjustColumns(1)}
        />
        <vita-button
          x={480}
          y={80}
          width={120}
          height={40}
          color={Colors.DARKBLUE}
          label={"Cols -"}
          onClick={() => adjustColumns(-1)}
        />
        <vita-button
          x={640}
          y={80}
          width={120}
          height={40}
          color={isAnimating ? Colors.GREEN : Colors.DARKBLUE}
          label={isAnimating ? "Stop" : "Animate"}
          onClick={toggleAnimation}
        />
        <vita-button x={780} y={80} width={120} height={40} color={Colors.RED} label={"Reset"} onClick={resetGrid} />

        <vita-rect
          x={GRID_LEFT - CELL_GAP}
          y={GRID_TOP - CELL_GAP}
          width={GRID_WIDTH + CELL_GAP * 2}
          height={GRID_HEIGHT + CELL_GAP * 2}
          color={theme.surface}
          borderColor={theme.navBackground}
        />
      </vita-rect>

      {gridRects}
    </>
  );
};
