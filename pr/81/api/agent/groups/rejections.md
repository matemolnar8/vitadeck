Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Review concern 01 of 04: Rejection tracker on the JS thread (`rejections`)

Part: Unhandled rejections

The why:

[Unhandled rejections never reach the Failure Screen](source:s1).

The what:

`track_rejection` in `js_runtime.c` collects unhandled promises, `drain_microtasks` reports the survivors through the new `js_report_rejection`, which shares `report_value` with `js_report_exception`.

Look for:
- Subtle: rejections are judged once per `drain_microtasks`, so a `catch` attached in a later `setTimeout` arrives too late and the Deck App already shows the Failure Screen.
- The Failure Screen message is the rejection reason itself (e.g. `Error: boom`); only the log line carries the `[unhandled rejection]` source tag.

Hunk refs for this concern:
- src/core/js_runtime.c
- src/jslib/error.c
- src/jslib/jslib.h
- CONTEXT.md