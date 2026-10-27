Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify aca107f03473df2cb635ce7f693158515dac9af1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 aca107f03473df2cb635ce7f693158515dac9af1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               aca107f03473df2cb635ce7f693158515dac9af1

Named refs at pin: origin/main ... origin/t3/drop-pr-attribution-clause

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 aca107f03473df2cb635ce7f693158515dac9af1

Review concern 01 of 01: Pull request body rule (`clause`)

The why:

The clause duplicated attribution that other instructions already add [retro item 1](source:s1).

The what:

The Body bullet keeps problem, fix and agent-check tier and loses its last sentence.

Hunk refs for this concern:
- AGENTS.md