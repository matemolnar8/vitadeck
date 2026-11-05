Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 77464ae1945fca65d55985db4fc3f4aa3bc38bff` and `git rev-parse --verify 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  77464ae1945fca65d55985db4fc3f4aa3bc38bff

head               2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Named refs at pin: 77464ae1945fca65d55985db4fc3f4aa3bc38bff ... 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Read the diff:

git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Commits:
- 2d3ee2e2 test(smoke): add a Shell Home golden with a non-ASCII link name
- 3f548c17 feat(shell): draw Shell text with the Runtime Default Font

Sources:
- transcript Claude Code session · Nov 5 Máté asks to move Shell DrawText/MeasureText calls (and the bootstrap startup/error screens) onto text_draw with the Runtime Default Font, keep layout equivalent, keep the raylib fallback, and not touch the Deck App render path.
- pr PR #149
  https://github.com/matemolnar8/vitadeck/pull/149

The title:

Shell text uses the Runtime Default Font

The why:

The Shell drew with raylib's built-in bitmap font, so it [could not render non-ASCII Deck App names or link labels and looked different from Deck Apps](source:s1).

The what (small):

The VitaDeck Shell, the loading lines and the Deck App failure screen draw through `text_draw` with the Runtime Default Font instead of raylib's `DrawText`, and `bootstrap_draw_deck_canvas` uploads new glyphs every frame so that text appears without a running Deck App.

## Review concerns

### 01 Glyph upload every frame, and the bootstrap screens (`frame`)

`bootstrap_draw_deck_canvas` now starts the text frame and uploads dirty atlas pages before anything draws, and the splash, loading line and failure screen draw through `text_draw`, with failure lines wrapped by `text_cluster_width` instead of per-byte `MeasureText`.

Attention (tradeoff):
Glyphs the Shell or the failure screen cache while drawing upload at the top of the next frame, so a first-seen string shows one frame late. With a ready Deck App, `text_begin_frame` now runs twice per frame (here and in `render_draw_list`), which keeps `src/ui/render.c` untouched; nothing caches glyphs between the two calls.

[groups/frame.md](groups/frame.md)

### 02 Shell draws through text_draw (`shell`)

Three static helpers in `shell.c` (`shell_text`, `shell_text_width`, `shell_text_y`) replace every `DrawText`/`MeasureText` call one to one, and `shell_text_y` centres a line in a row by `text_line_height` rather than the font size.

Depends on:
- 01 Glyph upload every frame, and the bootstrap screens (`frame`)

[groups/shell.md](groups/shell.md)

### 03 Shell Home golden (`golden`)

`smoke_harness.c` opens the Shell over the Failure Screen and screenshots it as `15-shell-home`, and `run_smoke_test.sh` seeds a Host Control Link named `Máté’s 日本 PC` so the golden covers Latin-1, punctuation and the CJK fallback font.

Depends on:
- 02 Shell draws through text_draw (`shell`)

[groups/golden.md](groups/golden.md)