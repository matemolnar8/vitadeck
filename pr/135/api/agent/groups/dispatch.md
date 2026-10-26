Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 30db9d4b17819151901696ae030c7093afa02eee` and `git rev-parse --verify bdb7e41350d161238273acfd43a954f91a66621d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 30db9d4b17819151901696ae030c7093afa02eee bdb7e41350d161238273acfd43a954f91a66621d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  30db9d4b17819151901696ae030c7093afa02eee

head               bdb7e41350d161238273acfd43a954f91a66621d

Named refs at pin: 30db9d4b17819151901696ae030c7093afa02eee ... t3/fix-minesweeper-input

Read the diff:

git diff --find-renames 30db9d4b17819151901696ae030c7093afa02eee bdb7e41350d161238273acfd43a954f91a66621d

Review concern 01 of 02: Dispatch iterates a snapshot (`dispatch`)

The why:

One Triangle press in minesweeper re-ran `reset` forever, which froze the JS thread: later Cross presses were never handled, and `js_runtime_stop` joined a thread that never stopped ([task](source:s2)).

The what:

The test's `Counter` calls `setState` twice in a Triangle handler, which is the pattern that made `dispatchPadButton`'s live-Set loop visit the subscriber `usePadButton` re-added mid-dispatch.

Look for:
- Subtle: a subscriber removed during a dispatch (its component re-rendered or unmounted) still gets that press with its previous closure, and one added during the dispatch waits for the next press.
- The test's `presses > 10` guard turns the infinite loop on `main` into a fast failure ("the press is dispatched again and again") instead of a hung test run.

Hunk refs for this concern:
- js/packages/sdk/src/pad-buttons.ts
- js/packages/sdk/test/pad-buttons.test.tsx