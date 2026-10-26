Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify e8f42a8e8117edf39ed40bcb14474eb1cc655439` and `git rev-parse --verify aacbaa4a6afbf91001dd2e499f830d2dc47df3ea` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- aacbaa4 Merge origin/main into fix/glyph-atlas-eviction-and-korean
- f51a30c feat(fonts): Korean fallback; lazy fonts open only for their Unicode blocks
- 0e7a0e7 fix(ui): glyphs with no atlas room draw a box and warn once

Sources:
- ticket #92 Overflow glyphs are skipped silently; evict LRU between frames or fail loudly; add a harness case.
  https://github.com/matemolnar8/vitadeck/issues/92
- ticket #95 Add a Hangul fallback subset, gate lazy loads by Unicode block, extend the harness and smoke line.
  https://github.com/matemolnar8/vitadeck/issues/95

The title:

Evict atlas pages between frames; Korean fallback with block-gated lazy loads

The why:

A frame needing more glyphs than four pages hold silently skipped the extra glyphs [#92](source:s1), and Korean rendered as missing-glyph boxes while stray codepoints loaded the 4 MB CJK font for nothing [#95](source:s2).

The what (medium):

Glyphs that find no atlas room draw as an outlined box with a one-time warning, and Noto Sans KR joins the fallback chain behind lazy fonts that open only for their Unicode blocks. Fallback glyphs now rasterise at the requested font's em size.

Look for:
- Done differently: [#95](source:s2) proposes a subset limited to the 11,172 syllables; the build downloads noto-cjk's prebuilt Korean SubsetOTF (4,644,748 bytes) because cutting it would need a font tool at build time.
- Done differently: [#95](source:s2) gates the CJK font on Hangul too; `vd_in_cjk_blocks` leaves Hangul syllables out because Droid Sans Fallback Full has 3 of 11,172, so Korean text opens only Noto Sans KR.
- [#92](source:s1) offers LRU eviction between frames; `atlas_allocate` already cleared the least recently used page not used this frame, so this change adds only the box, the warning and the harness cases.

## Review concerns

### 01 Overflow glyphs draw a box and warn once (`overflow`)

`rasterise` and `rasterise_emoji` return the glyph's box with `page` -1 and count it in `overflowed`, which `text_upload_pages` warns about once and `text_draw` outlines.

[groups/overflow.md](groups/overflow.md)

### 02 Lazy fallback fonts gated by Unicode block (`lazy`)

`glyph_cache_set_lazy_fonts` replaces the single loader with up to two `VdLazyFont`s, and `resolve_glyph` opens each only when its `covers` predicate accepts the codepoint; `fonts.c` wires Droid to `vd_in_cjk_blocks` and Noto Sans KR to `vd_in_hangul_blocks`.

[groups/lazy.md](groups/lazy.md)

### 03 Fallback glyphs at the requested font's em size (`em`)

`glyph_scale` keeps `stbtt_ScaleForPixelHeight` for the requested font and gives a fallback source the same em in pixels, and `glyph_cache_get` passes that scale to `rasterise`.

Depends on:
- 02 Lazy fallback fonts gated by Unicode block (`lazy`)

[groups/em.md](groups/em.md)

### 04 Noto Sans KR download, smoke line and docs (`asset`)

`cmake/fonts.cmake` fetches the font into `VITADECK_RUNTIME_FONTS`, which the VPK and `upload_vita` already loop over, while the smoke line, golden and glossary show 한글.

Depends on:
- 02 Lazy fallback fonts gated by Unicode block (`lazy`)

[groups/asset.md](groups/asset.md)