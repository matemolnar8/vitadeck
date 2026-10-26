Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 81705ef32a1e4968734146a11cbd9567880f5f7e` and `git rev-parse --verify 43c2875d41d7db87736e061a46b0615e02a57531` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 43c2875d41d7db87736e061a46b0615e02a57531 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  81705ef32a1e4968734146a11cbd9567880f5f7e

head               43c2875d41d7db87736e061a46b0615e02a57531

Named refs at pin: 81705ef32a1e4968734146a11cbd9567880f5f7e ... 43c2875d41d7db87736e061a46b0615e02a57531

Read the diff:

git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 43c2875d41d7db87736e061a46b0615e02a57531

Review concern 02 of 03: Let the D-pad reach controls scrolled out of view (`reach-hidden-rows`)

Part: Focus scroll-into-view

The why:

Without it the D-pad stops at the last row that touches the viewport, so evenly tiled rows below it are unreachable ([review](source:s3)). Unclipping every Scroll let hidden rows steal moves from controls outside it ([round-2 review](source:s4)).

The what:

find_nearest_focusable passes the current Focus to get_focusable_elements_for, and collect_focusable_instance clips a Scroll that contains Focus only to its x range, while any other Scroll keeps its viewport clip.

Look for:
- Hidden rows inside the focused Scroll share absolute y with whatever sits below it; nearest-centre picks the hidden row first, and test_focus_leaves_scroll checks that Down leaves the Scroll only after its last row.
- From outside a Scroll only its visible rows compete: Down from a header enters a bottom-scrolled list at its first visible row, and Up from a Button below enters at the last visible one (test_focus_outside_scroll_header, test_focus_outside_scroll_below).
- Perf: subtree_contains_id walks each Scroll subtree on every D-pad move, so a nested Scroll is walked once per enclosing Scroll.
- get_focusable_elements (no Focus) keeps viewport clipping, so hit_test_harness expects it to hide scrolled-out rows and get_focusable_elements_for("b1") to keep them.

Depends on:
- 01 Scroll a newly focused control into view (`scroll-into-view`)

Hunk refs for this concern:
- src/ui/instance_tree.c@1+1
- src/ui/instance_tree.c@435+436
- src/ui/instance_tree.c@466+481
- src/ui/instance_tree.c@497+519
- src/ui/instance_tree.h
- src/ui/input.c@479+487
- tests/hit_test_harness.c
- tests/input_harness.c@218+229
- tests/input_harness.c@385+621