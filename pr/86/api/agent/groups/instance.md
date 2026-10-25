Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` and `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

head               2d9850830f3b8f421e81f7d6750564e89db51ccc

Named refs at pin: origin/feat/focus-and-pad-input ... feat/text-input

Read the diff:

git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc

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