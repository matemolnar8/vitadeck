Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ece6f2c3bf4d9da1a964534b195c30231cc89a18` and `git rev-parse --verify 173943acf944d4c4ad2aa50f99a6a2933fcbac67` in this repository.
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

base (merge-base)  ece6f2c3bf4d9da1a964534b195c30231cc89a18

head               173943acf944d4c4ad2aa50f99a6a2933fcbac67

Named refs at pin: origin/main ... origin/t3/deck-app-input-harness

Read the diff:

git diff --find-renames ece6f2c3bf4d9da1a964534b195c30231cc89a18 173943acf944d4c4ad2aa50f99a6a2933fcbac67

Commits:
- 173943a Merge remote-tracking branch 'origin/main' into t3/deck-app-input-harness
- 411554a test: press every Pad Button in each example Deck App and switch Deck Apps

Sources:
- transcript Orchestrator brief · Oct 27 Add a committed functional regression check that presses every Pad Button in each example Deck App and switches Deck Apps, failing fast on a hang; run it in the Linux Docker smoke path.
- commit 444f2ce The bug this harness must catch.

The title:

Deck App input harness: press every Pad Button, switch Deck Apps

The why:

The minesweeper Triangle hang fixed in #135 slipped through every perf PR because nothing pressed Triangle in a Deck App or switched Deck Apps afterwards, so a hang blocked for an hour instead of failing fast [brief](source:s1).

The what (small):

A new smoke-labelled CTest harness presses every Pad Button on every section of each example Deck App and switches to the next, failing fast on a Deck App failure or a stuck JS thread; the macOS ctest steps now exclude by the `smoke` label.

Look for:
- The brief's ownership list is `tests/`, CMake, and scripts, but the change adds `event_queue_is_empty` under `src/core/`; no source asks for it, the harness needs it to tell a stuck handler from a slow one without stealing events [brief](source:s1).
- The brief suggests an `alarm()` watchdog for hangs; at head the Triangle hang is caught by the queue-drain wait in `press`, and the alarm only reports a `js_runtime_stop` or shutdown that never returns [brief](source:s1).

## Review concerns

### 01 event_queue_is_empty (`queue-probe`)

`event_queue_is_empty` reads `queue.count` under the queue mutex, next to `event_queue_pop`.

[groups/queue-probe.md](groups/queue-probe.md)

### 02 The harness and its CTest wiring (`harness`)

`run_deck_app_input_test.sh` stages the examples into a build-dir data root and runs `deck_app_input_harness`, which CMake builds from `HOST_RUNTIME_SOURCES` and registers with the `smoke` label and a 300 s TIMEOUT.

Depends on:
- 01 event_queue_is_empty (`queue-probe`)

[groups/harness.md](groups/harness.md)

### 03 Select smoke tests by label (`label-wiring`)

`smoke-docker.sh test` builds both smoke harnesses and runs `ctest -L smoke`, while CI's macOS step and `agent-check.sh host` switch from `-E smoke_harness` to `-LE smoke`.

Depends on:
- 02 The harness and its CTest wiring (`harness`)

[groups/label-wiring.md](groups/label-wiring.md)

### 04 Docs follow the label (`docs`)

The docs swap `smoke_harness` for the `smoke` label and describe the input harness; wording only.

Depends on:
- 03 Select smoke tests by label (`label-wiring`)

[groups/docs.md](groups/docs.md)