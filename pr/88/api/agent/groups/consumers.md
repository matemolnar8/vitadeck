Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` and `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2d9850830f3b8f421e81f7d6750564e89db51ccc 8944d5cdb7fc7fc450617cca3781bb2cf1808471 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2d9850830f3b8f421e81f7d6750564e89db51ccc

head               8944d5cdb7fc7fc450617cca3781bb2cf1808471

Named refs at pin: origin/feat/text-input ... feat/stack-layout

Read the diff:

git diff --find-renames 2d9850830f3b8f421e81f7d6750564e89db51ccc 8944d5cdb7fc7fc450617cca3781bb2cf1808471

Review concern 02 of 07: Render, hit testing, Focus and scroll lookup read the layout (`consumers`)

The why:

The proposal has every reader use the same computed rects; this deletes `scroll_flow_step` and its five walkers.

The what:

`instance_tree_swap` runs `layout_tree` on the new snapshot, and every walker in `instance_tree.c` and `render.c` adds `inst->layout` to its parent origin, subtracting a Scroll's offset at use.

Look for:
- A Stack hit-tests like a Rect: a point in the gap between two buttons in a Stack returns the Stack's id, not the enclosing Scroll's.
- `scroll_at_instance` now descends into Stacks as well as Rects, so drag and wheel find a Scroll nested inside a Stack.

Depends on:
- 01 Layout pass and Stack instance type (`layout-pass`)

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/render.c