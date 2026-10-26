Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 978568cb5b3b89bc65e656c665ead432947b05c6` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 978568cb5b3b89bc65e656c665ead432947b05c6 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               978568cb5b3b89bc65e656c665ead432947b05c6

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 978568cb5b3b89bc65e656c665ead432947b05c6

Review concern 02 of 04: Hit-transparent Stacks (`hit`)

The why:

The issue asks whether a Stack should take hits in the gap between its children ([#89](source:s1)).

The what:

`hit_test_instance` returns a Stack's id only when it has a fill, and `hit_test_harness` checks both kinds plus the nested-scroll fixture that now flows.

Depends on:
- 01 Scroll in the Stack flow (`layout`)

Hunk refs for this concern:
- src/ui/instance_tree.c
- tests/hit_test_harness.c