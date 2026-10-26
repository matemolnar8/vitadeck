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

Review concern 02 of 02: Vita3K baseline (`baseline`)

The why:

The bench skill asks a PR that changes performance to commit the new baseline with it.

The what:

`meta.json` and `result.json` under `tests/bench/baselines/vita3k/` come from one full Vita3K run at the fix commit; `minesweeper-navigate` drops to 0 commits and about 2.7 ms/s of JS busy time.

Depends on:
- 01 Focused cell in a ref (`focus-ref`)

Hunk refs for this concern:
- tests/bench/baselines/vita3k/meta.json
- tests/bench/baselines/vita3k/result.json