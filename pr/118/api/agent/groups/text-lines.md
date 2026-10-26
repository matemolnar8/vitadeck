Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 81705ef32a1e4968734146a11cbd9567880f5f7e` and `git rev-parse --verify 6afe270d3820e867d31a904d54495e96f4218c2d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 6afe270d3820e867d31a904d54495e96f4218c2d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  81705ef32a1e4968734146a11cbd9567880f5f7e

head               6afe270d3820e867d31a904d54495e96f4218c2d

Named refs at pin: 81705ef32a1e4968734146a11cbd9567880f5f7e ... 6afe270d3820e867d31a904d54495e96f4218c2d

Read the diff:

git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 6afe270d3820e867d31a904d54495e96f4218c2d

Review concern 01 of 05: One line breaker for layout and render (`text-lines`)

The why:

Layout must size a Text with the measurement the renderer draws with ([brief](source:s3)), so the breaker leaves `render.c`.

The what:

`text_lines()` in `text.c` takes over the joining, wrapping and widths that `render.c` built per frame, and `render_text_instance` now draws its result at `inst->layout`, aligning each line with `text_align_offset` inside `TextLines.width`.

Look for:
- Subtle: `text_lines()` returns one static `TextLines`, overwritten by the next call; both callers, `own_bounds` and `render_text_instance`, read it before calling again.
- `render_children` no longer counts `text_index`, so render draws every Text at `layout.y`; the per-Text line offset now comes from `layout.c`.
- The merge with #113 replaces its `measure_text_width` call in `render_button_instance` with `text_measure` rounded the same way, because this branch removed that helper from `render.c`.
- `render_text_instance` aligns inside `lines->width`, the width layout gives the Text, not `Text.width`, which resolves [Codex](source:s4): a centred 200 px unwrapped line in `width={100}` now starts at its slot's x instead of 50 px left of it. Lines narrower than `width` align as before.

Hunk refs for this concern:
- src/ui/text.c
- src/ui/text.h
- src/ui/render.c