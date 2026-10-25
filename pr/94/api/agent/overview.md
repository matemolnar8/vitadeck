Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify d105b810b495e10a8d957673abeadf9b8055c125` and `git rev-parse --verify 499076450739a23bece60dc7cdcd09d4828225c8` in this repository.
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

base (merge-base)  d105b810b495e10a8d957673abeadf9b8055c125

head               499076450739a23bece60dc7cdcd09d4828225c8

Named refs at pin: d105b810b495e10a8d957673abeadf9b8055c125 ... 499076450739a23bece60dc7cdcd09d4828225c8

Read the diff:

git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8

Commits:
- 4990764 Merge origin/main (#91 squashed) into feat/cjk-fallback
- 8cf9dfb build: keep download stamps out of the shipped asset folders
- 4ca6117 docs: runtime fonts download at configure time, with fallbacks
- 97122c1 test(ui): glyph cache harness skips the downloaded-font cases offline
- db9bd3b feat(ui): draw with raylib's font when DejaVu Sans did not download
- 8a01b22 build: download DejaVu Sans at configure time
- 0cb3912 build: download Droid Sans Fallback Full at configure time
- f13ff83 build: one pinned-download function for configure-time assets
- ce205ae Merge branch 'feat/color-emoji' into feat/cjk-fallback
- 8468e93 docs: Twemoji download in setup, build skill and glossary fallback
- 6b81ce7 test(ui): emoji without Twemoji images draw the fallback glyph once
- 531d115 feat(ui): emoji without an image draw a fallback glyph
- a74be52 build: download Twemoji at configure time instead of vendoring it
- f4b78b8 Merge origin/main into feat/color-emoji
- a9a7092 docs: Runtime Fallback Font in the glossary and SDK README
- dfd91ee test(smoke): golden draws Japanese and Chinese from the CJK fallback
- 1d11070 test(ui): glyph cache harness covers the three-font chain and lazy load
- ebbf9b5 feat(ui): lazy CJK fallback after the Runtime Default Font
- 66a8709 chore(assets): vendor Droid Sans Fallback Full for CJK text
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
- ticket #50 Revised proposal: dynamic glyph cache, DejaVu Sans then Droid Sans Fallback Full as fallbacks, Twemoji colour emoji; three PRs.
  https://github.com/matemolnar8/vitadeck/issues/50
- transcript Máté's rule after reviewing #91 No font or emoji binaries in git; the build system downloads all fonts into a gitignored folder, with a fallback when the download fails.

The title:

Lazy CJK fallback via Droid Sans Fallback Full, with runtime fonts downloaded at configure time

The why:

Issue #50 asks for full Unicode text; the approved [revised proposal](source:s1) falls back from the requested font to DejaVu Sans to a bundled CJK font, with the CJK font loading on first miss. The [repo rule](source:s2) keeps font binaries out of git and has the build download them, with a fallback when that fails.

The what (medium):

The glyph cache gains a third fallback font, Droid Sans Fallback Full, which the font registry opens on the first character the requested font and DejaVu Sans both lack. Both runtime fonts now leave git: one CMake function, shared with Twemoji, downloads them pinned by SHA-256 into the gitignored `assets/fonts`, and a build that cannot download them still boots with fallbacks.

Look for:
- Droid Sans Fallback Full has no Hangul syllables, so Korean still draws DejaVu's missing-glyph box; the [proposal](source:s1) does not mention this, and the smoke line uses `日本語 かな 中文` instead of Korean.
- The downloaded `DejaVuSans.ttf` is the upstream 2.37 release build (757,076 bytes), not the 759,720-byte 2.37 build that was vendored; the smoke golden still matches within tolerance.
- The Droid font comes from GitHub's `aosp-mirror/platform_frameworks_base` at the `android-7.1.2_r39` commit, because android.googlesource.com serves raw files only as base64; the bytes match the previously vendored file.

## Review concerns

### 01 Three-font fallback chain in the glyph cache (`chain`)

`glyph_cache_get` walks the requested font, the fallback, then the font `glyph_cache_set_fallback_loader`'s callback opens once; `has_glyph` asks the same three fonts for an emoji's first character, and the harness skips the cases that need a downloaded font when it is missing.

[groups/chain.md](groups/chain.md)

### 02 Configure-time download of runtime fonts and Twemoji (`download`)

`vitadeck_fetch_asset` in `cmake/fetch_asset.cmake` downloads, hash-checks and stamps each pin; `cmake/twemoji.cmake` and `cmake/fonts.cmake` call it, and `CMakeLists.txt` packs only the fonts that arrived into the VPK and `upload_vita`.

[groups/download.md](groups/download.md)

### 03 Font registry owns the Runtime Fallback Font (`registry`)

`load_fallback_font` in `fonts.c` opens `assets/fonts/DroidSansFallbackFull.ttf` for the glyph cache and frees it at shutdown; the licence file stays tracked and names the download source.

Depends on:
- 01 Three-font fallback chain in the glyph cache (`chain`)

[groups/registry.md](groups/registry.md)

### 04 Raylib font when DejaVu Sans is missing (`offline`)

`font_registry_init` becomes `void`: it logs one warning and gives the Runtime Default Font raylib's built-in font when `DejaVuSans.ttf` does not load, and `bootstrap.c` stops treating that as fatal.

Depends on:
- 02 Configure-time download of runtime fonts and Twemoji (`download`)

[groups/offline.md](groups/offline.md)

### 05 Smoke golden draws CJK (`smoke`)

The smoke header gets a `日本語 かな 中文` line beside the Hungarian text, and the Linux golden is refreshed to match.

Depends on:
- 03 Font registry owns the Runtime Fallback Font (`registry`)

[groups/smoke.md](groups/smoke.md)

### 06 Glossary and SDK README (`docs`)

`CONTEXT.md` adds **Runtime Fallback Font** and the raylib fallback of the Runtime Default Font in place, the SDK README names the CJK coverage, and `AGENTS.md` and the build skill say configure downloads the fonts.

Depends on:
- 03 Font registry owns the Runtime Fallback Font (`registry`)

[groups/docs.md](groups/docs.md)