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

Review concern 04 of 07: Focus navigation extraction and hit test harness (`focus-nav`)

Part: Native harnesses

The why:

[Hit testing and focus navigation were untested](source:s1), and the D-pad target choice was a static function mixed with raylib input.

The what:

`focus_nav_pick` in `src/ui/focus_nav.c` takes over the scoring loop from `find_nearest_focusable` in `input.c`, and `hit_test_harness` checks it together with hit order, scroll offsets and focusable clipping on a tree built through the instance-tree API.

Look for:
- Production code in a test PR: `find_nearest_focusable` now returns an index-based pick and calls `free_focusable_elements` on an empty list too, which is a no-op `arrfree(NULL)`; the scoring loop is moved verbatim.

Depends on:
- 02 Back-tree fixture (`fixture`)

Hunk refs for this concern:
- src/ui/focus_nav.c
- src/ui/focus_nav.h
- src/ui/input.c@3+3
- src/ui/input.c@432+421
- src/ui/input.c@448+435
- src/ui/input.c@580+492
- CMakeLists.txt@150+150
- CMakeLists.txt@257+307
- tests/hit_test_harness.c
- CMakeLists.txt@242+243