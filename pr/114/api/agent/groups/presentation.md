Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 138c59c9e555099aec58a77c632b43fed0096475` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 138c59c9e555099aec58a77c632b43fed0096475 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               138c59c9e555099aec58a77c632b43fed0096475

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... fix/emoji-bare-codepoint-lookup

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 138c59c9e555099aec58a77c632b43fed0096475

Review concern 01 of 02: Emoji-presentation symbols start a cluster (`presentation`)

The why:

Before this, only codepoints from U+1F000 up or ones followed by U+FE0F or a keycap mark started a cluster, so ✨ never reached the image lookup ([#107](source:s1)).

The what:

`emoji_by_default` checks the `EMOJI_PRESENTATION` table copied from Unicode 17's emoji-data.txt, and `vd_emoji_cluster` uses it in place of the bare `first < 0x1f000` test.

Look for:
- Subtle: U+FE0E right after any codepoint now returns 0 from `vd_emoji_cluster`, so `🎮︎` draws through the text path; with no font glyph it still gets the image from the fallback in the next group.

Hunk refs for this concern:
- src/ui/glyph_cache.c@14+14
- src/ui/glyph_cache.c@500+505
- src/ui/glyph_cache.c@523+547
- src/ui/glyph_cache.h@99+99
- tests/glyph_cache_harness.c@357+358
- tests/glyph_cache_harness.c@413+418