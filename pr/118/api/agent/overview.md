Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47` and `git rev-parse --verify 109d347e5c7b68a18469c3a8cfffda8db7e201c1` in this repository.
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

base (merge-base)  3e933a90f2ad45d2abbbbfac04a7b9a987bbad47

head               109d347e5c7b68a18469c3a8cfffda8db7e201c1

Named refs at pin: 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 ... 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Read the diff:

git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Commits:
- 109d347 fix(render): measure centred Button labels with text_measure after merging main
- cf372ab Merge remote-tracking branch 'origin/main' into t3/stack-lays-out-text-1
- 3d85b50 feat(layout): Stack flows Text using measured size

Sources:
- ticket #104 Give Text a measured height so Stack can flow it; document or remove the implicit Rect-child stacking; extend the layout harness.
  https://github.com/matemolnar8/vitadeck/issues/104
- pr PR #118 Measured Text, layout on the UI thread, Rect stacking kept and documented, demo Label without its Rect.
  https://github.com/matemolnar8/vitadeck/pull/118
- transcript Orchestrator brief · Oct 26 Measure in layout.c with the renderer's measurement, keep or remove Rect stacking without changing the smoke golden, extend tests, simplify the demo with identical behaviour.
- pr-comment chatgpt-codex-connector on PR #118
  https://github.com/matemolnar8/vitadeck/pull/118#discussion_r4373361349

The title:

Stack flows Text using measured size

The why:

Every line of text in a `Stack` needed a wrapper `Rect`, and sibling `Text` children of a `Rect` stacked one line apart by an undocumented rule ([#104](source:s1)).

The what (medium):

A `Text` gets a measured layout size from the same line breaker the renderer draws, so `Stack` and `Scroll` flow it. Layout moves to the UI thread, the implicit Rect stacking moves into layout and gets documented, and the demo drops its wrapper `Rect`s.

Look for:
- The brief asked for the demo's behaviour to stay identical ([brief](source:s3)); under Fetch's "No description" the stars line now sits one line higher, because `Label` no longer reserves two lines.
- The brief scoped `instance_tree.*` to Text bounds ([brief](source:s3)); this PR also moves `layout_tree` out of `instance_tree_swap` onto the first UI-thread reader, because the glyph cache is not safe to use from the JS thread.
- Not done: [Codex](source:s4) asks that an over-width centred or right-aligned Text align by its measured width; `own_bounds` reports the widest line while `render_text_instance` still aligns inside `Text.width`, so such a line starts left of its laid-out box.

## Review concerns

### 01 One line breaker for layout and render (`text-lines`)

`text_lines()` in `text.c` takes over the joining, wrapping and widths that `render.c` built per frame, and `render_text_instance` now draws its result at `inst->layout`.

[groups/text-lines.md](groups/text-lines.md)

### 02 Text takes its slot in a Stack (`layout`)

`own_bounds` sizes a Text from `text_lines()`, `layout_flow` flows every child except raw text, and `stack_rect_texts` applies the old Rect-only line offset; the harness checks both with the Runtime Default Font that `LAYOUT_SOURCES` links in.

Depends on:
- 01 One line breaker for layout and render (`text-lines`)

[groups/layout.md](groups/layout.md)

### 03 Lay out on the UI thread (`ui-thread`)

`laid_out_front()` lays out a new front snapshot once, under `snapshot_mutex`, for the five readers of `layout`: render roots, hit test, focusables, scroll metrics and scroll-at.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

[groups/ui-thread.md](groups/ui-thread.md)

### 04 Demo Label without its wrapper Rect (`demo`)

`Label` in `ui.tsx` renders a bare `Text`, each section's `lines={n}` becomes `wrap` because the Text now measures its own height, and `ScrollSection` drops a comment that #98 made untrue.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

[groups/demo.md](groups/demo.md)

### 05 Author docs for Text in Stack (`docs`)

`CONTEXT.md` gains a **Text** entry that defines the measured size and the Rect stacking rule, and the README Layout section and `Stack` JSDoc say that Text flows.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

[groups/docs.md](groups/docs.md)