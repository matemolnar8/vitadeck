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

Review concern 01 of 04: Scroll in the Stack flow (`layout`)

The why:

The issue asks for a Scroll that takes its stated main-axis size or fills the rest as the last child ([#89](source:s1)).

The what:

`flows()` now accepts `NT_SCROLL`, and `fills_rest()` lets `layout_flow` overwrite a last-child Scroll's fitted main size with what the parent's stated size leaves, which the new `layout_harness` cases pin down.

Look for:
- A filling Scroll is first fitted to its content by its own `layout_flow`, then resized by its parent; this is safe only because a Scroll is always a start-aligned column, so its children do not depend on its size.
- The fill size is clamped at 0: a 40px-tall column holding a 60px Rect gives its Scroll a 0 height, not a negative one.

Hunk refs for this concern:
- src/ui/layout.c
- src/ui/layout.h
- tests/layout_harness.c