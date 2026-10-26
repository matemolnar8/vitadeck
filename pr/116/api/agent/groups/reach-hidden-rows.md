Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 21f616e174d3a9beedead96ee8bb6df07b9b2bca` and `git rev-parse --verify c603dd347c11466593118be5db42c7483c2608a8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca c603dd347c11466593118be5db42c7483c2608a8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 03: Let the D-pad reach controls scrolled out of view (`reach-hidden-rows`)

Part: Focus scroll-into-view

The why:

Without it the D-pad stops at the last row that touches the viewport, so evenly tiled rows below it are unreachable ([review](source:s3)).

The what:

collect_focusable_instance clips a Scroll's descendants only to the Scroll's x range, so get_focusable_elements returns hidden rows at their scrolled positions and focus_nav_pick can pick them.

Look for:
- Hidden rows below a Scroll share absolute y with whatever sits below the Scroll; nearest-centre picks the hidden row first, and test_focus_leaves_scroll checks that Down leaves the Scroll only after its last row.
- Children of a nested Scroll stay candidates even when the outer Scroll has scrolled that nested Scroll away; the hit_test_harness expectations flip to match.

Depends on:
- 01 Scroll a newly focused control into view (`scroll-into-view`)

Hunk refs for this concern:
- src/ui/instance_tree.c@1+1
- src/ui/instance_tree.c@471+472
- tests/hit_test_harness.c
- tests/input_harness.c@218+229