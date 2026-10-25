Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5` and `git rev-parse --verify 76eef5b1bd42f8b9e1039d96814122116bccd798` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 76eef5b1bd42f8b9e1039d96814122116bccd798 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5

head               76eef5b1bd42f8b9e1039d96814122116bccd798

Named refs at pin: 18cbdfd ... feat/scroll-in-stack

Read the diff:

git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 76eef5b1bd42f8b9e1039d96814122116bccd798

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