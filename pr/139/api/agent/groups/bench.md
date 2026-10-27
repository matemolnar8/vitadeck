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

Review concern 02 of 04: bench.sh: lock after build, stop on timeout (`bench`)

The why:

Bench runs collided in the shared emulator home, and a hung Deck App never let Vita3K quit ([PR #139](source:s1)).

The what:

`run_host` and `run_vita3k` take the lock after building and before writing the request, then `bench_gui` polls for `result.json` and calls `gui_stop` at the deadline or 10 s after the result.

Look for:
- Behavior change: the host target used to fail with `vitadeck failed` on a nonzero exit; now any exit that leaves `result.json` counts as success, as `vita3k` already did.
- Subtle: the lock is held until `bench.sh` exits, so the `pnpm bench:report` step also runs under it, a second or two.
- `run_host` now does `cd "$ROOT_DIR/out"` in the script's own shell instead of a subshell, so later steps run from `out/`; they all use absolute paths.

Depends on:
- 01 The GUI lock and process-group helper (`helper`)

Hunk refs for this concern:
- scripts/bench.sh