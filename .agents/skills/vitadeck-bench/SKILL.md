---
name: vitadeck-bench
description: Measures VitaDeck performance with Benchmark Mode on the Vita, in Vita3K, or on the desktop host, and compares a run against the committed baseline. Use when the user asks about performance, CPU use, frame time, FPS, slow rendering or slow React updates, wants a benchmark or baseline, or wants to check whether a change made VitaDeck faster or slower.
metadata:
  internal: true
---

# VitaDeck benchmarks

`scripts/bench.sh` runs VitaDeck in Benchmark Mode ([ADR 0005](../../../docs/adr/0005-benchmark-mode-in-the-shipping-binary.md)), then prints each scenario next to the baseline in `tests/bench/baselines/<target>/`:

```sh
scripts/bench.sh vita3k                       # build the VPK, run every scenario in Vita3K
scripts/bench.sh vita                         # same on the Vita at $PSVITAIP
scripts/bench.sh host minesweeper-navigate    # one scenario on the desktop host
scripts/bench.sh vita3k --no-build            # rerun the existing build
```

Results land in `out-bench/<target>-<time>/` (`result.json`, `meta.json`, `log.txt`). `pnpm --dir js bench:report <run dir> [baseline dir]` reprints any run against any other.

- `vita` is the source of truth. Ask Máté before running it; it is his device. It needs VitaShell's FTP server (1337) and vitacompanion (1338) running, uploads `eboot.bin`, `runtime.js`, the example Deck Apps, and any fonts or emoji the Vita lacks (the first time, about 4000 emoji take most of an hour), and leaves the Vita on the LiveArea when done. Get the IP from Máté rather than scanning for it: a bare connection such as `nc -z` crashes VitaShell's FTP and vitacompanion until the Vita restarts.
- `vita3k` needs the emulator set up (`vitadeck-emulator` skill). Its numbers depend on the machine running it, so compare only against a baseline recorded on the same machine (`meta.json` says which).
- `host` is fast for checking that a scenario works, but says little about the Vita.

## The loop

1. Record a baseline on the target you care about, from `main`: `scripts/bench.sh vita3k --update-baseline`.
2. Make the change, then run `scripts/bench.sh vita3k` and read the change column. The report only calls a change better or worse past 10%, about the run-to-run spread in Vita3K; rerun once before trusting a small win.
3. When a PR changes performance, put the report in the PR body and commit the new baseline with it.

## Scenarios

Each one starts with a Deck App Runtime Restart, plays its setup script, warms up for 3 s, then measures for 10 s while its loop script plays. They live in `SCENARIOS` in `src/core/bench.c`; add one there, scripted against an example Deck App, and re-record the baselines.

| Scenario | What it stresses |
|---|---|
| `minesweeper-idle` | Steady drawing of a dense static screen (145 Buttons) |
| `minesweeper-navigate` | React re-render and commit on every D-pad move (10 per second) |
| `demo-text` | Wrapped Latin, CJK and emoji text |
| `demo-scroll` | Native scrolling of a 40-row Scroll under the left stick, no React work |

## Reading the numbers

- **UI work** is the UI thread's CPU-side frame cost: `input`, `layout` (the render lock, plus layout on frames with a new snapshot), `glyphs`, `draw`, `shell`. **Begin + present** is `BeginDrawing` and `EndDrawing`: GPU sync, buffer swap, vsync and the frame limiter, mostly waiting, and the wait moves between the two from run to run. The frame budget is 16.7 ms.
- **UI/JS thread CPU %** is CPU time the OS charged to each thread over the window, idle polling included. On the Vita it comes from `runClocks`; Vita3K does not implement that, so it shows `-` there.
- **Input latency** is from an input event entering the queue to its JS handler returning, synchronous React render included. When it climbs far past the busy iteration time, the Deck App is not keeping up and events are queuing.
- **JS busy ms/s** is time the JS thread spent outside its idle sleep: input handlers, timers, fetch callbacks, microtasks and snapshot swaps, plus about 250 empty polls a second when idle (a millisecond or two per second in Vita3K). React renders inside these. **Commit** is React's commit phase alone (`prepareForCommit` to `resetAfterCommit`), so busy time minus commit time is mostly React rendering.
- **Native mutations** counts the reconciler's calls into C. Many commits with zero mutations means React re-rendered without changing anything on screen.
- **Startup** is the JS thread from creation to the first published snapshot: `runtime.js`, the Deck App Package Entry and the first render.
