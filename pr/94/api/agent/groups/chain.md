Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` and `git rev-parse --verify a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames e6f928144dc6343cb90c4b20f7c6ecbb247fab82 a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  e6f928144dc6343cb90c4b20f7c6ecbb247fab82

head               a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3

Named refs at pin: origin/feat/color-emoji ... feat/cjk-fallback

Read the diff:

git diff --find-renames e6f928144dc6343cb90c4b20f7c6ecbb247fab82 a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3

Review concern 01 of 04: Three-font fallback chain in the glyph cache (`chain`)

The why:

The [proposal](source:s1) puts a CJK font after DejaVu Sans and loads it on first miss.

The what:

`glyph_cache_get` walks the requested font, the fallback, then the font `glyph_cache_set_fallback_loader`'s callback opens once, and the harness checks each link plus the single lazy load.

Look for:
- Subtle: any codepoint both earlier fonts lack opens the 4 MB font, not only CJK; a private-use character such as U+E000 in a Latin-only app triggers the load once.
- A loader that returns NULL is consumed anyway, so a missing font file logs one warning and later misses go straight to the missing-glyph box.

Hunk refs for this concern:
- src/ui/glyph_cache.h
- src/ui/glyph_cache.c
- tests/glyph_cache_harness.c