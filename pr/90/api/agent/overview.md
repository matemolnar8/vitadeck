Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` and `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` in this repository.
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

base (merge-base)  8944d5cdb7fc7fc450617cca3781bb2cf1808471

head               b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Named refs at pin: origin/feat/stack-layout ... origin/feat/glyph-cache

Read the diff:

git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Commits:
- b6e820d test(smoke): Hungarian and emoji line in the golden
- 18aa4bd test(smoke): refresh Linux golden for glyph cache text
- 9ac5782 docs: font fallback in the glossary and SDK README
- e2cb9ee test(ui): glyph cache harness
- a5aad18 feat: Button font and fontSize props
- 0a148f8 feat(ui): draw text through the glyph cache with Runtime Default Font fallback
- ebb6617 feat(ui): on-demand glyph cache over stb_truetype
- 7a47238 chore(vendor): stb_truetype v1.26, the copy raylib 5.5 compiles

Sources:
- ticket #50 Revised proposal: dynamic glyph cache over stb_truetype, fallback fonts, Twemoji; shipped in three PRs. Slice 1 is the cache, call sites and Button props.
  https://github.com/matemolnar8/vitadeck/issues/50
- commit feat(ui): draw text through the glyph cache with Runtime Default Font fallback
- pr-comment matemolnar8 on PR #90
  https://github.com/matemolnar8/vitadeck/pull/90

The title:

On-demand glyph cache with DejaVu Sans fallback; Button font props

The why:

Máté rejected a Latin-only glyph set and asked for full Unicode coverage including colour emoji. The approved design is a [dynamic glyph cache with fallback fonts](source:s1).

The what (large):

TrueType and OpenType text now rasterises per glyph, at the exact Font Size, into shared atlas pages through stb_truetype. A character the requested Deck App Font lacks falls back to the Runtime Default Font. Button gains `font` and `fontSize`. This is slice 1 of three; CJK fallback and colour emoji come later.

Look for:
- The [proposal](source:s1) uploads misses at the next frame start. Here `render_draw_list` caches every glyph the frame draws before `text_upload_pages`, so new text shows on its first frame. The first smoke run showed the lag as blank text.
- The [proposal](source:s1) keys glyphs by font, glyph id and size, which leaves room for shaping. This change keys by `(font, size, codepoint)`, which is simpler while there is no shaping.
- The [proposal](source:s1) moves the text calls in `shell.c` and `bootstrap.c` to `text_*`. Not done: the Shell and the failure screen still draw with raylib's built-in font, outside this slice's file ownership.
- The [proposal](source:s1) treats DejaVu Sans as a fallback after the requested font. DejaVu Sans is already the Runtime Default Font, so the chain has two fonts and no second copy is vendored.
- The [proposal](source:s1) feeds `.fnt`/`.bdf` fonts into the cache. Here they keep raylib's `DrawTextEx` path and get no fallback.
- No kerning: pairs would cost a GPOS lookup per glyph pair on every per-frame measurement. Text keeps raylib's 1 px tracking, so line widths match the old layout.
- The [proposal's](source:s1) smoke string `Újra 日本語 👍🏽` is not added. Per [Máté's review](source:s3) the smoke header draws Hungarian text with ő and ű plus 🎮, which is a `.notdef` box until slice 3; CJK waits for slice 2.

## Review concerns

### 01 stb_truetype v1.26 (`vendor`)

`vendor/stb_truetype.h` is raylib 5.5's byte-for-byte copy; skip it.

[groups/vendor.md](groups/vendor.md)

### 02 Glyph cache core (`cache`)

`glyph_cache_get` looks up `(font, size, codepoint)` in an stb_ds map and on a miss rasterises from the font or the fallback into a shelf-packed RGBA8 page, which `glyph_cache_harness` checks headlessly, including that every Hungarian letter is a real DejaVu Sans glyph.

Depends on:
- 01 stb_truetype v1.26 (`vendor`)

[groups/cache.md](groups/cache.md)

### 03 Text drawing through the cache (`render`)

`render.c` measures and draws through `text.c`, which takes `VdFont`s from the reworked `fonts.c` and uploads changed atlas pages once per frame after `cache_glyphs` warms them.

Depends on:
- 02 Glyph cache core (`cache`)

[groups/render.md](groups/render.md)

### 04 Button font and fontSize (`button`)

`VitaButtonProps` gains `font`/`fontSize`, the reconciler passes them after the label, and `read_button_props` stores the font name that `render_button_instance` draws with.

Depends on:
- 03 Text drawing through the cache (`render`)

[groups/button.md](groups/button.md)

### 05 Glossary and SDK README (`docs`)

The **Deck App Font** and **Runtime Default Font** entries, and one SDK README paragraph, describe the fallback and the Button font props.

Depends on:
- 03 Text drawing through the cache (`render`)

[groups/docs.md](groups/docs.md)