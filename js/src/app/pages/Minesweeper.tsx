import React, { useCallback, useMemo, useRef, useState } from "react";
import { useTheme } from "../theme";

type Cell = {
  id: string;
  mine: boolean;
  revealed: boolean;
  flagged: boolean;
  adjacent: number;
};

type Grid = Cell[][];

const playfield = { x: 20, y: 20, w: 920, h: 444 };
const gridSize = { rows: 9, cols: 16 }; // 16x9 fits nicely on 920x444
const headerHeight = 40;
const boardPadding = 10;
const resetButton = { width: 120, height: 30 };
const cellSize = {
  width: Math.floor((playfield.w - boardPadding * 2) / gridSize.cols),
  height: Math.floor((playfield.h - headerHeight - boardPadding * 2) / gridSize.rows),
};
const numMines = 22;
const longPressMs = 400;

function makeEmptyGrid(rows: number, cols: number): Grid {
  const grid: Grid = [];
  for (let r = 0; r < rows; r++) {
    const row: Cell[] = [];
    for (let c = 0; c < cols; c++) {
      row.push({ id: `${r}:${c}`, mine: false, revealed: false, flagged: false, adjacent: 0 });
    }
    grid.push(row);
  }
  return grid;
}

function inBounds(r: number, c: number, rows: number, cols: number): boolean {
  return r >= 0 && r < rows && c >= 0 && c < cols;
}

function neighbors(r: number, c: number, rows: number, cols: number): [number, number][] {
  const deltas = [-1, 0, 1];
  const res: [number, number][] = [];
  for (const dr of deltas) {
    for (const dc of deltas) {
      if (dr === 0 && dc === 0) continue;
      const nr = r + dr;
      const nc = c + dc;
      if (inBounds(nr, nc, rows, cols)) res.push([nr, nc]);
    }
  }
  return res;
}

function computeAdjacents(grid: Grid) {
  const rows = grid.length;
  const cols = grid[0] ? grid[0].length : 0;
  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      const row = grid[r];
      if (!row) continue;
      const cell = row[c];
      if (!cell) continue;
      if (cell.mine) {
        cell.adjacent = 0;
        continue;
      }
      let count = 0;
      for (const [nr, nc] of neighbors(r, c, rows, cols)) {
        const nrow = grid[nr];
        if (!nrow) continue;
        const n = nrow[nc];
        if (n?.mine) count++;
      }
      cell.adjacent = count;
    }
  }
}

function placeMines(grid: Grid, mines: number, safeR: number, safeC: number) {
  const rows = grid.length;
  const cols = grid[0] ? grid[0].length : 0;
  const safeSet = new Set<string>();
  // Make first click safe including its neighbors
  safeSet.add(`${safeR}:${safeC}`);
  for (const [nr, nc] of neighbors(safeR, safeC, rows, cols)) safeSet.add(`${nr}:${nc}`);

  let placed = 0;
  while (placed < mines) {
    const r = Math.floor(Math.random() * rows);
    const c = Math.floor(Math.random() * cols);
    const key = `${r}:${c}`;
    if (safeSet.has(key)) continue;
    const row = grid[r];
    if (!row) continue;
    const cell = row[c];
    if (!cell) continue;
    if (!cell.mine) {
      cell.mine = true;
      placed++;
    }
  }
  computeAdjacents(grid);
}

function cloneGrid(src: Grid): Grid {
  return src.map((row) => row.map((c) => ({ ...c })));
}

export const GameMinesweeper = () => {
  const { theme } = useTheme();
  const [grid, setGrid] = useState<Grid>(() => makeEmptyGrid(gridSize.rows, gridSize.cols));
  const [minesPlaced, setMinesPlaced] = useState(false);
  const [gameOver, setGameOver] = useState<null | "lost" | "won">(null);

  // Touch handling refs
  const pressTimerIdRef = useRef<number | null>(null);
  const longPressFiredRef = useRef(false);
  const pressedCellRef = useRef<{ r: number; c: number } | null>(null);

  const flagsLeft = useMemo(() => {
    let totalFlags = 0;
    for (const row of grid) {
      for (const c of row) if (c.flagged) totalFlags++;
    }
    return numMines - totalFlags;
  }, [grid]);

  const revealCell = useCallback(
    (r: number, c: number) => {
      if (gameOver) return;
      const current = cloneGrid(grid);
      if (!minesPlaced) {
        placeMines(current, numMines, r, c);
        setMinesPlaced(true);
      }
      if (!current[r] || !current[r][c]) return;
      const cell = current[r][c];
      if (cell.revealed || cell.flagged) return; // no-op
      cell.revealed = true;
      if (cell.mine) {
        setGrid(current);
        setGameOver("lost");
        return;
      }
      // Flood fill zeros
      if (cell.adjacent === 0) {
        const rows = current.length;
        const cols = current[0] ? current[0].length : 0;
        const queue: [number, number][] = [[r, c]];
        const seen = new Set<string>([`${r}:${c}`]);
        while (queue.length) {
          const item = queue.shift();
          if (!item) break;
          const [cr, cc] = item;
          for (const [nr, nc] of neighbors(cr, cc, rows, cols)) {
            if (!current[nr] || !current[nr][nc]) continue;
            const ncell = current[nr][nc];
            if (ncell.revealed || ncell.flagged) continue;
            ncell.revealed = true;
            if (ncell.adjacent === 0 && !ncell.mine) {
              const key = `${nr}:${nc}`;
              if (!seen.has(key)) {
                seen.add(key);
                queue.push([nr, nc]);
              }
            }
          }
        }
      }
      // Check win
      let allSafeRevealed = true;
      outer: for (const row of current) {
        for (const c of row) {
          if (!c.mine && !c.revealed) {
            allSafeRevealed = false;
            break outer;
          }
        }
      }
      if (allSafeRevealed) {
        setGrid(current);
        setGameOver("won");
        return;
      }
      setGrid(current);
    },
    [grid, gameOver, minesPlaced],
  );

  const toggleFlag = useCallback(
    (r: number, c: number) => {
      if (gameOver) return;
      if (!minesPlaced) return; // Do not allow flags before first reveal; simplifies UX
      const current = cloneGrid(grid);
      if (!current[r] || !current[r][c]) return;
      const cell = current[r][c];
      if (cell.revealed) return;
      cell.flagged = !cell.flagged;
      setGrid(current);
    },
    [grid, gameOver, minesPlaced],
  );

  const reset = useCallback(() => {
    setGrid(makeEmptyGrid(gridSize.rows, gridSize.cols));
    setMinesPlaced(false);
    setGameOver(null);
    longPressFiredRef.current = false;
    if (pressTimerIdRef.current !== null) {
      clearTimeout(pressTimerIdRef.current);
      pressTimerIdRef.current = null;
    }
    pressedCellRef.current = null;
  }, []);

  const onCellDown = useCallback(
    (r: number, c: number) => {
      if (gameOver) return;
      pressedCellRef.current = { r, c };
      longPressFiredRef.current = false;
      if (pressTimerIdRef.current !== null) {
        clearTimeout(pressTimerIdRef.current);
        pressTimerIdRef.current = null;
      }
      pressTimerIdRef.current = setTimeout(() => {
        longPressFiredRef.current = true;
        toggleFlag(r, c);
        pressTimerIdRef.current = null;
      }, longPressMs);
    },
    [toggleFlag, gameOver],
  );

  const onCellUp = useCallback(
    (r: number, c: number) => {
      if (gameOver) return;
      const wasPressing = pressedCellRef.current && pressedCellRef.current.r === r && pressedCellRef.current.c === c;
      if (pressTimerIdRef.current !== null) {
        clearTimeout(pressTimerIdRef.current);
        pressTimerIdRef.current = null;
      }
      pressedCellRef.current = null;
      if (!wasPressing) return;
      if (!longPressFiredRef.current) {
        revealCell(r, c);
      }
    },
    [revealCell, gameOver],
  );

  const headerText = useMemo(() => {
    if (gameOver === "won") return "You win!";
    if (gameOver === "lost") return "Boom!";
    return `Flags left: ${flagsLeft}`;
  }, [gameOver, flagsLeft]);

  return (
    <>
      {/* Board container */}
      <vita-rect x={playfield.x} y={playfield.y} width={playfield.w} height={playfield.h} color={theme.surface} borderRadius={0.15}>
        {/* Header inside the board */}
        <vita-rect x={0} y={0} width={playfield.w} height={headerHeight} color={theme.navBackground}>
          <vita-text fontSize={24} color={theme.navText}>
            {headerText}
          </vita-text>
          {/* Reset button lives inside header; coordinates relative to header */}
          <vita-button
            x={playfield.w - boardPadding - resetButton.width}
            y={(headerHeight - resetButton.height) / 2}
            width={resetButton.width}
            height={resetButton.height}
            label={"Reset"}
            onClick={reset}
            color={theme.accent}
            textColor={theme.navText}
            borderRadius={0.2}
          />
        </vita-rect>
        {/* Grid within padded content area */}
        {grid.map((row, r) => {
          return row.map((cell, c) => {
            const x = boardPadding + c * cellSize.width;
            const y = headerHeight + boardPadding + r * cellSize.height;
            const isRevealed = cell.revealed || (gameOver === "lost" && cell.mine);
            const baseColor = isRevealed ? theme.surfaceAlt : theme.primary;
            const mineColor = gameOver === "lost" && cell.mine ? theme.danger : baseColor;
            const fillColor = cell.flagged ? theme.warning : mineColor;
            const label = isRevealed && !cell.mine && cell.adjacent > 0 ? String(cell.adjacent) : "";
            const textColor = isRevealed ? theme.text : theme.buttonText;
            return (
              <vita-button
                key={cell.id}
                x={x}
                y={y}
                width={cellSize.width - 2}
                height={cellSize.height - 2}
                color={fillColor}
                textColor={textColor}
                label={label}
                onMouseDown={() => onCellDown(r, c)}
                onMouseUp={() => onCellUp(r, c)}
                borderRadius={0.1}
              />
            );
          });
        })}
      </vita-rect>
    </>
  );
};
