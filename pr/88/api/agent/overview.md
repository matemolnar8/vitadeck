Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8c25053b52e85f60efa5027dffa62bae617dbee1` and `git rev-parse --verify 0f7b8aa7abe6171068e8a6195e17d7545a857733` in this repository.
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

base (merge-base)  8c25053b52e85f60efa5027dffa62bae617dbee1

head               0f7b8aa7abe6171068e8a6195e17d7545a857733

Named refs at pin: 8c25053b52e85f60efa5027dffa62bae617dbee1 ... 0f7b8aa7abe6171068e8a6195e17d7545a857733

Read the diff:

git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733

Commits:
- 0f7b8aa Merge origin/main into feat/stack-layout
- 8944d5c test(layout): column, row, gap, padding, align, nesting and Scroll parity
- 5f9c796 docs: Stack glossary entry and SDK Layout section
- 49e9d7e feat(examples): lay out the timers rows and buttons with Stack
- 6d12275 feat(runtime): create and update Stack instances; default x/y to 0
- d4ab5a9 feat(sdk): Stack component; x/y optional on Rect, Button, TextInput and Image
- f427852 feat(ui): layout pass per swap, Stack instance, Scroll laid out as a column Stack
- 2d98508 test(input): split the emoji literal so its hex escape ends
- ac74646 docs: Text Input glossary entry, host mappings and SDK README section
- f20e514 feat(examples): write your own message in the chat compose row
- dccbcb4 test(input): text input editing, ending and limits; text event ownership
- a11774f feat(sdk): TextInput component
- 7bc88db feat(runtime): create text inputs and deliver confirmed values to onChange
- b0150c5 feat(ui): vita-text-input instance, editing session and rendering
- 12859cc feat(platform): text input editing via Vita IME dialog and host keyboard
- b31c0d6 docs: Focus, Pad Button and the host mapping table
- de0d962 feat(examples): minesweeper uses focus color, autoFocus and pad buttons
- 6ca8d96 feat(runtime): dispatch focus, blur and pad buttons; autoFocus on commit
- 62534f5 feat(sdk): Button focus props and usePadButton
- 95723a6 feat(input): native focus events, pad buttons and hover focus

Sources:
- ticket #47 Approved design: Stack with direction, gap, padding, align, fit-content sizing; child x/y are margin and offset; Scroll becomes a scrolling column Stack; one layout pass per swap.
  https://github.com/matemolnar8/vitadeck/issues/47
- transcript Orchestrator brief · Oct 25 Use Stack in timers or minesweeper, not smoke; the smoke golden must not change; add tests/layout_harness.c.

The title:

Stack element; Scroll becomes a scrolling column Stack

The why:

Authors position every child by hand today; [issue #47](source:s1) asks for a `Stack` that lays children out, with `Scroll` as a scrolling column Stack so there is one layout path.

The what (large):

A native layout pass computes every instance's parent-relative bounds once per instance-tree swap, a new `Stack` element flows its children along a column or row, and `Scroll` reuses the same flow code. Render, hit testing, Focus collection and scroll lookup read the computed bounds instead of walking the old scroll flow.

Look for:
- [The proposal](source:s1) has the smoke fixture gain a Stack and refreshes the golden; [the brief](source:s2) overrides that, so smoke is untouched, the golden is unchanged, and `timers` uses Stack instead.
- [The proposal](source:s1) makes `x`/`y` optional on Rect, Button and Image; `TextInput` also gets optional `x`/`y`, because it flows in a Stack too.
- [The proposal](source:s1) lists Rect, Button, Image, TextInput and Stack as flowed children, so a `Scroll` inside a Stack or Scroll keeps its own `x`/`y` (measured inside the padding), exactly as a nested Scroll behaved before.

## Review concerns

### 01 Layout pass and Stack instance type (`layout-pass`)

`layout_tree` in `layout.c` fills the new `InstanceLayout` on `ReactInstance` from props, and `NT_STACK` plus a shared `StackProps` (which `NT_SCROLL` now also uses) feed its flow step.

[groups/layout-pass.md](groups/layout-pass.md)

### 02 Render, hit testing, Focus and scroll lookup read the layout (`consumers`)

`instance_tree_swap` runs `layout_tree` on the new snapshot, and every walker in `instance_tree.c` and `render.c` adds `inst->layout` to its parent origin, subtracting a Scroll's offset at use.

Depends on:
- 01 Layout pass and Stack instance type (`layout-pass`)

[groups/consumers.md](groups/consumers.md)

### 03 Native create and update for Stack (`bindings`)

`create_stack` and `update_stack` serve both Scroll (12 arguments, always a start-aligned column) and Stack (14 arguments, adding direction and align) through one `read_stack_props`.

Depends on:
- 01 Layout pass and Stack instance type (`layout-pass`)

[groups/bindings.md](groups/bindings.md)

### 04 SDK Stack component and reconciler case (`sdk-runtime`)

`Stack` and `VitaStackProps` join the SDK, a shared `Position` type makes `x`/`y` optional, and the reconciler maps `vita-stack` props to native arguments while defaulting every `x`/`y` to 0.

Depends on:
- 03 Native create and update for Stack (`bindings`)

[groups/sdk-runtime.md](groups/sdk-runtime.md)

### 05 Timers example uses Stack (`example`)

A column Stack replaces the hand-computed row `y` values, and a row Stack replaces each pair of button `x` values, landing on the same integer positions.

Depends on:
- 04 SDK Stack component and reconciler case (`sdk-runtime`)

[groups/example.md](groups/example.md)

### 06 Glossary and SDK Layout section (`docs`)

`CONTEXT.md` gains a **Stack** entry and three Relationships bullets, and the SDK README gains a Layout section with the proposal's example.

[groups/docs.md](groups/docs.md)

### 07 Layout harness and Stack hit tests (`tests`)

`layout_harness.c` checks column, row, gap, padding, align, nesting, unflowed Text and Scroll parity against hand-computed positions, `hit_test_harness.c` checks hit, Focus navigation and scroll lookup through a row Stack, and `scroll_math_harness` drops its moved flow tests and links only `scroll.c`.

Depends on:
- 01 Layout pass and Stack instance type (`layout-pass`)

[groups/tests.md](groups/tests.md)