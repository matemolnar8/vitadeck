Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` and `git rev-parse --verify 56d8e56692382c5f4de8a8472ee1cb72238466f4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  4d6d7c5a7816464240a173bb7185dcb1c8090429

head               56d8e56692382c5f4de8a8472ee1cb72238466f4

Named refs at pin: origin/build/warnings-sanitizers-js-limits ... test/native-harnesses

Read the diff:

git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4

Review concern 05 of 07: Scroll math extraction and harness (`scroll-math`)

Part: Native harnesses

The why:

[Scroll math and momentum were untested](source:s1), and the momentum step lived inside a function that reads raylib frame time.

The what:

`scroll_max_offset` and `scroll_momentum_step` in `scroll.c` replace inline math in `input.c`, and `scroll_math_harness` checks them with `scroll_flow_step` and `scroll_content_height`.

Look for:
- Production code in a test PR: `touch_apply_momentum` still applies friction after the step, so a fling that lands exactly on offset 0 or the max still zeroes velocity before friction, as before.

Hunk refs for this concern:
- src/ui/scroll.c
- src/ui/scroll.h
- src/ui/input.c@81+82
- src/ui/input.c@138+138
- tests/scroll_math_harness.c
- CMakeLists.txt@242+243