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

Review concern 01 of 04: Emoji_Presentation above U+1F000 (`classifier`)

Part: Emoji classification

The why:

[The >= U+1F000 shortcut misclassifies non-emoji](source:s1).

The what:

`emoji_by_default` scans one sorted Emoji_Presentation table that now extends past U+1F000, and `vd_emoji_cluster` also starts a cluster when a skin tone follows, as the header comment now says.

Look for:
- Subtle: `a` followed by U+1F3FB now clusters as {U+61, U+1F3FB}; no image exists, so only `a` draws and the swatch disappears, where before the swatch drew alone.
- Subtle: U+1F3F3 U+200D U+1F308 without U+FE0F no longer clusters; the white flag and the rainbow each draw their own image through the first-codepoint fallback in `glyph_cache_get`, as text-default bases below U+1F000 already did.
- Perf: the loop returns false at the first range whose start exceeds the codepoint, so ASCII stops after one comparison instead of scanning all 33 rows as before.

Hunk refs for this concern:
- src/ui/glyph_cache.c
- src/ui/glyph_cache.h