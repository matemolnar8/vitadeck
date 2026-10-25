Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` and `git rev-parse --verify 18aa4bde59163aa7744bf6b90f621270956d3ba5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 18aa4bde59163aa7744bf6b90f621270956d3ba5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8944d5cdb7fc7fc450617cca3781bb2cf1808471

head               18aa4bde59163aa7744bf6b90f621270956d3ba5

Named refs at pin: origin/feat/stack-layout ... feat/glyph-cache

Read the diff:

git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 18aa4bde59163aa7744bf6b90f621270956d3ba5

Review concern 01 of 05: stb_truetype v1.26 (`vendor`)

The why:

raylib compiles its copy with `STBTT_STATIC` and installs no `external/` headers, so the cache needs its own copy.

The what:

`vendor/stb_truetype.h` is raylib 5.5's byte-for-byte copy; skip it.

Hunk refs for this concern:
- vendor/stb_truetype.h