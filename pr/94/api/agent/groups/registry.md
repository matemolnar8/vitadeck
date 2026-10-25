Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify d105b810b495e10a8d957673abeadf9b8055c125` and `git rev-parse --verify 499076450739a23bece60dc7cdcd09d4828225c8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 03 of 06: Font registry owns the Runtime Fallback Font (`registry`)

The why:

The chain needs a loader that owns the font, and the device needs the file in the VPK.

The what:

`load_fallback_font` in `fonts.c` opens `assets/fonts/DroidSansFallbackFull.ttf` for the glyph cache and frees it at shutdown; the licence file stays tracked and names the download source.

Depends on:
- 01 Three-font fallback chain in the glyph cache (`chain`)

Hunk refs for this concern:
- src/ui/fonts.c
- assets/fonts/DroidSansFallbackFull-LICENSE.txt