Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fcb9c6928d3a15b16fffaea3d7185875f5370fc2` and `git rev-parse --verify f869d239cde93bf2f60ff7fe25f3c0e22a770d28` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 f869d239cde93bf2f60ff7fe25f3c0e22a770d28 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  fcb9c6928d3a15b16fffaea3d7185875f5370fc2

head               f869d239cde93bf2f60ff7fe25f3c0e22a770d28

Named refs at pin: fcb9c692 ... HEAD

Read the diff:

git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 f869d239cde93bf2f60ff7fe25f3c0e22a770d28

Review concern 01 of 02: Blur the Deck App behind the Shell (`blur`)

The why:

The Shell drew only a tint over the Deck App; the [request](source:s2) asks for a blur.

The what:

`draw_frame` in `src/main.c` and the smoke harness wrap `bootstrap_draw_deck_canvas` in `blur_begin`/`blur_end` while the Shell is visible, and `src/ui/blur.c` halves the captured canvas four times before stretching the 60×34 level over the screen.

Attention (tradeoff):
Each Shell frame now costs six render targets (full-size canvas, four 2x downsamples, the screen), each a GXM scene on the Vita. Dropping the upsample chain cut three scenes; the stretched 60×34 level is slightly blockier but sits under the Shell's alpha-200 tint. With the Shell hidden, `draw_frame` takes the old path.

Hunk refs for this concern:
- src/ui/blur.c
- src/ui/blur.h
- CMakeLists.txt
- src/main.c@5+5
- src/main.c@17+18
- src/core/bootstrap.c
- tests/smoke_harness.c
- tests/fixtures/golden/Linux/15-shell-home.png