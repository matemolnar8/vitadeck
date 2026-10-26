Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47` and `git rev-parse --verify 109d347e5c7b68a18469c3a8cfffda8db7e201c1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e933a90f2ad45d2abbbbfac04a7b9a987bbad47

head               109d347e5c7b68a18469c3a8cfffda8db7e201c1

Named refs at pin: 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 ... 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Read the diff:

git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Review concern 01 of 05: One line breaker for layout and render (`text-lines`)

The why:

Layout must size a Text with the measurement the renderer draws with ([brief](source:s3)), so the breaker leaves `render.c`.

The what:

`text_lines()` in `text.c` takes over the joining, wrapping and widths that `render.c` built per frame, and `render_text_instance` now draws its result at `inst->layout`.

Look for:
- Subtle: `text_lines()` returns one static `TextLines`, overwritten by the next call; both callers, `own_bounds` and `render_text_instance`, read it before calling again.
- `render_children` no longer counts `text_index`, so render draws every Text at `layout.y`; the per-Text line offset now comes from `layout.c`.
- The merge with #113 replaces its `measure_text_width` call in `render_button_instance` with `text_measure` rounded the same way, because this branch removed that helper from `render.c`.

Hunk refs for this concern:
- src/ui/text.c
- src/ui/text.h
- src/ui/render.c