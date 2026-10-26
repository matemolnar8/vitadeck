Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 989f1225eecb81c2eb269979162851e409cb484c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 989f1225eecb81c2eb269979162851e409cb484c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               989f1225eecb81c2eb269979162851e409cb484c

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... 989f1225eecb81c2eb269979162851e409cb484c

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 989f1225eecb81c2eb269979162851e409cb484c

Review concern 02 of 02: Entry points call the check (`callers`)

The why:

The scripts that open a macOS window must run the check at startup [(request)](source:s1).

The what:

`vita3k.sh run|launch`, `bench.sh` for every target except `vita`, and the host CMake `run` target call the check before they build or open a window.

Look for:
- `bench.sh vita3k` runs the check twice: once itself, which prints to the terminal, and again inside `vita3k.sh launch`, which prints into `log.txt`.
- `bench.sh host` checks only before the build, so a display that sleeps during a long build still reaches the GLFW crash.

Depends on:
- 01 The shared display check (`check`)

Hunk refs for this concern:
- scripts/vita3k.sh
- scripts/bench.sh
- CMakeLists.txt