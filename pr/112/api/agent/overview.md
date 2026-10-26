Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 96ddad42a99c962958f50f86b75a371d80cb28b7` in this repository.
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

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               96ddad42a99c962958f50f86b75a371d80cb28b7

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 96ddad42a99c962958f50f86b75a371d80cb28b7

Commits:
- 96ddad4 feat(shell): confirming the Active Deck App restarts it

Sources:
- ticket #108 Make confirming the Active Deck App in the Shell perform a Deck App Runtime Restart; update the Shell glossary entries.
  https://github.com/matemolnar8/vitadeck/issues/108

The title:

Confirming the Active Deck App restarts it

The why:

After the Deck App Failure Screen, the only way to retry was to activate another Deck App and switch back, and a single-app device had no way out ([issue](source:s1)).

The what (small):

Confirming the active row on the Shell Home Screen now goes through the same activate path as any other row, which requests a Deck App Runtime Restart and hides the Shell. The footer hint and the glossary say so.

## Review concerns

### 01 Confirm on the active row restarts (`confirm`)

`shell_poll_system_input` drops the `!pkg->is_active` guard, so `activate_package` calls `package_library_set_active` on the already-active package and sets `request_runtime_restart`, which `main.c` already handles by stopping and restarting the JS runtime.

[groups/confirm.md](groups/confirm.md)

### 02 Footer hint names the restart (`hint`)

`shell_render` sets `restart_focused` while the active row's first cell has focus, and `shell_draw_footer_hint` then reads `Confirm: restart`.

Depends on:
- 01 Confirm on the active row restarts (`confirm`)

[groups/hint.md](groups/hint.md)

### 03 Glossary lines (`glossary`)

`Deck App Runtime Restart` gains the Shell confirm as a trigger, and `Shell Home Screen` says what confirming a package and the Active Deck App does.

[groups/glossary.md](groups/glossary.md)