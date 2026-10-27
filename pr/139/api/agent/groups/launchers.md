Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify 082fca0b6123d1c025f533ae625e4b5a6a5273b9` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 082fca0b6123d1c025f533ae625e4b5a6a5273b9 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               082fca0b6123d1c025f533ae625e4b5a6a5273b9

Named refs at pin: 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 ... 082fca0b6123d1c025f533ae625e4b5a6a5273b9

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 082fca0b6123d1c025f533ae625e4b5a6a5273b9

Review concern 03 of 04: vita3k.sh and the CMake run target take the lock (`launchers`)

The why:

Interactive Vita3K and host runs also skew timings and share the emulator home ([brief](source:s2)).

The what:

`vita3k.sh launch` takes the lock before writing the emulator config and runs Vita3K through `gui_run`, and the CMake host `run` target wraps `./vitadeck` in `scripts/gui-run.sh`.

Depends on:
- 01 The GUI lock and process-group helper (`helper`)

Hunk refs for this concern:
- scripts/vita3k.sh
- CMakeLists.txt