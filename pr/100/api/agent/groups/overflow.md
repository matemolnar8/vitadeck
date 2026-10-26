Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify e8f42a8e8117edf39ed40bcb14474eb1cc655439` and `git rev-parse --verify aacbaa4a6afbf91001dd2e499f830d2dc47df3ea` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames e8f42a8e8117edf39ed40bcb14474eb1cc655439 aacbaa4a6afbf91001dd2e499f830d2dc47df3ea -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  e8f42a8e8117edf39ed40bcb14474eb1cc655439

head               aacbaa4a6afbf91001dd2e499f830d2dc47df3ea

Named refs at pin: e8f42a8e8117edf39ed40bcb14474eb1cc655439 ... aacbaa4

Read the diff:

git diff --find-renames e8f42a8e8117edf39ed40bcb14474eb1cc655439 aacbaa4a6afbf91001dd2e499f830d2dc47df3ea

Review concern 01 of 04: Overflow glyphs draw a box and warn once (`overflow`)

Part: Atlas overflow

The why:

Overflow glyphs drew nothing and logged nothing [#92](source:s1).

The what:

`rasterise` and `rasterise_emoji` return the glyph's box with `page` -1 and count it in `overflowed`, which `text_upload_pages` warns about once and `text_draw` outlines.

Look for:
- An emoji with no room returns `color` false, so its box is tinted with the text colour like other overflow boxes.
- The warning flag lives in `text.c` and survives `text_reset`, so a package switch does not warn again.

Hunk refs for this concern:
- src/ui/glyph_cache.c@62+62
- src/ui/glyph_cache.c@223+274
- src/ui/glyph_cache.c@283+336
- src/ui/glyph_cache.c@397+449
- src/ui/glyph_cache.c@408+461
- src/ui/glyph_cache.h@6+6
- src/ui/glyph_cache.h@27+28
- src/ui/glyph_cache.h@52+63
- src/ui/text.c
- tests/glyph_cache_harness.c@422+501