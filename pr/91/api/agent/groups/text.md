Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2` and `git rev-parse --verify 8468e93ba1ac7e88961ea5137dc35753a0e46643` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 03 of 05: Drawing and measuring by cluster (`text`)

The why:

Text, Button and TextInput must draw emoji, and wrapping must not split a cluster ([design](source:s1)).

The what:

`text.c` walks clusters instead of codepoints and treats every emoji cluster as one glyph from `glyph_cache_get_emoji`, `render.c` wraps and fits TextInput text by `text_cluster_width`, and `fonts.c` supplies the loader that reads `assets/emoji/<hex-seq>.png` through raylib.

Look for:
- Behaviour change: U+200D and U+FE00–U+FE0F now draw nothing and have no width anywhere; before, they drew the font's missing-glyph box.
- An emoji is a Font Size square whose top sits at the font's ascent; it advances by Font Size plus the 1 px tracking, and keeps only the text colour's alpha.
- The TextInput keep-end fit now measures the whole value once, then drops clusters from the start until the rest fits, because clusters only parse forwards.

Depends on:
- 02 Emoji clusters and atlas entries (`cache`)

Hunk refs for this concern:
- src/ui/text.h
- src/ui/text.c
- src/ui/render.c
- src/ui/fonts.c