Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 21f616e174d3a9beedead96ee8bb6df07b9b2bca` and `git rev-parse --verify c603dd347c11466593118be5db42c7483c2608a8` in this repository.
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

base (merge-base)  21f616e174d3a9beedead96ee8bb6df07b9b2bca

head               c603dd347c11466593118be5db42c7483c2608a8

Named refs at pin: 21f616e174d3a9beedead96ee8bb6df07b9b2bca ... c603dd347c11466593118be5db42c7483c2608a8

Read the diff:

git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca c603dd347c11466593118be5db42c7483c2608a8

Commits:
- c603dd3 fix(input): the D-pad reaches controls scrolled out of view
- 13d4fea Merge remote-tracking branch 'origin/main' into t3/focus-scroll-review-fixes
- 087217d fix(input): press Pad Buttons held when a Deck App starts
- 453fcf2 fix(input): scroll a newly focused control into view

Sources:
- ticket #106 Scroll the focused control into view on focus change; treat Pad Buttons held at startup as pressed or document the edge; cover both in input_harness.
  https://github.com/matemolnar8/vitadeck/issues/106
- pr PR #116 Scroll-into-view on focus change and Pad Buttons held at startup press once; fixes #106.
  https://github.com/matemolnar8/vitadeck/pull/116
- transcript Merge-gate review · Oct 26 Blocker: the D-pad cannot reach rows outside a Scroll viewport; low: hover scroll cascades under the pointer; nit: input.h wording.

The title:

Scroll focused controls into view; honour pad buttons held at startup

The why:

D-pad Focus could move to a control inside a Scroll that stayed off screen, and Pad Buttons held at the runtime's first input sample were ignored until pressed again ([#106](source:s1)).

The what (small):

A Scroll now scrolls to show a control the D-pad or autoFocus moves Focus to, the D-pad can reach controls scrolled out of view, and a Pad Button already held when a Deck App starts now reports one onPressStart instead of being ignored.

## Review concerns

### 01 Scroll a newly focused control into view (`scroll-into-view`)

set_focus_into_view in input.c calls the new instance_scroll_into_view for D-pad and autoFocus Focus, which walks the focused instance's parents in the front snapshot and moves each Scroll's offset to the nearest edge that shows its layout bounds, clamped to the content.

[groups/scroll-into-view.md](groups/scroll-into-view.md)

### 02 Let the D-pad reach controls scrolled out of view (`reach-hidden-rows`)

collect_focusable_instance clips a Scroll's descendants only to the Scroll's x range, so get_focusable_elements returns hidden rows at their scrolled positions and focus_nav_pick can pick them.

Depends on:
- 01 Scroll a newly focused control into view (`scroll-into-view`)

[groups/reach-hidden-rows.md](groups/reach-hidden-rows.md)

### 03 Press Pad Buttons held when a Deck App starts (`held-pad-buttons`)

input_clear_focus resets Pad Button states without setting pad_ignore_held, so a held button presses on the next sample unless the Shell hide that preceded the restart already asked to ignore it.

[groups/held-pad-buttons.md](groups/held-pad-buttons.md)