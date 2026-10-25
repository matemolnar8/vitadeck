Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify d105b810b495e10a8d957673abeadf9b8055c125` and `git rev-parse --verify 499076450739a23bece60dc7cdcd09d4828225c8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  d105b810b495e10a8d957673abeadf9b8055c125

head               499076450739a23bece60dc7cdcd09d4828225c8

Named refs at pin: d105b810b495e10a8d957673abeadf9b8055c125 ... 499076450739a23bece60dc7cdcd09d4828225c8

Read the diff:

git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8

Review concern 01 of 06: Three-font fallback chain in the glyph cache (`chain`)

The why:

The [proposal](source:s1) puts a CJK font after DejaVu Sans and loads it on first miss.

The what:

`glyph_cache_get` walks the requested font, the fallback, then the font `glyph_cache_set_fallback_loader`'s callback opens once; `has_glyph` asks the same three fonts for an emoji's first character, and the harness skips the cases that need a downloaded font when it is missing.

Look for:
- Subtle: any codepoint both earlier fonts lack opens the 4 MB font, not only CJK; a private-use character such as U+E000 in a Latin-only app triggers the load once.
- A loader that returns NULL is consumed anyway, so a missing font file logs one warning and later misses go straight to the missing-glyph box.
- Subtle: an emoji without an image whose first character only the CJK font has, like `日` + U+FE0F, now opens the 4 MB font through `has_glyph` instead of drawing U+FFFD.

Hunk refs for this concern:
- src/ui/glyph_cache.h
- src/ui/glyph_cache.c
- tests/glyph_cache_harness.c