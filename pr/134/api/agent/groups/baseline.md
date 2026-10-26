Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8` and `git rev-parse --verify 4a1449f44de68a038d099df1316497ee3f1e37cc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8 4a1449f44de68a038d099df1316497ee3f1e37cc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8

head               4a1449f44de68a038d099df1316497ee3f1e37cc

Named refs at pin: fdd0f22 ... 4a1449f

Read the diff:

git diff --find-renames fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8 4a1449f44de68a038d099df1316497ee3f1e37cc

Review concern 01 of 01: Vita baseline data (`baseline`)

The why:

Future `scripts/bench.sh vita` runs compare against this baseline, so it must describe current `main`.

The what:

`meta.json` names the run (commit `fdd0f22`, `dirty: false`) whose scenario numbers replace every value in `result.json`.

Hunk refs for this concern:
- tests/bench/baselines/vita/meta.json
- tests/bench/baselines/vita/result.json