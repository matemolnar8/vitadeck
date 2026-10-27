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

Review concern 01 of 04: The GUI lock and process-group helper (`helper`)

The why:

Every GUI launcher needs the same lock and the same way to stop what it started ([brief](source:s2)).

The what:

`gui_lock` takes the `mkdir` lock that `gui_cleanup` releases on exit, and `gui_start`/`gui_stop` run and kill a command as one process group, which `gui_run` combines for callers.

Look for:
- Race: stale reclaim is read-owner, check PID, `rm -rf`; two waiters reclaiming the same dead lock in the same instant can let the second one delete the lock the first just took.
- Subtle: a dead owner PID that the OS has reused for another process makes the lock look held; the waiting message prints the owner so a human can remove it.
- Subtle: `gui_lock` is a no-op when the owner PID is `$$` or the exported `VITADECK_GUI_LOCK_PID`, so `vita3k.sh launch` under `bench.sh` reuses the parent's lock and `gui_run` then runs Vita3K in the foreground inside the parent's process group.
- Subtle: `gui_start` turns on `set -m`, so the GUI gets its own process group and a terminal Ctrl-C reaches only the script; its `INT` trap exits, and the `EXIT` trap stops the group and removes the lock.
- `VITADECK_GUI_LOCK` overrides the lock path; it is documented only in the code and exists for tests.

Hunk refs for this concern:
- scripts/gui-run.sh