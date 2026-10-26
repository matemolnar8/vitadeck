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

Review concern 02 of 04: Lazy fallback fonts gated by Unicode block (`lazy`)

Part: Korean fallback

The why:

Stray codepoints opened the 4 MB CJK font even though the glyph stayed missing [#95](source:s2).

The what:

`glyph_cache_set_lazy_fonts` replaces the single loader with up to two `VdLazyFont`s, and `resolve_glyph` opens each only when its `covers` predicate accepts the codepoint; `fonts.c` wires Droid to `vd_in_cjk_blocks` and Noto Sans KR to `vd_in_hangul_blocks`.

Look for:
- An emoji without an image above U+1F000 no longer opens Droid Sans Fallback Full when looking for its first character, because `glyph_cache_get_emoji` now goes through the same gated `resolve_glyph`.

Hunk refs for this concern:
- src/ui/glyph_cache.c@62+62
- src/ui/glyph_cache.c@105+113
- src/ui/glyph_cache.c@123+133
- src/ui/glyph_cache.c@146+171
- src/ui/glyph_cache.c@258+313
- src/ui/glyph_cache.c@365+423
- src/ui/glyph_cache.h@42+44
- src/ui/glyph_cache.h@60+73
- src/ui/fonts.c
- tests/glyph_cache_harness.c@11+11
- tests/glyph_cache_harness.c@164+165
- tests/glyph_cache_harness.c@172+203
- tests/glyph_cache_harness.c@192+226
- tests/glyph_cache_harness.c@446+539
- tests/glyph_cache_harness.c@457+551
- tests/glyph_cache_harness.c@470+566