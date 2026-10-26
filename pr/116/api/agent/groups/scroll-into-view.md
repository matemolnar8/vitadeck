Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3` and `git rev-parse --verify 087217de216a123c2dfed17077b0af7afa71ed6f` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 087217de216a123c2dfed17077b0af7afa71ed6f -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  27683e0ceec2ca0ab7c185c2e97d4f79787f16f3

head               087217de216a123c2dfed17077b0af7afa71ed6f

Named refs at pin: 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 ... 087217de216a123c2dfed17077b0af7afa71ed6f

Read the diff:

git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 087217de216a123c2dfed17077b0af7afa71ed6f

Review concern 01 of 02: Scroll a newly focused control into view (`scroll-into-view`)

Part: Focus scroll-into-view

The why:

The issue asks that a focus change into a Scroll adjust its offset so the focused bounds are visible ([#106](source:s1)).

The what:

set_focus in input.c calls the new instance_scroll_into_view on every new Focus, which walks the focused instance's parents in the front snapshot and moves each Scroll's offset to the nearest edge that shows its layout bounds, clamped to the content.

Look for:
- Every new Focus reveals, mouse press included; touch never focuses, so a touch drag cannot fight the adjustment.
- A reveal calls touch_stop_momentum, so a D-pad press during a flick stops the glide instead of letting it carry the new Focus off screen.
- When the focused control is taller than the viewport, the top edge wins because the top check runs after the bottom one.

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/instance_tree.h
- src/ui/input.c@212+212
- tests/input_harness.c@2+2
- tests/input_harness.c@218+229
- tests/input_harness.c@385+473
- CONTEXT.md