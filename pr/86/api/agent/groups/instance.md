Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 6ec5488d92569d22ca087ea754dd8d016282272b` and `git rev-parse --verify d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 03 of 07: vita-text-input instance and rendering (`instance`)

The why:

The reconciler needs a native instance to create, hit-test, focus and paint.

The what:

`NT_TEXT_INPUT` props built by `nativeCreateTextInput`/`nativeUpdateTextInput` flow through the snapshot into hit testing, focus collection, Scroll flow layout and `render_text_input_instance`.

Look for:
- `instance_is_button` is renamed `instance_is_focusable` and also accepts `NT_TEXT_INPUT`, so mouse hover and press now focus a TextInput like a Button.
- The renderer trims overflowing text with `fit_text` instead of a scissor, because a nested `BeginScissorMode` would end a parent Scroll's clip; while editing it keeps the end visible so the caret stays inside.

Depends on:
- 01 Platform text input: IME dialog and host keyboard (`platform`)

Hunk refs for this concern:
- src/ui/instance_tree.h
- src/ui/instance_tree.c
- src/jslib/instance.c
- src/ui/render.c
- tests/instance_fixture.h