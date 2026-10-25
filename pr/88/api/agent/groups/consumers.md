Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8c25053b52e85f60efa5027dffa62bae617dbee1` and `git rev-parse --verify 0f7b8aa7abe6171068e8a6195e17d7545a857733` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8c25053b52e85f60efa5027dffa62bae617dbee1

head               0f7b8aa7abe6171068e8a6195e17d7545a857733

Named refs at pin: 8c25053b52e85f60efa5027dffa62bae617dbee1 ... 0f7b8aa7abe6171068e8a6195e17d7545a857733

Read the diff:

git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733

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