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

Review concern 02 of 02: Run and typecheck SDK tests in test/ (`wiring`)

The why:

Enables the regression test in the dispatch group to run in CI and `agent-check.sh fast`.

The what:

`test:cli` also runs `test/*.test.tsx`, `tsconfig.json` typechecks that folder, and `react-reconciler` with its types joins the SDK's devDependencies from the existing catalog.

Depends on:
- 01 Dispatch iterates a snapshot (`dispatch`)

Hunk refs for this concern:
- js/packages/sdk/package.json
- js/packages/sdk/tsconfig.json