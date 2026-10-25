Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` and `git rev-parse --verify 56d8e56692382c5f4de8a8472ee1cb72238466f4` in this repository.
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

base (merge-base)  4d6d7c5a7816464240a173bb7185dcb1c8090429

head               56d8e56692382c5f4de8a8472ee1cb72238466f4

Named refs at pin: origin/build/warnings-sanitizers-js-limits ... test/native-harnesses

Read the diff:

git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4

Commits:
- 56d8e56 chore(scripts): prefer docker compose in agent-check full
- 46aec64 docs(agents): point the vendored jimp.h at vendor/jimp
- 6340798 test: Runtime Upload Single-Flight harness
- 3691494 test: scroll flow layout, max offset and momentum step harness
- dbf7059 test: hit testing, focusables and D-pad focus navigation harness
- f87dc5d test: snapshot swap vs hit test stress harness (#39)
- 8a35101 test: event queue harness for order, cap and concurrent push

Sources:
- ticket #58 Add harnesses for untested pure modules, run them under sanitizers, and make a two-thread swap stress test possible.
  https://github.com/matemolnar8/vitadeck/issues/58
- ticket #39 Hit-test ids pointed into a snapshot the JS thread could free; asks for a harness that swaps while hit-testing under ASan.
  https://github.com/matemolnar8/vitadeck/issues/39
- transcript Orchestrator brief · Oct 25 Priority-ordered harness list, keep the diff under about 900 added lines, assert current event queue cap behaviour, and two side fixes.

The title:

test: harnesses for event queue, snapshot swap, hit testing, scroll math, upload single-flight

The why:

Only a few CTest harnesses existed, and [the event queue, hit testing, focus navigation, scroll math and snapshot swap concurrency were untested](source:s1).

The what (medium):

Five new plain-C CTest harnesses cover the event queue, the snapshot swap under concurrent hit testing, hit testing with focus navigation, scroll math, and Runtime Upload Single-Flight. Two small extractions from `src/ui/input.c` make focus navigation and scroll momentum testable. Two side fixes correct the `jimp.h` path in `AGENTS.md` and make `agent-check.sh full` prefer `docker compose`.

Look for:
- The brief asked for [roughly 900 added lines at most](source:s3); the diff adds about 1030, of which 78 in `src/ui/focus_nav.c` are moved, not new.
- The brief asked to document whether the full event queue drops oldest or newest; `event_queue_harness` asserts the current drop-newest behaviour, and whether that is intended stays an open question for the owner.

## Review concerns

### 01 Event queue harness (`event-queue`)

`event_queue_harness` drives the real `event_queue.c` for FIFO order across the ring boundary, the 256 cap, clear, shutdown, and one concurrent producer, and `CMakeLists.txt` registers it with the other new harnesses.

[groups/event-queue.md](groups/event-queue.md)

### 02 Back-tree fixture (`fixture`)

`tests/instance_fixture.h` creates, attaches, destroys and publishes back-tree instances the way `src/jslib/instance.c` does.

[groups/fixture.md](groups/fixture.md)

### 03 Snapshot swap stress harness (`snapshot-swap`)

The main thread mutates and swaps 20000 snapshots while a reader thread calls every front-snapshot accessor and checks the ids it copies out.

Depends on:
- 02 Back-tree fixture (`fixture`)

[groups/snapshot-swap.md](groups/snapshot-swap.md)

### 04 Focus navigation extraction and hit test harness (`focus-nav`)

`focus_nav_pick` in `src/ui/focus_nav.c` takes over the scoring loop from `find_nearest_focusable` in `input.c`, and `hit_test_harness` checks it together with hit order, scroll offsets and focusable clipping on a tree built through the instance-tree API.

Depends on:
- 02 Back-tree fixture (`fixture`)

[groups/focus-nav.md](groups/focus-nav.md)

### 05 Scroll math extraction and harness (`scroll-math`)

`scroll_max_offset` and `scroll_momentum_step` in `scroll.c` replace inline math in `input.c`, and `scroll_math_harness` checks them with `scroll_flow_step` and `scroll_content_height`.

[groups/scroll-math.md](groups/scroll-math.md)

### 06 Runtime Upload Single-Flight harness (`single-flight`)

The harness `#include`s `src/upload/http_server.c` to call the static `handle_upload` over socketpairs, holds one upload mid-body, and checks the second gets 409 without touching staging.

[groups/single-flight.md](groups/single-flight.md)

### 07 AGENTS.md path and docker compose fallback (`side-fixes`)

`AGENTS.md` points at `vendor/jimp/jimp.h`, and `run_vita` in `agent-check.sh` tries `docker compose` before `docker-compose`.

[groups/side-fixes.md](groups/side-fixes.md)