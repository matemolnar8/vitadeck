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

Review concern 02 of 05: Text takes its slot in a Stack (`layout`)

The why:

A measured Text lets `Stack` flow it like any child ([#104](source:s1)).

The what:

`own_bounds` sizes a Text from `text_lines()`, `layout_flow` flows every child except raw text, and `stack_rect_texts` applies the old Rect-only line offset; the harness checks both with the Runtime Default Font that `LAYOUT_SOURCES` links in.

Look for:
- A Text directly in a `Stack` or `Scroll` used to sit at its own x/y plus padding, shifted one line per earlier Text sibling; now it takes space in the flow, so the timers example's empty-log Text grows its Scroll's content height.
- The Rect stacking offset is each Text's own line height times the Text siblings before it, as render computed it; the smoke golden is unchanged.

Depends on:
- 01 One line breaker for layout and render (`text-lines`)

Hunk refs for this concern:
- src/ui/layout.c
- src/ui/layout.h
- tests/layout_harness.c
- CMakeLists.txt