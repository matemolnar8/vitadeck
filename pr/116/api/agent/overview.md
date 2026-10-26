Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 534b52add2e2cb44270ae6afe15f3535ca24cb6f` and `git rev-parse --verify 6b26f72d79e69b431a9c8a58e2496ae8108c74d2` in this repository.
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

base (merge-base)  534b52add2e2cb44270ae6afe15f3535ca24cb6f

head               6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Named refs at pin: 534b52add2e2cb44270ae6afe15f3535ca24cb6f ... 6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Read the diff:

git diff --find-renames 534b52add2e2cb44270ae6afe15f3535ca24cb6f 6b26f72d79e69b431a9c8a58e2496ae8108c74d2

Commits:
- 6b26f72 Merge origin/main into fix/focus-scroll-into-view
- 43c2875 fix(input): hidden Scroll rows only compete from inside their Scroll
- 7b67540 Merge remote-tracking branch 'origin/main' into t3/focus-scroll-into-view-fix
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
- transcript Round-2 review · Oct 26 Unclipping every Scroll let hidden rows compete with controls outside it; unclip only the Scroll that contains Focus, restore viewport-clip expectations, drop the unreachable guard in set_focus_into_view.

The title:

Scroll focused controls into view; honour pad buttons held at startup

The why:

D-pad Focus could move to a control inside a Scroll that stayed off screen, and Pad Buttons held at the runtime's first input sample were ignored until pressed again ([#106](source:s1)).

The what (small):

A Scroll now scrolls to show a control the D-pad or autoFocus moves Focus to, the D-pad can reach controls scrolled out of view, and a Pad Button already held when a Deck App starts now reports one onPressStart instead of being ignored.

## Review concerns

### 01 Scroll a newly focused control into view (`scroll-into-view`)

set_focus_into_view in input.c calls the new instance_scroll_into_view for D-pad and autoFocus Focus, which walks the focused instance's parents in the laid-out front snapshot (via laid_out_front) and moves each Scroll's offset to the nearest edge that shows its layout bounds, clamped to the content.

[groups/scroll-into-view.md](groups/scroll-into-view.md)

### 02 Let the D-pad reach controls scrolled out of view (`reach-hidden-rows`)

find_nearest_focusable passes the current Focus to get_focusable_elements_for, and collect_focusable_instance clips a Scroll that contains Focus only to its x range, while any other Scroll keeps its viewport clip.

Depends on:
- 01 Scroll a newly focused control into view (`scroll-into-view`)

[groups/reach-hidden-rows.md](groups/reach-hidden-rows.md)

### 03 Press Pad Buttons held when a Deck App starts (`held-pad-buttons`)

input_clear_focus resets Pad Button states without setting pad_ignore_held, so a held button presses on the next sample unless the Shell hide that preceded the restart already asked to ignore it.

[groups/held-pad-buttons.md](groups/held-pad-buttons.md)