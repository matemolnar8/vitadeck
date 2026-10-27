Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify 082fca0b6123d1c025f533ae625e4b5a6a5273b9` in this repository.
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

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               082fca0b6123d1c025f533ae625e4b5a6a5273b9

Named refs at pin: 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 ... 082fca0b6123d1c025f533ae625e4b5a6a5273b9

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 082fca0b6123d1c025f533ae625e4b5a6a5273b9

Commits:
- 082fca0 fix(scripts): GUI runs share one machine-wide lock and bench kills hung runs

Sources:
- pr PR #139 One machine-wide GUI lock for bench, vita3k.sh and the CMake run target; bench kills the GUI process group on timeout.
  https://github.com/matemolnar8/vitadeck/pull/139
- transcript Claude session · Oct 27 Orchestrator brief: per-machine lock held only around the GUI run, mkdir-based, stale-PID recovery, release on signal; kill Vita3K on result timeout without timeout/gtimeout; update the bench and emulator skills.

The title:

GUI runs share one machine-wide lock and bench kills hung runs

The why:

Concurrent Vita3K benches from several worktrees shared `~/.vitadeck/vita3k` and contaminated each other's results, and a hung Deck App blocked `bench.sh vita3k` for up to an hour because the timeout did not apply to the emulator ([PR #139](source:s1)).

The what (small):

A new `scripts/gui-run.sh` serializes every VitaDeck window and Vita3K run on the machine behind one `mkdir` lock. `scripts/bench.sh` now runs the GUI in its own process group and kills it when no result arrives within `VITADECK_BENCH_TIMEOUT`.

## Review concerns

### 01 The GUI lock and process-group helper (`helper`)

`gui_lock` takes the `mkdir` lock that `gui_cleanup` releases on exit, and `gui_start`/`gui_stop` run and kill a command as one process group, which `gui_run` combines for callers.

[groups/helper.md](groups/helper.md)

### 02 bench.sh: lock after build, stop on timeout (`bench`)

`run_host` and `run_vita3k` take the lock after building and before writing the request, then `bench_gui` polls for `result.json` and calls `gui_stop` at the deadline or 10 s after the result.

Depends on:
- 01 The GUI lock and process-group helper (`helper`)

[groups/bench.md](groups/bench.md)

### 03 vita3k.sh and the CMake run target take the lock (`launchers`)

`vita3k.sh launch` takes the lock before writing the emulator config and runs Vita3K through `gui_run`, and the CMake host `run` target wraps `./vitadeck` in `scripts/gui-run.sh`.

Depends on:
- 01 The GUI lock and process-group helper (`helper`)

[groups/launchers.md](groups/launchers.md)

### 04 Skills mention the lock and the timeout (`docs`)

`vitadeck-bench` says runs wait for the lock and time out with the emulator killed, and `vitadeck-emulator` says launching waits for the same lock.

Depends on:
- 02 bench.sh: lock after build, stop on timeout (`bench`)
- 03 vita3k.sh and the CMake run target take the lock (`launchers`)

[groups/docs.md](groups/docs.md)