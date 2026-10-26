Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... HEAD

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Review concern 03 of 04: Shapes batch with text through the glyph atlas (`atlas-shapes`)

The why:

Shapes sampled raylib's default font texture and text the glyph atlas, so rlgl opened a new draw call at nearly every switch: 113 a frame in demo-scroll [PR #130](source:s2).

The what:

Every atlas page in `glyph_cache.c` starts with an opaque white square, and `text_upload_pages` points raylib's shapes texture at page 0's, restoring raylib's in `text_reset` before the pages unload.

Look for:
- Perf: the Shell draws labels with raylib's `DrawText`, so once a Deck App creates page 0 the Shell's shapes and labels switch textures, a few dozen extra draw calls while it is open.
- Subtle: `GLYPH_SIZE_MAX` is 1019, so a glyph or emoji 1020-1024 px wide no longer fits a page.

Hunk refs for this concern:
- src/ui/glyph_cache.c
- src/ui/glyph_cache.h
- src/ui/text.c
- tests/glyph_cache_harness.c