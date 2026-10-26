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

Review concern 02 of 05: Text takes its slot in a Stack (`layout`)

The why:

A measured Text lets `Stack` flow it like any child ([#104](source:s1)).

The what:

`own_bounds` sizes a Text from `text_lines()`, `layout_flow` flows every child except raw text, and `stack_rect_texts` applies the old Rect-only line offset; the harness checks both with the Runtime Default Font that `LAYOUT_SOURCES` links in.

Look for:
- A Text directly in a `Stack` or `Scroll` used to sit at its own x/y plus padding, shifted one line per earlier Text sibling; now it takes space in the flow, so the timers example's empty-log Text grows its Scroll's content height.
- The Rect stacking offset is each Text's own line height times the Text siblings before it, as render computed it; the smoke golden is unchanged.
- `test_over_width_text_aligns_in_its_box` checks only `text_align_offset` with `TextLines.width`, because `render.c` needs a window; it does not call `render_text_instance`.

Depends on:
- 01 One line breaker for layout and render (`text-lines`)

Hunk refs for this concern:
- src/ui/layout.c
- src/ui/layout.h
- tests/layout_harness.c
- CMakeLists.txt