Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify 411554aaf96796d4f990e94021382e9e01662620` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 411554aaf96796d4f990e94021382e9e01662620 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               411554aaf96796d4f990e94021382e9e01662620

Named refs at pin: origin/main ... HEAD

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 411554aaf96796d4f990e94021382e9e01662620

Review concern 01 of 04: event_queue_is_empty (`queue-probe`)

The why:

The harness needs to see whether the JS thread has taken a press without popping the event itself.

The what:

`event_queue_is_empty` reads `queue.count` under the queue mutex, next to `event_queue_pop`.

Look for:
- Production code in a test PR: a read-only helper with no caller in the app; nothing in the shipped host changes.

Hunk refs for this concern:
- src/core/event_queue.c
- src/core/event_queue.h