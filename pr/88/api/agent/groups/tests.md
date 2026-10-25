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

Review concern 07 of 07: Layout harness and Stack hit tests (`tests`)

The why:

Proves the flow math by hand, and that Scroll lands where the old flow put the smoke app's rows.

The what:

`layout_harness.c` checks column, row, gap, padding, align, nesting, unflowed Text and Scroll parity against hand-computed positions, `hit_test_harness.c` checks hit, Focus navigation and scroll lookup through a row Stack, and `scroll_math_harness` drops its moved flow tests and links only `scroll.c`.

Depends on:
- 01 Layout pass and Stack instance type (`layout-pass`)

Hunk refs for this concern:
- tests/layout_harness.c
- tests/hit_test_harness.c
- tests/instance_fixture.h
- tests/scroll_math_harness.c
- CMakeLists.txt@301+304