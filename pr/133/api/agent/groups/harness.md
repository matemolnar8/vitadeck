Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 613ea51e22a4e3ffb93b28da7f52c16830b91e83` and `git rev-parse --verify e2a3b22bb6662ae3fc7556a750655d76a2dda7f9` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 613ea51e22a4e3ffb93b28da7f52c16830b91e83 e2a3b22bb6662ae3fc7556a750655d76a2dda7f9 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 02: input_budget_harness (`harness`)

The why:

It pins the bound without a Vita: slow handlers must leave events queued, cheap ones must still drain in one call.

The what:

`tests/input_budget_harness.c` stubs `vitadeck.input.onPadButtonFromNative` with a busy-wait and calls `process_input_events` directly, and `CMakeLists.txt` builds it from `HOST_RUNTIME_SOURCES` as a CTest.

Look for:
- The cheap-handler case asserts 8 trivial calls finish inside 16 ms of wall-clock time; a long preemption on a loaded CI runner would fail it.

Depends on:
- 01 Input time budget (`budget`)

Hunk refs for this concern:
- tests/input_budget_harness.c
- CMakeLists.txt