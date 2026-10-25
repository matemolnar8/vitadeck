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

Review concern 04 of 06: Raylib font when DejaVu Sans is missing (`offline`)

The why:

The [repo rule](source:s2) needs a fallback when the download fails; without one `font_registry_init` failed and the app did not open.

The what:

`font_registry_init` becomes `void`: it logs one warning and gives the Runtime Default Font raylib's built-in font when `DejaVuSans.ttf` does not load, and `bootstrap.c` stops treating that as fatal.

Look for:
- Raylib's built-in font has no ő, ű, CJK or emoji, so an offline build draws `?` for them; Deck App Fonts still fall back to the Droid font, if it downloaded.

Depends on:
- 02 Configure-time download of runtime fonts and Twemoji (`download`)

Hunk refs for this concern:
- src/ui/fonts.c
- src/ui/fonts.h
- src/core/bootstrap.c