Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` and `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 05: Drawing and measuring by cluster (`text`)

The why:

Text, Button and TextInput must draw emoji, and wrapping must not split a cluster ([design](source:s1)).

The what:

`text.c` walks clusters instead of codepoints, `render.c` wraps and fits TextInput text by `text_cluster_width`, and `fonts.c` supplies the loader that reads `assets/emoji/<hex-seq>.png` through raylib.

Look for:
- Behaviour change: U+200D and U+FE00–U+FE0F now draw nothing and have no width anywhere; before, they drew the font's missing-glyph box.
- An emoji is a Font Size square whose top sits at the font's ascent; it advances by Font Size plus the 1 px tracking, and keeps only the text colour's alpha.
- The TextInput keep-end fit now measures the whole value once, then drops clusters from the start until the rest fits, because clusters only parse forwards.

Depends on:
- 01 Emoji clusters and atlas entries (`cache`)

Hunk refs for this concern:
- src/ui/text.h
- src/ui/text.c
- src/ui/render.c
- src/ui/fonts.c