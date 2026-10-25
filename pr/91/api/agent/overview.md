Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` and `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` in this repository.
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

base (merge-base)  b6e820df59a1e9576e5d45a9543fa6689edfb6d6

head               e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Named refs at pin: origin/feat/glyph-cache ... feat/color-emoji

Read the diff:

git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Commits:
- e6f9281 docs: colour emoji in the glossary and SDK README
- 45e1fe9 test(smoke): golden draws the controller and thumbs-up emoji in colour
- 7d32907 feat(ui): colour emoji in the glyph atlas
- d83468d chore(assets): Twemoji v17.0.3 72px PNGs under assets/emoji

Sources:
- ticket #50 Approved design: dynamic glyph cache, fallback fonts, Twemoji colour emoji keyed by codepoint sequence; three PRs, this is the emoji slice.
  https://github.com/matemolnar8/vitadeck/issues/50
- pr-comment matemolnar8 on PR #90
  https://github.com/matemolnar8/vitadeck/pull/90

The title:

feat(fonts): colour emoji from Twemoji in the glyph atlas

The why:

The approved [design](source:s1) asks for full Unicode coverage including colour emoji, with Twemoji as the art, and [Máté's review](source:s2) asks for an emoji in the smoke golden.

The what (medium):

Slice 3 of the glyph-cache design: Twemoji 72 px PNGs ship under `assets/emoji`, and Text, Button and TextInput draw emoji clusters from them in colour through the existing RGBA8 atlas pages.

Look for:
- The [proposal](source:s1) lets the longest existing file win for ZWJ, skin tone, flag and keycap clusters. Done; when no prefix longer than one emoji has an image, the cluster draws as its first emoji and the rest of the cluster is swallowed.
- The [proposal](source:s1) does not say when a symbol is an emoji. Here a codepoint below U+1F000 is an emoji only when U+FE0F or U+20E3 follows it, so `❤`, `©` and `™`, and emoji-presentation symbols such as `⭐` or `✅` typed without U+FE0F, still draw from the font.
- The [proposal's](source:s1) smoke string `Újra 日本語 👍🏽 👨‍👩‍👧 🇭🇺` is not used. The golden draws 🎮 in a Text and 👍 in a Button label; skin tones, ZWJ and flags are covered by `glyph_cache_harness` only, and CJK waits for slice 2.
- The [proposal](source:s1) credits Twemoji in the README. Credit is in `assets/emoji/README.md` (shipped in the VPK with the licence) and the SDK README; the root README is unchanged.
- Manual step: `upload_vita` still FTPs only the eboot, runtime and DejaVu Sans, so a device needs this VPK installed once before emoji appear.

## Review concerns

### 01 Emoji clusters and atlas entries (`cache`)

`vd_emoji_cluster` parses a cluster into a `VdEmojiSequence`, and `glyph_cache_get_emoji` resolves its longest prefix with an image through an injected `VdEmojiLoader`, resampling it into the shared RGBA8 pages, which `glyph_cache_harness` checks with a fake loader.

[groups/cache.md](groups/cache.md)

### 02 Drawing and measuring by cluster (`text`)

`text.c` walks clusters instead of codepoints, `render.c` wraps and fits TextInput text by `text_cluster_width`, and `fonts.c` supplies the loader that reads `assets/emoji/<hex-seq>.png` through raylib.

Depends on:
- 01 Emoji clusters and atlas entries (`cache`)

[groups/text.md](groups/text.md)

### 03 Smoke golden with colour emoji (`smoke`)

The smoke Button label gains 👍 and the refreshed golden shows it and the header's 🎮 in colour instead of the missing-glyph box.

Depends on:
- 02 Drawing and measuring by cluster (`text`)

[groups/smoke.md](groups/smoke.md)

### 04 Glossary and SDK README (`docs`)

The **Deck App Font** entry and one SDK README paragraph say emoji draw in colour from Twemoji and where they do not.

Depends on:
- 02 Drawing and measuring by cluster (`text`)

[groups/docs.md](groups/docs.md)

### 05 Vendored Twemoji images (`assets`)

4,009 unmodified Twemoji v17.0.3 72 px PNGs, renamed without `-fe0f`, with the licence and attribution; `CMakeLists.txt` adds the folder to the VPK. Skip the PNGs: they are binary copies.

[groups/assets.md](groups/assets.md)