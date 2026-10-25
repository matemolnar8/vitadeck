Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` and `git rev-parse --verify 56d8e56692382c5f4de8a8472ee1cb72238466f4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  4d6d7c5a7816464240a173bb7185dcb1c8090429

head               56d8e56692382c5f4de8a8472ee1cb72238466f4

Named refs at pin: origin/build/warnings-sanitizers-js-limits ... test/native-harnesses

Read the diff:

git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4

Review concern 01 of 07: Event queue harness (`event-queue`)

Part: Native harnesses

The why:

The [event queue was untested](source:s1).

The what:

`event_queue_harness` drives the real `event_queue.c` for FIFO order across the ring boundary, the 256 cap, clear, shutdown, and one concurrent producer, and `CMakeLists.txt` registers it with the other new harnesses.

Look for:
- Pushing 300 events into an empty queue keeps events 0 to 255 and silently drops 256 to 299; the harness pins that, so changing to drop-oldest must update `test_full_queue_drops_newest`.

Hunk refs for this concern:
- tests/event_queue_harness.c
- CMakeLists.txt@242+243