Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2` and `git rev-parse --verify 8468e93ba1ac7e88961ea5137dc35753a0e46643` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2

head               8468e93ba1ac7e88961ea5137dc35753a0e46643

Named refs at pin: 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 ... HEAD

Read the diff:

git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643

Review concern 02 of 05: Emoji clusters and atlas entries (`cache`)

The why:

The [design](source:s1) keys emoji by codepoint sequence and loads them into the atlas on demand like glyphs, and the [change request](source:s3) asks for a fallback character when the images are missing.

The what:

`vd_emoji_cluster` parses a cluster into a `VdEmojiSequence`, and `glyph_cache_get_emoji` resolves its longest prefix with an image through an injected `VdEmojiLoader`, resampling it into the shared RGBA8 pages, or returns the fallback font glyph when no prefix has one; `glyph_cache_harness` checks both with fake loaders.

Look for:
- Perf: the first sight of a (font, size, sequence) reads a file per candidate prefix; a sequence with no image is remembered as a non-colour entry until `glyph_cache_reset`, and those entries are never evicted. With `assets/emoji` empty, each distinct emoji therefore costs one failed `fopen` per prefix once, then hash lookups.
- Subtle: an emoji that loads but finds no room this frame is not cached, so its PNG is decoded again on the next frame.
- Behaviour change: `glyph_cache_get_emoji` now always returns a glyph for the whole cluster. With no image, 🇭🇺 or 👍🏽 draws one `U+FFFD` instead of two `.notdef` boxes, and `❤️` draws DejaVu Sans's `❤`.
- `has_glyph` checks only the requested font and `g_fallback`; it does not go through `glyph_cache_get`'s chain, so a later fallback font (#94) has to be added there too.
- `clear_page` now also drops emoji entries that lived on the cleared page.

Hunk refs for this concern:
- src/ui/glyph_cache.h
- src/ui/glyph_cache.c
- tests/glyph_cache_harness.c