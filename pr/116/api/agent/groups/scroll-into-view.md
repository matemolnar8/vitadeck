Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 534b52add2e2cb44270ae6afe15f3535ca24cb6f` and `git rev-parse --verify 6b26f72d79e69b431a9c8a58e2496ae8108c74d2` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 534b52add2e2cb44270ae6afe15f3535ca24cb6f 6b26f72d79e69b431a9c8a58e2496ae8108c74d2 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  534b52add2e2cb44270ae6afe15f3535ca24cb6f

head               6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Named refs at pin: 534b52add2e2cb44270ae6afe15f3535ca24cb6f ... 6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Read the diff:

git diff --find-renames 534b52add2e2cb44270ae6afe15f3535ca24cb6f 6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Review concern 01 of 03: Scroll a newly focused control into view (`scroll-into-view`)

Part: Focus scroll-into-view

The why:

The issue asks that a focus change into a Scroll adjust its offset so the focused bounds are visible ([#106](source:s1)).

The what:

set_focus_into_view in input.c calls the new instance_scroll_into_view for D-pad and autoFocus Focus, which walks the focused instance's parents in the laid-out front snapshot (via laid_out_front) and moves each Scroll's offset to the nearest edge that shows its layout bounds, clamped to the content.

Look for:
- Mouse hover and mouse press still move Focus but no longer scroll, so a hovered row that is partly hidden stays where it is under the pointer ([review](source:s3)).
- A reveal calls touch_stop_momentum, so a D-pad press during a flick stops the glide instead of letting it carry the new Focus off screen.
- When the focused control is taller than the viewport, the top edge wins because the top check runs after the bottom one.

Hunk refs for this concern:
- src/ui/instance_tree.c@582+609
- src/ui/instance_tree.h
- src/ui/input.c@220+220
- src/ui/input.c@512+520
- src/ui/input.c@526+534
- tests/input_harness.c@2+2
- tests/input_harness.c@218+229
- tests/input_harness.c@385+621
- CONTEXT.md