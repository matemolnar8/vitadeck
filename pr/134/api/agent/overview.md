Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8` and `git rev-parse --verify 4a1449f44de68a038d099df1316497ee3f1e37cc` in this repository.
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

base (merge-base)  fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8

head               4a1449f44de68a038d099df1316497ee3f1e37cc

Named refs at pin: fdd0f22 ... 4a1449f

Read the diff:

git diff --find-renames fdd0f22b606ff6068c0a3234ef5014d4d9ad40a8 4a1449f44de68a038d099df1316497ee3f1e37cc

Commits:
- 4a1449f test(bench): record the Vita baseline at fdd0f22

Sources:
- transcript Claude Code session · Oct 26 Record the Vita baseline from the existing clean run of main at fdd0f22 without running the device; copy result.json and meta.json as --update-baseline does.

The title:

test(bench): record the Vita baseline at fdd0f22

The why:

The last perf PRs (#127–#130, #133) only recorded Vita3K baselines, so the Vita baseline still described `4202f0d`. [Máté asked](source:s1) to update it to current `main`.

The what (trivial):

The committed Vita benchmark baseline moves from `4202f0d` to `fdd0f22`, copied byte for byte from a clean Vita run as `scripts/bench.sh --update-baseline` would.

## Review concerns

### 01 Vita baseline data (`baseline`)

`meta.json` names the run (commit `fdd0f22`, `dirty: false`) whose scenario numbers replace every value in `result.json`.

[groups/baseline.md](groups/baseline.md)