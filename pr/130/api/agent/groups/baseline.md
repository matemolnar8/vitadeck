Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... HEAD

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Review concern 04 of 04: Vita3K baseline (`baseline`)

The why:

A PR that changes performance commits the new baseline with its report.

The what:

Regenerated Benchmark Mode output from this branch on the same machine as main's baseline; the numbers are the PR's proof, so skim rather than read.

Hunk refs for this concern:
- tests/bench/baselines/vita3k/meta.json
- tests/bench/baselines/vita3k/result.json