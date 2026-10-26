Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 613ea51e22a4e3ffb93b28da7f52c16830b91e83` and `git rev-parse --verify e2a3b22bb6662ae3fc7556a750655d76a2dda7f9` in this repository.
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

base (merge-base)  613ea51e22a4e3ffb93b28da7f52c16830b91e83

head               e2a3b22bb6662ae3fc7556a750655d76a2dda7f9

Named refs at pin: 613ea51e22a4e3ffb93b28da7f52c16830b91e83 ... HEAD

Read the diff:

git diff --find-renames 613ea51e22a4e3ffb93b28da7f52c16830b91e83 e2a3b22bb6662ae3fc7556a750655d76a2dda7f9

Commits:
- e2a3b22 Merge remote-tracking branch 'origin/main' into t3/perf-input-backlog-124
- ba1d804 chore(runtime): address review on the input budget
- 3eac5f5 perf(runtime): bound input handling per JS loop iteration

Sources:
- ticket #124 Bound the work per JS loop iteration so a slow-handler backlog still publishes snapshots.
  https://github.com/matemolnar8/vitadeck/issues/124

The title:

perf(runtime): bound input handling per JS loop iteration

The why:

When input arrives faster than its handlers finish, the JS loop never leaves the input `while`, so no snapshot is published and [the screen stops updating until the backlog clears](source:s1).

The what (small):

`process_input_events` stops after about 16 ms of handlers so the JS loop reaches timers, microtasks and the snapshot swap during an input backlog, and a new CTest harness checks the bound.

Look for:
- The ticket's check is [swaps well above 0 in `minesweeper-navigate`](source:s1), which this change cannot show: that scenario's commits make 0 native mutations (focus is native), and on main #125 removed its renders entirely, so `instance_tree_swap` has nothing to publish. The PR proves swaps with a throwaway, uncommitted `minesweeper-flag` scenario instead.
- The ticket expects [input latency to stop growing over the window](source:s1). It does not: the budget reorders the same work, so a backlog of slower-than-input handlers still grows; only the screen keeps updating.
- The ticket names the Vita baseline; the before/after was measured in Vita3K, and no baseline is updated because no committed scenario moves.

## Review concerns

### 01 Input time budget (`budget`)

`process_input_events` stamps a deadline 16 ms out before the first pop and breaks after any event that ends past it, so one event always runs.

[groups/budget.md](groups/budget.md)

### 02 input_budget_harness (`harness`)

`tests/input_budget_harness.c` stubs `vitadeck.input.onPadButtonFromNative` with a busy-wait and calls `process_input_events` directly, and `CMakeLists.txt` builds it from `HOST_RUNTIME_SOURCES` as a CTest.

Depends on:
- 01 Input time budget (`budget`)

[groups/harness.md](groups/harness.md)