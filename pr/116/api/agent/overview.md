Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3` and `git rev-parse --verify 087217de216a123c2dfed17077b0af7afa71ed6f` in this repository.
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

base (merge-base)  27683e0ceec2ca0ab7c185c2e97d4f79787f16f3

head               087217de216a123c2dfed17077b0af7afa71ed6f

Named refs at pin: 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 ... 087217de216a123c2dfed17077b0af7afa71ed6f

Read the diff:

git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 087217de216a123c2dfed17077b0af7afa71ed6f

Commits:
- 087217d fix(input): press Pad Buttons held when a Deck App starts
- 453fcf2 fix(input): scroll a newly focused control into view

Sources:
- ticket #106 Scroll the focused control into view on focus change; treat Pad Buttons held at startup as pressed or document the edge; cover both in input_harness.
  https://github.com/matemolnar8/vitadeck/issues/106
- pr PR #116 Scroll-into-view on focus change and Pad Buttons held at startup press once; fixes #106.
  https://github.com/matemolnar8/vitadeck/pull/116

The title:

Scroll focused controls into view; honour pad buttons held at startup

The why:

D-pad Focus could move to a control inside a Scroll that stayed off screen, and Pad Buttons held at the runtime's first input sample were ignored until pressed again ([#106](source:s1)).

The what (small):

A Scroll now scrolls to show a control that takes Focus from the D-pad, autoFocus or hover, and a Pad Button already held when a Deck App starts now reports one onPressStart instead of being ignored.

Look for:
- The D-pad still only picks focusables that intersect a Scroll viewport, so a row fully below the viewport is reachable by autoFocus, hover or the left stick but not by the D-pad; the issue does not ask for that, and it is left as a follow-up.

## Review concerns

### 01 Scroll a newly focused control into view (`scroll-into-view`)

set_focus in input.c calls the new instance_scroll_into_view on every new Focus, which walks the focused instance's parents in the front snapshot and moves each Scroll's offset to the nearest edge that shows its layout bounds, clamped to the content.

[groups/scroll-into-view.md](groups/scroll-into-view.md)

### 02 Press Pad Buttons held when a Deck App starts (`held-pad-buttons`)

input_clear_focus resets Pad Button states without setting pad_ignore_held, so a held button presses on the next sample unless the Shell hide that preceded the restart already asked to ignore it.

[groups/held-pad-buttons.md](groups/held-pad-buttons.md)