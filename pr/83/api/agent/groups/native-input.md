Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 07: Native input: events, pad buttons, hover focus (`native-input`)

The why:

Delivers the native half of [#46](source:s1): focus events, Pad Buttons with host keys, and mouse hover moving Focus.

The what:

`input.c` splits each poll into a raylib sample and an `input_apply_*` step, emits the renamed events through `push_event`, and `jslib.c` routes the new `EVT_PAD_BUTTON` to `vitadeck.input.onPadButtonFromNative`; `input_harness` drives the apply step without a window.

Look for:
- Behavior change: hovering a Button focuses it, and moving the pointer onto empty space or a Rect keeps Focus; before, any hover change cleared Focus.
- Subtle: `hide_shell_to_deck` calls `input_ignore_held_pad_buttons`, so the Circle or Backspace that closes the Shell is ignored until released, even though the Deck App polls run in the same frame.
- Subtle: `input_clear_focus`, called by `js_runtime.c` on start and stop, now also resets Pad Buttons and ignores held ones, so a button held across a restart gets neither `pressend` nor a new `pressstart`.
- Touch hover no longer emits any event; `touch_hovered_id` only drives the hover tint.

Depends on:
- 01 autoFocus channel in the instance tree (`focus-request`)

Hunk refs for this concern:
- src/ui/input.c
- src/ui/input.h
- src/core/event_queue.h
- src/jslib/jslib.c
- src/shell/shell.c
- tests/input_harness.c
- CMakeLists.txt