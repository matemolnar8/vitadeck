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

Review concern 02 of 07: String payload on the input event queue (`event`)

The why:

`InputEvent` only holds 32-byte names, so a confirmed value needs its own payload, as the [proposal](source:s1) says.

The what:

`InputEvent` gains an owned `text` pointer for `EVT_TEXT_INPUT`, which the queue frees when full or cleared and `process_input_events` frees after calling `onTextInputFromNative`.

Look for:
- Every popper now owns `evt.text`; `process_input_events` and the input harness `drain` free it, and a new popper that skips `free` leaks under LSan.

Hunk refs for this concern:
- src/core/event_queue.h
- src/core/event_queue.c
- src/jslib/jslib.c
- tests/event_queue_harness.c