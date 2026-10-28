Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify eb61ca83421e50b68494e84294baac13b7c346d1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               eb61ca83421e50b68494e84294baac13b7c346d1

Named refs at pin: a447e3b9 ... HEAD

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1

Review concern 06 of 08: Shell Host Control entry and screen (`shell`)

The why:

The user reads the link address and forgets links on the Vita ([#111](source:s3)).

The what:

A Host Control row after Upload opens `VD_SHELL_HOST_CONTROL`, which starts the listener, shows the address, status and linked computers, and stops the listener on a link, Circle, Start or shutdown.

Look for:
- After a successful link the listener has stopped, so the screen swaps the `host-companion link` hint for "go Back and open Host Control again"; linking a second computer needs that round trip.

Depends on:
- 05 Link listener, store and native getter (`native-link`)

Hunk refs for this concern:
- src/shell/shell.c
- src/shell/shell.h
- tests/shell_input_harness.c