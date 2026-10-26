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

Review concern 01 of 04: Rounded rect fills without trig (`rounded`)

The why:

The profile put 1.5 ms of minesweeper-idle's 1.85 ms draw span in raylib's DrawRectangleRounded, which calls sinf and cosf 96 times per rect [PR #130](source:s2).

The what:

`draw_rounded_rect` in `render.c` replays raylib 5.5's quad path from a corner-direction table built once per segment count, and the four rounded fill call sites switch to it.

Look for:
- Subtle: the copy is pinned to raylib 5.5's `SUPPORT_QUADS_DRAW_MODE` path; odd or sub-4 segment counts fall back to raylib, and a raylib bump needs a re-check that the vertices still match.

Hunk refs for this concern:
- src/ui/render.c@2+2
- src/ui/render.c@30+31
- src/ui/render.c@71+157
- src/ui/render.c@147+234
- src/ui/render.c@204+291
- src/ui/render.c@272+359