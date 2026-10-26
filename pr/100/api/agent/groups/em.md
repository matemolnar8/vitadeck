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

Review concern 03 of 04: Fallback glyphs at the requested font's em size (`em`)

Part: Korean fallback

The why:

Noto Sans KR's tall line metrics drew Hangul at 80% of the surrounding DejaVu Sans text.

The what:

`glyph_scale` keeps `stbtt_ScaleForPixelHeight` for the requested font and gives a fallback source the same em in pixels, and `glyph_cache_get` passes that scale to `rasterise`.

Look for:
- Behavior change no ticket asks for: Droid Sans Fallback Full glyphs grow about 12% (18.3 to 20.6 px em at size 24), visible in the golden's CJK line.
- The harness now checks a fallback ideograph is as wide as DejaVu Sans' em dash and a Noto syllable 0.92 of it, instead of comparing against the fallback font requested directly.

Depends on:
- 02 Lazy fallback fonts gated by Unicode block (`lazy`)

Hunk refs for this concern:
- src/ui/glyph_cache.c@205+247
- src/ui/glyph_cache.c@258+313
- tests/glyph_cache_harness.c@192+226