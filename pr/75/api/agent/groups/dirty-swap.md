Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` and `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  06341f44bca0e9643f246ebca9f9f173697c5600

head               5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Named refs at pin: origin/refactor/native-util-module ... HEAD

Read the diff:

git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Review concern 01 of 04: Dirty flag and idle sleep (`dirty-swap`)

Part: Idle JS thread

The why:

The JS thread called `instance_tree_swap` every iteration with only `sched_yield` between ([#54](source:s1)).

The what:

Every `native*` instance call in `instance.c` sets the flag that `instance_tree_swap` now checks, and `js_runtime.c` sleeps through the new `vd_sleep_ms` when no input, timer, fetch or swap ran.

Look for:
- Subtle: the mark sits at the top of each native function, before argument checks, so a rejected call still triggers one extra swap; it never skips one.
- Perf: while busy the loop still calls `vd_thread_yield`, which on Vita is a 1 ms `sceKernelDelayThread`, as before.
- Input queued while the thread sleeps waits at most `JS_IDLE_SLEEP_MAX_MS` (4 ms); timers wake on time because the sleep is clamped to `timeouts_next_due`.

Hunk refs for this concern:
- src/core/js_runtime.c
- src/jslib/fetch.c
- src/jslib/jslib.c
- src/jslib/jslib.h
- src/jslib/timeout.c
- src/platform/thread.h
- src/platform/thread_posix.c
- src/platform/thread_vita.c
- src/jslib/instance.c
- src/ui/instance_tree.c@22+23
- src/ui/instance_tree.c@207+209
- src/ui/instance_tree.c@231+241
- src/ui/instance_tree.c@243+254
- src/ui/instance_tree.h@84+87
- src/ui/instance_tree.h@119+123