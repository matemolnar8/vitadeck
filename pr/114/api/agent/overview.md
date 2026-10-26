Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 138c59c9e555099aec58a77c632b43fed0096475` in this repository.
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

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               138c59c9e555099aec58a77c632b43fed0096475

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... fix/emoji-bare-codepoint-lookup

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 138c59c9e555099aec58a77c632b43fed0096475

Commits:
- 138c59c fix(fonts): resolve emoji images for bare and text-presentation codepoints

Sources:
- ticket #107 ✨ renders as a box although Twemoji ships it; audit the codepoint-to-filename mapping and fall back only when no file exists.
  https://github.com/matemolnar8/vitadeck/issues/107

The title:

Resolve emoji images for bare and text-presentation codepoints

The why:

✨ (U+2728) draws the missing-glyph box even though Twemoji ships its image ([#107](source:s1)).

The what (small):

`vd_emoji_cluster` now starts a cluster at Unicode's Emoji_Presentation symbols below U+1F000, and `glyph_cache_get` draws a codepoint's Twemoji image when no font has its glyph. The harness walks every image in `assets/emoji` and checks it resolves.

## Review concerns

### 01 Emoji-presentation symbols start a cluster (`presentation`)

`emoji_by_default` checks the `EMOJI_PRESENTATION` table copied from Unicode 17's emoji-data.txt, and `vd_emoji_cluster` uses it in place of the bare `first < 0x1f000` test.

[groups/presentation.md](groups/presentation.md)

### 02 A codepoint no font has draws its emoji image (`no-glyph`)

`glyph_cache_get` calls `find_emoji` with the codepoint alone when `resolve_glyph` finds no font glyph, and the harness walks every `assets/emoji` image through the same paths `text.c` uses.

Depends on:
- 01 Emoji-presentation symbols start a cluster (`presentation`)

[groups/no-glyph.md](groups/no-glyph.md)