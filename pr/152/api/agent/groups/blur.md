Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fcb9c6928d3a15b16fffaea3d7185875f5370fc2` and `git rev-parse --verify 6adf657dd6eff69bf072e747e9189193259b547c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 6adf657dd6eff69bf072e747e9189193259b547c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  fcb9c6928d3a15b16fffaea3d7185875f5370fc2

head               6adf657dd6eff69bf072e747e9189193259b547c

Named refs at pin: fcb9c692 ... HEAD

Read the diff:

git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 6adf657dd6eff69bf072e747e9189193259b547c

Review concern 01 of 02: Blur the Deck App behind the Shell (`blur`)

The why:

The Shell drew only a tint over the Deck App; the [request](source:s2) asks for a blur.

The what:

`draw_frame` in `src/main.c` and the smoke harness wrap `bootstrap_draw_deck_canvas` in `blur_begin`/`blur_end` while the Shell is visible, and `src/ui/blur.c` halves the captured canvas four times before stretching the 60×34 level over the screen.

Attention (tradeoff):
With the Shell open, the Vita spends 3.85 ms more wall time per frame in the `shell` span, and UI thread CPU rises from 38% to 48.6%. FPS stays at 59.4 with no dropped frames, but only about 7 ms of slack remains. `draw_frame` re-renders and re-blurs the Deck App every Shell frame so live apps stay live behind the tint; a backdrop frozen when the Shell opens measured 2.5 ms of UI work and 13% CPU.

Hunk refs for this concern:
- src/ui/blur.c
- src/ui/blur.h
- CMakeLists.txt
- src/main.c@5+5
- src/main.c@17+18
- src/core/bootstrap.c
- tests/smoke_harness.c
- tests/fixtures/golden/Linux/15-shell-home.png