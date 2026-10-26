Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e` and `git rev-parse --verify 13bca3a2dc9450bfd52c3da86d4c2acb6b9c09f4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e 13bca3a2dc9450bfd52c3da86d4c2acb6b9c09f4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 01 of 02: Focused cell in a ref (`focus-ref`)

The why:

Only the Square handler reads the focused cell, so storing it in state re-rendered App on every focus and blur ([#125](source:s1)).

The what:

The cell `onFocus`/`onBlur` handlers write `focusedCellRef` and the Square `usePadButton` handler reads it, replacing the `focusedCell` state.

Look for:
- Subtle: moving focus from a cell to the Reset button fires the cell's `onBlur` first, which clears `focusedCellRef`, so Square on Reset does nothing, as before.

Hunk refs for this concern:
- js/examples/minesweeper/src/App.tsx