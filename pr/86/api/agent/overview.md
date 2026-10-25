Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 6ec5488d92569d22ca087ea754dd8d016282272b` and `git rev-parse --verify d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc` in this repository.
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

base (merge-base)  6ec5488d92569d22ca087ea754dd8d016282272b

head               d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

Named refs at pin: 6ec5488 ... d3d67e5

Read the diff:

git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

Commits:
- d3d67e5 docs(context): drop glossary and host mapping blocks the merge duplicated
- 1b13b09 Merge origin/main into feat/text-input
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
- ticket #48 Approved design proposal: controlled TextInput, IME dialog on Vita, in-place host editing, onChange once on confirm, no onSubmit.
  https://github.com/matemolnar8/vitadeck/issues/48
- ticket #48 approval Máté approved with one requirement: it must also work on the desktop host as a first-class path.
  https://github.com/matemolnar8/vitadeck/issues/48

The title:

TextInput with Vita IME dialog and host keyboard editing

The why:

Deck Apps need a way to [enter text](source:s1), and the approval requires the desktop host to be a [first-class path](source:s2), not a fallback.

The what (large):

Adds a controlled `TextInput` to `@vitadeck/sdk`, backed by a new `vita-text-input` native instance. A Press starts an edit: the Vita opens `sceImeDialog`, the desktop host edits in place, and `onChange` fires once on confirm.

Look for:
- The [proposal](source:s1) expected a raylib-vita swap change for the common dialog; none is needed because raylib-vita's `SwapScreenBuffer` calls `SDL_GL_SwapWindow`, which calls `vglSwapBuffers(GL_TRUE)` every frame.
- The [proposal](source:s1) put the F1-cancels-editing rule in `src/ui/input.c`; it lives in `src/main.c`, which cancels any edit whenever the Deck App does not own input (Shell open, runtime not ready or failed).
- `TextInput` also takes `borderRadius`, which the [proposal](source:s1) did not list, to match `Button` geometry.
- `maxLength` counts UTF-16 code units like `string.length` on both platforms; the [proposal](source:s1) did not say what it counts.
- Untested on hardware: the Vita path compiles in the Vitasdk Docker build only, and a TextInput with no `placeholder` passes an empty dialog title.

## Review concerns

### 01 Platform text input: IME dialog and host keyboard (`platform`)

`text_input.h` defines one begin/poll/cancel session that `text_input_vita.c` implements with `sceImeDialog` and `text_input_host.c` with raylib keys, and CMake picks one per target.

[groups/platform.md](groups/platform.md)

### 02 String payload on the input event queue (`event`)

`InputEvent` gains an owned `text` pointer for `EVT_TEXT_INPUT`, which the queue frees when full or cleared and `process_input_events` frees after calling `onTextInputFromNative`.

[groups/event.md](groups/event.md)

### 03 vita-text-input instance and rendering (`instance`)

`NT_TEXT_INPUT` props built by `nativeCreateTextInput`/`nativeUpdateTextInput` flow through the snapshot into hit testing, focus collection, Scroll flow layout and `render_text_input_instance`.

Depends on:
- 01 Platform text input: IME dialog and host keyboard (`platform`)

[groups/instance.md](groups/instance.md)

### 04 Editing session in the input loop (`session`)

`push_press` starts an edit through `vd_text_input_begin`, `main.c` polls the session every frame and skips the other polls while it runs, and `input_apply_text_input` pushes `EVT_TEXT_INPUT` on confirm.

Depends on:
- 01 Platform text input: IME dialog and host keyboard (`platform`)
- 02 String payload on the input event queue (`event`)
- 03 vita-text-input instance and rendering (`instance`)

[groups/session.md](groups/session.md)

### 05 SDK TextInput and runtime wiring (`js`)

`TextInput` renders `vita-text-input`, whose props the reconciler passes to `nativeCreateTextInput`/`nativeUpdateTextInput` and whose `onChange` handler `onTextInputFromNative` calls.

Depends on:
- 03 vita-text-input instance and rendering (`instance`)
- 04 Editing session in the input loop (`session`)

[groups/js.md](groups/js.md)

### 06 Chat compose row (`example`)

The chat compose row adds a `TextInput` whose trimmed draft `sendMessage` sends instead of the scripted suggestion, then clears.

Depends on:
- 05 SDK TextInput and runtime wiring (`js`)

[groups/example.md](groups/example.md)

### 07 Glossary, host mappings and SDK README (`docs`)

CONTEXT.md adds **Text Input**, its relationships and three host mapping rows, and the SDK README adds a Text input section; wording only.

Depends on:
- 05 SDK TextInput and runtime wiring (`js`)

[groups/docs.md](groups/docs.md)