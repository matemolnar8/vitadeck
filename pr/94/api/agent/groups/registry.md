Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` and `git rev-parse --verify a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames e6f928144dc6343cb90c4b20f7c6ecbb247fab82 a9a7092e92ef4a0a4c7e7b17a0cf563ef2d379d3 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 04: Font registry owns and ships the Runtime Fallback Font (`registry`)

The why:

The chain needs a loader that owns the font, and the device needs the file in the VPK.

The what:

`load_fallback_font` in `fonts.c` opens `assets/fonts/DroidSansFallbackFull.ttf` for the glyph cache and frees it at shutdown, and `CMakeLists.txt` packs it with its Apache-2.0 licence into the VPK and `upload_vita`.

Depends on:
- 01 Three-font fallback chain in the glyph cache (`chain`)

Hunk refs for this concern:
- src/ui/fonts.c
- CMakeLists.txt
- assets/fonts/DroidSansFallbackFull-LICENSE.txt