Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e` and `git rev-parse --verify 13bca3a2dc9450bfd52c3da86d4c2acb6b9c09f4` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e

head               13bca3a2dc9450bfd52c3da86d4c2acb6b9c09f4

Named refs at pin: 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e ... HEAD

Read the diff:

git diff --find-renames 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e 13bca3a2dc9450bfd52c3da86d4c2acb6b9c09f4

Commits:
- 13bca3a Merge remote-tracking branch 'origin/main' into t3/perf-minesweeper-125
- 100551b test(bench): record the Vita3K baseline after the minesweeper focus fix
- bb2409c perf(minesweeper): keep the focused cell in a ref

Sources:
- ticket #125 focusedCell App state set from every cell's onFocus/onBlur re-renders all 145 Buttons per move.
  https://github.com/matemolnar8/vitadeck/issues/125

The title:

perf(minesweeper): keep the focused cell in a ref

The why:

Each D-pad move in minesweeper cost about 145 ms of JS on the Vita while changing nothing on screen through React ([#125](source:s1)).

The what (small):

Minesweeper tracks the focused cell in a ref instead of App state, so a D-pad move no longer re-renders the board. The Vita3K bench baseline is re-recorded with the faster numbers.

Look for:
- [#125](source:s1) proposes an SDK change so `commitUpdate` re-registers handlers cheaply; the PR makes none, because `updateHandlers` in `js/packages/runtime/src/input.ts` is already a single `Map.set` and the cost was React re-rendering 145 Buttons.
- [#125](source:s1) verifies with `scripts/bench.sh vita`; this PR measures in Vita3K only and leaves `tests/bench/baselines/vita/` as it was, since the Vita is Máté's device.

## Review concerns

### 01 Focused cell in a ref (`focus-ref`)

The cell `onFocus`/`onBlur` handlers write `focusedCellRef` and the Square `usePadButton` handler reads it, replacing the `focusedCell` state.

[groups/focus-ref.md](groups/focus-ref.md)

### 02 Vita3K baseline (`baseline`)

`meta.json` and `result.json` under `tests/bench/baselines/vita3k/` come from one full Vita3K run at the fix commit; `minesweeper-navigate` drops to 0 commits and about 2.7 ms/s of JS busy time.

Depends on:
- 01 Focused cell in a ref (`focus-ref`)

[groups/baseline.md](groups/baseline.md)