Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` and `git rev-parse --verify 18aa4bde59163aa7744bf6b90f621270956d3ba5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 18aa4bde59163aa7744bf6b90f621270956d3ba5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8944d5cdb7fc7fc450617cca3781bb2cf1808471

head               18aa4bde59163aa7744bf6b90f621270956d3ba5

Named refs at pin: origin/feat/stack-layout ... feat/glyph-cache

Read the diff:

git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 18aa4bde59163aa7744bf6b90f621270956d3ba5

Review concern 02 of 05: Glyph cache core (`cache`)

The why:

The approved [design](source:s1) rasterises glyphs on first use at the exact size, with fallback fonts and capped atlas pages.

The what:

`glyph_cache_get` looks up `(font, size, codepoint)` in an stb_ds map and on a miss rasterises from the font or the fallback into a shelf-packed RGBA8 page, which `glyph_cache_harness` checks headlessly.

Look for:
- Perf: each page keeps a 4 MiB CPU mirror besides its 4 MiB texture, so the 4-page cap is 32 MiB worst case. The smoke app uses 1 page with 135 glyphs.
- Subtle: when all pages are full and every page was used this frame, `atlas_allocate` refuses. The glyph returns `page = -1`, is not cached, and draws nothing until a later frame clears the least recently used page.
- `glyph_font_load` rejects a font whose table directory points outside the file. stb_truetype still trusts the table contents, as it did under raylib.

Depends on:
- 01 stb_truetype v1.26 (`vendor`)

Hunk refs for this concern:
- src/ui/glyph_cache.h
- src/ui/glyph_cache.c
- tests/glyph_cache_harness.c
- CMakeLists.txt@282+284