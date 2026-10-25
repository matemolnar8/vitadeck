Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b7df893f29a6ec038ac778734faa2ee18f88c214` and `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b7df893f29a6ec038ac778734faa2ee18f88c214 06341f44bca0e9643f246ebca9f9f173697c5600 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b7df893f29a6ec038ac778734faa2ee18f88c214

head               06341f44bca0e9643f246ebca9f9f173697c5600

Named refs at pin: b7df893f29a6ec038ac778734faa2ee18f88c214 ... 06341f44bca0e9643f246ebca9f9f173697c5600

Read the diff:

git diff --find-renames b7df893f29a6ec038ac778734faa2ee18f88c214 06341f44bca0e9643f246ebca9f9f173697c5600

Review concern 07 of 07: Dead code (`dead`)

The why:

The ticket lists these as [dead code to delete](source:s1).

The what:

`EVT_SHUTDOWN`, `js_runtime_restart`, `VdShell.bind_error`, and the FPS counter and touch circles in `main.c` go away with their only writers.

Look for:
- The host and Vita windows no longer show the FPS counter in the top right or red circles under touches; neither was behind a debug flag, and `smoke_harness` never drew them, so the golden is unaffected.

Hunk refs for this concern:
- src/core/event_queue.h
- src/core/js_runtime.h
- src/core/js_runtime.c@169+151
- src/main.c
- src/shell/shell.c@161+159
- src/shell/shell.c@176+172
- src/shell/shell.h