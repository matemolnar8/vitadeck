Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 30db9d4b17819151901696ae030c7093afa02eee` and `git rev-parse --verify 4043e588ee5505a0e8d9b8a96ed6e4fdba82edfa` in this repository.
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

base (merge-base)  30db9d4b17819151901696ae030c7093afa02eee

head               4043e588ee5505a0e8d9b8a96ed6e4fdba82edfa

Named refs at pin: 30db9d4b17819151901696ae030c7093afa02eee ... t3/fix-minesweeper-input

Read the diff:

git diff --find-renames 30db9d4b17819151901696ae030c7093afa02eee 4043e588ee5505a0e8d9b8a96ed6e4fdba82edfa

Commits:
- 4043e58 test(smoke): refresh the failure screen golden for the moved runtime.js column
- bdb7e41 Merge remote-tracking branch 'origin/main' into t3/fix-minesweeper-input
- 444f2ce fix(sdk): a pad button handler that re-renders runs once per press

Sources:
- pr PR #135 Root cause, fix, regression test, Vita3K before/after proof and benchmark.
  https://github.com/matemolnar8/vitadeck/pull/135
- transcript Claude Code session · Oct 26 Máté reports that Cross on a minesweeper cell sometimes does nothing and that switching Deck Apps from the Shell hangs, and asks for the root cause, a minimal fix and a regression test.

The title:

fix(sdk): a pad button handler that re-renders runs once per press

The why:

In minesweeper, Cross presses sometimes do nothing and switching Deck Apps from the Shell hangs ([task](source:s2)).

The what (small):

`dispatchPadButton` iterates a copy of its subscriber Set, so a handler whose re-render re-subscribes no longer runs again in the same dispatch; an SDK unit test reproduces the loop through a legacy-root renderer.

Look for:
- The task suspected today's perf PRs (#129, #133, #130, #128); none of them is changed here, because the loop lives in `pad-buttons.ts`, unchanged since #83 ([PR](source:s1)).

## Review concerns

### 01 Dispatch iterates a snapshot (`dispatch`)

The test's `Counter` calls `setState` twice in a Triangle handler, which is the pattern that made `dispatchPadButton`'s live-Set loop visit the subscriber `usePadButton` re-added mid-dispatch.

[groups/dispatch.md](groups/dispatch.md)

### 02 Run and typecheck SDK tests in test/ (`wiring`)

`test:cli` also runs `test/*.test.tsx`, `tsconfig.json` typechecks that folder, and `react-reconciler` with its types joins the SDK's devDependencies from the existing catalog.

Depends on:
- 01 Dispatch iterates a snapshot (`dispatch`)

[groups/wiring.md](groups/wiring.md)

### 03 Failure screen golden (`golden`)

Regenerated output: only the stack frame column changes, to runtime.js:24:9126, so the reader can skip it.

Depends on:
- 01 Dispatch iterates a snapshot (`dispatch`)

[groups/golden.md](groups/golden.md)