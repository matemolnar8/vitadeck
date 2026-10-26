Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 138c59c9e555099aec58a77c632b43fed0096475` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 138c59c9e555099aec58a77c632b43fed0096475 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 02: A codepoint no font has draws its emoji image (`no-glyph`)

The why:

Text-style emoji such as © or ❤ stay text without U+FE0F; when no font has them they should still draw their Twemoji image rather than a box ([#107](source:s1)).

The what:

`glyph_cache_get` calls `find_emoji` with the codepoint alone when `resolve_glyph` finds no font glyph, and the harness walks every `assets/emoji` image through the same paths `text.c` uses.

Look for:
- Perf: an emoji glyph returned from `glyph_cache_get` is cached in `g_emoji`, not `g_glyphs`, so each later call for that codepoint repeats `resolve_glyph` before the `find_emoji` hit.
- The harness formats file names itself to match `load_emoji_image` in `src/ui/fonts.c`; a change to that format needs the same change in `emoji_file_name`.

Depends on:
- 01 Emoji-presentation symbols start a cluster (`presentation`)

Hunk refs for this concern:
- src/ui/glyph_cache.c@297+298
- src/ui/glyph_cache.c@316+319
- src/ui/glyph_cache.h@73+73
- tests/glyph_cache_harness.c@2+2
- tests/glyph_cache_harness.c@368+372
- tests/glyph_cache_harness.c@443+450
- tests/glyph_cache_harness.c@559+740