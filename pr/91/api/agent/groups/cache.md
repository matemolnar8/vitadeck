Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` and `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b6e820df59a1e9576e5d45a9543fa6689edfb6d6

head               e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Named refs at pin: origin/feat/glyph-cache ... feat/color-emoji

Read the diff:

git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Review concern 01 of 05: Emoji clusters and atlas entries (`cache`)

The why:

The [design](source:s1) keys emoji by codepoint sequence and loads them into the atlas on demand like glyphs.

The what:

`vd_emoji_cluster` parses a cluster into a `VdEmojiSequence`, and `glyph_cache_get_emoji` resolves its longest prefix with an image through an injected `VdEmojiLoader`, resampling it into the shared RGBA8 pages, which `glyph_cache_harness` checks with a fake loader.

Look for:
- Perf: the first sight of a (font, size, sequence) reads a file per candidate prefix; a sequence with no image is remembered as a non-colour entry until `glyph_cache_reset`, and those entries are never evicted.
- Subtle: an emoji that loads but finds no room this frame is not cached, so its PNG is decoded again on the next frame.
- `clear_page` now also drops emoji entries that lived on the cleared page.

Hunk refs for this concern:
- src/ui/glyph_cache.h
- src/ui/glyph_cache.c
- tests/glyph_cache_harness.c