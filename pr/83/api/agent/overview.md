Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` in this repository.
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

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Commits:
- b31c0d6 docs: Focus, Pad Button and the host mapping table
- de0d962 feat(examples): minesweeper uses focus color, autoFocus and pad buttons
- 6ca8d96 feat(runtime): dispatch focus, blur and pad buttons; autoFocus on commit
- 62534f5 feat(sdk): Button focus props and usePadButton
- 95723a6 feat(input): native focus events, pad buttons and hover focus

Sources:
- ticket #46 Approved design: FocusProps on Button, usePadButton for Circle/Triangle/Square/L/R, Start and Select stay System Input; approval adds host keyboard and mouse parity.
  https://github.com/matemolnar8/vitadeck/issues/46
- transcript Claude Code session · Oct 25 Implementation brief: host mapping table once in CONTEXT.md, a windowless CTest for pad dispatch and hover focus, golden must not change.

The title:

Focus events, pad buttons, host keyboard and mouse parity

The why:

Issue #46 asks for focus callbacks and direct pad buttons for Deck Apps, and [Máté's approval](source:s1) adds that the host must support keyboard and mouse for everything this adds.

The what (large):

`Button` gains `onFocus`, `onBlur`, `focusColor` and `autoFocus`, and a new `usePadButton` hook delivers Circle, Triangle, Square, L and R. Native input renames its events to `focus`/`blur`/`pressstart`/`pressend`/`press`, reads the extra buttons with host keys, and moves Focus on mouse hover.

Look for:
- Done differently: [the proposal](source:s1) puts `autoFocus` on `nativeCreateButton`; the runtime instead returns true from `finalizeInitialChildren` for autoFocus Buttons and calls a new `nativeRequestFocus(id)` from `commitMount`, so a render React discards never steals Focus.
- Done differently: [the proposal](source:s1) says mouse press clears Focus; with hover moving Focus, a mouse press on a Button now focuses it and a press on empty space clears it. Touch press still clears Focus.
- Breaking for mismatched builds: `nativeCreateButton`/`nativeUpdateButton` now require 21 arguments, so an older `runtime.js` against this host creates no Buttons. The runtime ships in the same build, so only hand-mixed builds hit it.

## Review concerns

### 01 autoFocus channel in the instance tree (`focus-request`)

`instance_back_request_focus` stores an id in the back tree, `instance_tree_swap` publishes it with the snapshot under `snapshot_mutex`, and `instance_take_focus_request` hands it to the UI thread once; `instance_is_button` lets hover ignore Rects and Scrolls.

[groups/focus-request.md](groups/focus-request.md)

### 02 Native input: events, pad buttons, hover focus (`native-input`)

`input.c` splits each poll into a raylib sample and an `input_apply_*` step, emits the renamed events through `push_event`, and `jslib.c` routes the new `EVT_PAD_BUTTON` to `vitadeck.input.onPadButtonFromNative`; `input_harness` drives the apply step without a window.

Depends on:
- 01 autoFocus channel in the instance tree (`focus-request`)

[groups/native-input.md](groups/native-input.md)

### 03 focusColor in the native Button (`focus-color`)

`read_button_props` reads the shared Button arguments, including the new `has_focus_color`/`focus_color`, for both create and update, and `render.c` paints `focus_color` when the Button has Focus and is not pressed.

Depends on:
- 02 Native input: events, pad buttons, hover focus (`native-input`)

[groups/focus-color.md](groups/focus-color.md)

### 04 SDK: FocusProps and usePadButton (`sdk`)

`pad-buttons.ts` holds the subscriber set that `usePadButton` adds to and `dispatchPadButton` (re-exported from `@vitadeck/sdk/internal`) walks, and `VitaButtonProps` now intersects `FocusProps`.

[groups/sdk.md](groups/sdk.md)

### 05 Runtime: dispatch and autoFocus (`runtime`)

`input.ts` maps the renamed native events to `onFocus`/`onBlur`/press handlers and forwards Pad Buttons to `dispatchPadButton`, while the reconciler passes `focusColor` natively, skips `autoFocus` in `diffProps`, and requests Focus from `commitMount`.

Depends on:
- 04 SDK: FocusProps and usePadButton (`sdk`)

[groups/runtime.md](groups/runtime.md)

### 06 Minesweeper uses the new API (`example`)

Cells take `focusColor` and track the focused cell via `onFocus`/`onBlur`, the first cell autofocuses, Square flags the focused cell and Triangle resets.

Depends on:
- 05 Runtime: dispatch and autoFocus (`runtime`)

[groups/example.md](groups/example.md)

### 07 Glossary and SDK README (`docs`)

`CONTEXT.md` adds **Focus**, **Pad Button**, **Host Pad Button Mapping**, the Select System Input and one Host mappings table that the SDK README links to from its new focus section.

[groups/docs.md](groups/docs.md)