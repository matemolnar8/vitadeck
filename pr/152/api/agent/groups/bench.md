Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fcb9c6928d3a15b16fffaea3d7185875f5370fc2` and `git rev-parse --verify b620259d4c8c3c5c93bb958a11dcf4835da87d06` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 b620259d4c8c3c5c93bb958a11dcf4835da87d06 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  fcb9c6928d3a15b16fffaea3d7185875f5370fc2

head               b620259d4c8c3c5c93bb958a11dcf4835da87d06

Named refs at pin: fcb9c692 ... HEAD

Read the diff:

git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 b620259d4c8c3c5c93bb958a11dcf4835da87d06

Review concern 02 of 02: Benchmark the Shell frame (`bench`)

The why:

The [request](source:s2) asks to benchmark the blur's cost, and no scenario kept the Shell open.

The what:

The `shell` flag on `BenchScenario` makes `bench_shell_open` true, and `main.c` presses Start for the Shell whenever that differs from `shell_is_visible`, so `minesweeper-shell` measures the Shell over Minesweeper and later scenarios get it closed.

Attention (risk):
The re-recorded Vita3K baseline moves every scenario, not only `minesweeper-shell`. The PR says the base build shows the same drift on that Mac, so the old numbers no longer described it. The device baseline has no `minesweeper-shell` entry yet.

Hunk refs for this concern:
- src/core/bench.c
- src/core/bench.h
- src/main.c@60+67
- .agents/skills/vitadeck-bench/SKILL.md
- tests/bench/baselines/vita3k/meta.json
- tests/bench/baselines/vita3k/result.json