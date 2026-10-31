Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify fbb6af02ce01caf22a295e56ad95c7aacba7f803` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f fbb6af02ce01caf22a295e56ad95c7aacba7f803 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               fbb6af02ce01caf22a295e56ad95c7aacba7f803

Named refs at pin: a447e3b9 ... fbb6af02

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f fbb6af02ce01caf22a295e56ad95c7aacba7f803

Review concern 07 of 09: Shell listener toggle and Host Control screen (`shell`)

The why:

The user reads the Vita address, turns the listener off when wanted, and forgets links on the Vita ([#111](source:s3), [session](source:s5)).

The what:

The Upload row becomes `Upload and link`, showing the LAN Listener URL and toggling it, and the Host Control row opens `VD_SHELL_HOST_CONTROL`, which shows the `link` command with the listener address and the linked computers without touching the listener.

Look for:
- With no Deck App installed, `shell_show_home` focuses the Host Control row through `home_first_row()`, so the first Cross on a fresh Vita opens Host Control instead of stopping the listener.
- Focus is a row index and uploads add package rows above the entries; `keep_entry_focus()` in `shell_update` shifts focus by the packages added while it sits on an entry, so an install does not slide it from Host Control onto Upload and link.
- Messages from the listener (an install, a link, a rejected link) land in the Shell message even while the Shell is hidden, and show when it opens.

Depends on:
- 06 LAN Listener, link store and native getter (`native-link`)

Hunk refs for this concern:
- src/shell/shell.c
- src/shell/shell.h
- tests/shell_input_harness.c