Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 96ddad42a99c962958f50f86b75a371d80cb28b7` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 96ddad42a99c962958f50f86b75a371d80cb28b7 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               96ddad42a99c962958f50f86b75a371d80cb28b7

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 96ddad42a99c962958f50f86b75a371d80cb28b7

Review concern 01 of 03: Confirm on the active row restarts (`confirm`)

The why:

The issue asks that confirming the Active Deck App perform a Deck App Runtime Restart ([issue](source:s1)).

The what:

`shell_poll_system_input` drops the `!pkg->is_active` guard, so `activate_package` calls `package_library_set_active` on the already-active package and sets `request_runtime_restart`, which `main.c` already handles by stopping and restarting the JS runtime.

Look for:
- `package_library_set_active` on the active package rewrites the persisted active state with the same name each time; the harness case only proves that this succeeds and keeps `is_active`, not the Shell input path.
- The Shell message reads `Restarted <package>.` instead of `Activated <package>.` when the confirmed row was already active; it is visible the next time the Shell opens.

Hunk refs for this concern:
- src/shell/shell.c@186+187
- src/shell/shell.c@322+324
- tests/package_library_harness.c