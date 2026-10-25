Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2` and `git rev-parse --verify 8468e93ba1ac7e88961ea5137dc35753a0e46643` in this repository.
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

base (merge-base)  2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2

head               8468e93ba1ac7e88961ea5137dc35753a0e46643

Named refs at pin: 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 ... HEAD

Read the diff:

git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643

Commits:
- 8468e93 docs: Twemoji download in setup, build skill and glossary fallback
- 6b81ce7 test(ui): emoji without Twemoji images draw the fallback glyph once
- 531d115 feat(ui): emoji without an image draw a fallback glyph
- a74be52 build: download Twemoji at configure time instead of vendoring it
- f4b78b8 Merge origin/main into feat/color-emoji
- e6f9281 docs: colour emoji in the glossary and SDK README
- 45e1fe9 test(smoke): golden draws the controller and thumbs-up emoji in colour
- 7d32907 feat(ui): colour emoji in the glyph atlas
- d83468d chore(assets): Twemoji v17.0.3 72px PNGs under assets/emoji
- b6e820d test(smoke): Hungarian and emoji line in the golden
- 18aa4bd test(smoke): refresh Linux golden for glyph cache text
- 9ac5782 docs: font fallback in the glossary and SDK README
- e2cb9ee test(ui): glyph cache harness
- a5aad18 feat: Button font and fontSize props
- 0a148f8 feat(ui): draw text through the glyph cache with Runtime Default Font fallback
- ebb6617 feat(ui): on-demand glyph cache over stb_truetype
- 7a47238 chore(vendor): stb_truetype v1.26, the copy raylib 5.5 compiles
- 8944d5c test(layout): column, row, gap, padding, align, nesting and Scroll parity
- 5f9c796 docs: Stack glossary entry and SDK Layout section
- 49e9d7e feat(examples): lay out the timers rows and buttons with Stack
- 6d12275 feat(runtime): create and update Stack instances; default x/y to 0
- d4ab5a9 feat(sdk): Stack component; x/y optional on Rect, Button, TextInput and Image
- f427852 feat(ui): layout pass per swap, Stack instance, Scroll laid out as a column Stack
- 2d98508 test(input): split the emoji literal so its hex escape ends
- ac74646 docs: Text Input glossary entry, host mappings and SDK README section
- f20e514 feat(examples): write your own message in the chat compose row
- dccbcb4 test(input): text input editing, ending and limits; text event ownership
- a11774f feat(sdk): TextInput component
- 7bc88db feat(runtime): create text inputs and deliver confirmed values to onChange
- b0150c5 feat(ui): vita-text-input instance, editing session and rendering
- 12859cc feat(platform): text input editing via Vita IME dialog and host keyboard
- b31c0d6 docs: Focus, Pad Button and the host mapping table
- de0d962 feat(examples): minesweeper uses focus color, autoFocus and pad buttons
- 6ca8d96 feat(runtime): dispatch focus, blur and pad buttons; autoFocus on commit
- 62534f5 feat(sdk): Button focus props and usePadButton
- 95723a6 feat(input): native focus events, pad buttons and hover focus

Sources:
- ticket #50 Approved design: dynamic glyph cache, fallback fonts, Twemoji colour emoji keyed by codepoint sequence; three PRs, this is the emoji slice.
  https://github.com/matemolnar8/vitadeck/issues/50
- pr-comment matemolnar8 on PR #90
  https://github.com/matemolnar8/vitadeck/pull/90
- transcript Máté via orchestrator · Oct 25 Do not commit the emoji; download them with the build system into a gitignored folder, and show a fallback character when they are missing.

The title:

feat(fonts): colour emoji from Twemoji in the glyph atlas

The why:

The approved [design](source:s1) asks for full Unicode coverage including colour emoji, with Twemoji as the art, and [Máté's review](source:s2) asks for an emoji in the smoke golden. [Máté's change request](source:s3) asks to download the images at build time instead of committing them, with a fallback character when they are missing.

The what (medium):

Slice 3 of the glyph-cache design: CMake downloads the pinned Twemoji 72 px PNGs into the gitignored `assets/emoji`, and Text, Button and TextInput draw emoji clusters from them in colour through the existing RGBA8 atlas pages, or a fallback glyph when an image is missing.

Look for:
- The [proposal](source:s1) lets the longest existing file win for ZWJ, skin tone, flag and keycap clusters. Done; when no prefix longer than one emoji has an image, the cluster draws as its first emoji and the rest of the cluster is swallowed.
- The [proposal](source:s1) does not say when a symbol is an emoji. Here a codepoint below U+1F000 is an emoji only when U+FE0F or U+20E3 follows it, so `❤`, `©` and `™`, and emoji-presentation symbols such as `⭐` or `✅` typed without U+FE0F, still draw from the font.
- The [proposal's](source:s1) smoke string `Újra 日本語 👍🏽 👨‍👩‍👧 🇭🇺` is not used. The golden draws 🎮 in a Text and 👍 in a Button label; skin tones, ZWJ and flags are covered by `glyph_cache_harness` only, and CJK waits for slice 2.
- The [proposal](source:s1) credits Twemoji in the README. Credit is in `assets/emoji/README.md` (shipped in the VPK with the licence) and the SDK README; the root README is unchanged.
- The [change request](source:s3) asks for a fallback character. Done differently: a missing image draws the cluster's first codepoint when DejaVu Sans or the Deck App Font has it (so `❤️` stays `❤` and `1️⃣` stays `1`), and `U+FFFD` only when neither does.
- Subtle: the pin is the SHA-256 of GitHub's generated tag archive, not a release asset (Twemoji publishes none). If GitHub ever regenerates that zip differently, every fresh configure warns and builds without emoji until the hash is updated.
- Manual step: `upload_vita` still FTPs only the eboot, runtime and DejaVu Sans (#93), so a device needs this VPK installed once before emoji appear.

## Review concerns

### 01 Twemoji download at configure time (`assets`)

`cmake/twemoji.cmake` downloads the v17.0.3 tag zip once, checks its SHA-256, extracts `assets/72x72` into `assets/emoji` without `-fe0f` and writes a stamp, which `.gitignore` hides, the CI workflows cache and the VPK packs; the licence and attribution stay tracked.

[groups/assets.md](groups/assets.md)

### 02 Emoji clusters and atlas entries (`cache`)

`vd_emoji_cluster` parses a cluster into a `VdEmojiSequence`, and `glyph_cache_get_emoji` resolves its longest prefix with an image through an injected `VdEmojiLoader`, resampling it into the shared RGBA8 pages, or returns the fallback font glyph when no prefix has one; `glyph_cache_harness` checks both with fake loaders.

[groups/cache.md](groups/cache.md)

### 03 Drawing and measuring by cluster (`text`)

`text.c` walks clusters instead of codepoints and treats every emoji cluster as one glyph from `glyph_cache_get_emoji`, `render.c` wraps and fits TextInput text by `text_cluster_width`, and `fonts.c` supplies the loader that reads `assets/emoji/<hex-seq>.png` through raylib.

Depends on:
- 02 Emoji clusters and atlas entries (`cache`)

[groups/text.md](groups/text.md)

### 04 Smoke golden with colour emoji (`smoke`)

The smoke Button label gains 👍 and the refreshed golden shows it and the header's 🎮 in colour instead of the missing-glyph box.

Depends on:
- 03 Drawing and measuring by cluster (`text`)

[groups/smoke.md](groups/smoke.md)

### 05 Glossary, SDK README, setup and build skill (`docs`)

The **Deck App Font** entry says emoji draw from Twemoji or fall back, the SDK README credits Twemoji, and `AGENTS.md` Setup and the `vitadeck-build` skill say configure downloads the images into the gitignored folder.

Depends on:
- 03 Drawing and measuring by cluster (`text`)

[groups/docs.md](groups/docs.md)