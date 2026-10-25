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

Review concern 04 of 04: Warn on dropped input events (`dropped-events`)

Part: Hash seed race

The why:

Drops on a full queue were silent; the drop-newest policy stays, now with a log line.

The what:

`event_queue_push` logs a `TraceLog` warning after unlocking when the queue is full, and `event_queue_harness` now links raylib and silences the log.

Look for:
- Every dropped event logs one warning, so a sustained overflow logs once per push.

Hunk refs for this concern:
- src/core/event_queue.c
- tests/event_queue_harness.c
- CMakeLists.txt