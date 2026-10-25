Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify d1a60387997054feecfb96ad2f518d0f9cee05f0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               d1a60387997054feecfb96ad2f518d0f9cee05f0

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/deck-app-error-screen

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0

Review concern 04 of 04: Deck App Failure Screen paint (`paint`)

The why:

The issue asks that [the failure paint matters as much as the success paint](source:s1).

The what:

`bootstrap_draw_deck_canvas` replaces the old "failed to start" text with `draw_deck_app_failure_screen`, and `main.c` drops its separate recovery hint and stops polling Deck App input while failed.

Look for:
- `draw_failure_lines` wraps by summing per-glyph `MeasureText` widths each frame; it caps the message at 4 lines and the stack at 12.
- Font and image load failures now carry their error text onto the screen through `js_runtime_fail`; a boot-time font failure is still cleared by `js_runtime_start`, as before.

Depends on:
- 01 Failure state on VdJsRuntime (`failure-state`)

Hunk refs for this concern:
- src/core/bootstrap.c
- src/main.c
- CONTEXT.md