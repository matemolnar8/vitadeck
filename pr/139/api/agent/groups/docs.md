Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify 21f4d669c55fc00053f3ab785a59c45ec9ea9cb5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 21f4d669c55fc00053f3ab785a59c45ec9ea9cb5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               21f4d669c55fc00053f3ab785a59c45ec9ea9cb5

Named refs at pin: 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 ... origin/t3/gui-run-lock

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 21f4d669c55fc00053f3ab785a59c45ec9ea9cb5

Review concern 04 of 04: Skills mention the lock and the timeout (`docs`)

The why:

The bench and emulator skills would otherwise not say that runs wait or that a hung run is killed ([brief](source:s2)).

The what:

`vitadeck-bench` says runs wait for the lock and time out with the emulator killed, and `vitadeck-emulator` says launching waits for the same lock.

Depends on:
- 02 bench.sh: lock after build, stop on timeout (`bench`)
- 03 vita3k.sh and the CMake run target take the lock (`launchers`)

Hunk refs for this concern:
- .agents/skills/vitadeck-bench/SKILL.md
- .agents/skills/vitadeck-emulator/SKILL.md