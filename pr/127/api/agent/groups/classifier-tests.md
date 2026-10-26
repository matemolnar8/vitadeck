Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify e7d27d003d9cf91aaf9645382cb60facbc614bf0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               e7d27d003d9cf91aaf9645382cb60facbc614bf0

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... HEAD

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0

Review concern 02 of 04: Classification tests and Twemoji round-trip (`classifier-tests`)

Part: Emoji classification

The why:

Proves the classifier against the issue's examples and every Twemoji 17.0.3 image.

The what:

New `expect_cluster` cases cover U+1F000, U+1F004, U+1F170 with and without U+FE0F, and skin-tone qualification, and the Twemoji round-trip stops inserting U+FE0F before a skin tone.

Look for:
- The old round-trip wrote U+261D U+FE0F U+1F3FB for `261d-1f3fb.png`, which hid that U+261D U+1F3FB never clustered; the old classifier fails 36 Twemoji files under the new round-trip.

Depends on:
- 01 Emoji_Presentation above U+1F000 (`classifier`)

Hunk refs for this concern:
- tests/glyph_cache_harness.c@361+362
- tests/glyph_cache_harness.c@541+552
- tests/glyph_cache_harness.c@584+595