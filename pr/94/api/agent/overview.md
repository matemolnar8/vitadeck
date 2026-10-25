Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` and `git rev-parse --verify a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3` in this repository.
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

base (merge-base)  e6f928144dc6343cb90c4b20f7c6ecbb247fab82

head               a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3

Named refs at pin: origin/feat/color-emoji ... feat/cjk-fallback

Read the diff:

git diff --find-renames e6f928144dc6343cb90c4b20f7c6ecbb247fab82 a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3

Commits:
- a9a7092 docs: Runtime Fallback Font in the glossary and SDK README
- dfd91ee test(smoke): golden draws Japanese and Chinese from the CJK fallback
- 1d11070 test(ui): glyph cache harness covers the three-font chain and lazy load
- ebbf9b5 feat(ui): lazy CJK fallback after the Runtime Default Font
- 66a8709 chore(assets): vendor Droid Sans Fallback Full for CJK text

Sources:
- ticket #50 Revised proposal: dynamic glyph cache, DejaVu Sans then Droid Sans Fallback Full as fallbacks, Twemoji colour emoji; three PRs.
  https://github.com/matemolnar8/vitadeck/issues/50

The title:

Lazy CJK fallback via Droid Sans Fallback Full

The why:

Issue #50 asks for full Unicode text; the approved [revised proposal](source:s1) falls back from the requested font to DejaVu Sans to a bundled CJK font, with the CJK font loading on first miss.

The what (small):

The glyph cache gains a third fallback font, Droid Sans Fallback Full, which the font registry opens on the first character the requested font and DejaVu Sans both lack. The VPK ships the font, the smoke golden draws Japanese and Chinese, and the glossary names it the Runtime Fallback Font.

Look for:
- Droid Sans Fallback Full has no Hangul syllables, so Korean still draws DejaVu's missing-glyph box; the [proposal](source:s1) does not mention this, and the smoke line uses `日本語 かな 中文` instead of Korean.
- The vendored AOSP 7.1.2 build is 4.0 MB, not the 4.5 MB the [proposal](source:s1) estimates; the larger AOSP 4.4 build also lacks Hangul.

## Review concerns

### 01 Three-font fallback chain in the glyph cache (`chain`)

`glyph_cache_get` walks the requested font, the fallback, then the font `glyph_cache_set_fallback_loader`'s callback opens once, and the harness checks each link plus the single lazy load.

[groups/chain.md](groups/chain.md)

### 02 Font registry owns and ships the Runtime Fallback Font (`registry`)

`load_fallback_font` in `fonts.c` opens `assets/fonts/DroidSansFallbackFull.ttf` for the glyph cache and frees it at shutdown, and `CMakeLists.txt` packs it with its Apache-2.0 licence into the VPK and `upload_vita`.

Depends on:
- 01 Three-font fallback chain in the glyph cache (`chain`)

[groups/registry.md](groups/registry.md)

### 03 Smoke golden draws CJK (`smoke`)

The smoke header gets a `日本語 かな 中文` line beside the Hungarian text, and the Linux golden is refreshed to match.

Depends on:
- 02 Font registry owns and ships the Runtime Fallback Font (`registry`)

[groups/smoke.md](groups/smoke.md)

### 04 Glossary and SDK README (`docs`)

`CONTEXT.md` adds **Runtime Fallback Font** and rewrites the Deck App Font, Runtime Default Font and fallback entries in place for the three-font chain, and the SDK README names the CJK coverage in one sentence.

Depends on:
- 02 Font registry owns and ships the Runtime Fallback Font (`registry`)

[groups/docs.md](groups/docs.md)