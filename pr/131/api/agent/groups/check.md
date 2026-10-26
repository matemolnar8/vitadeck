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

Review concern 01 of 02: The shared display check (`check`)

The why:

The request asks for the check in one shared place, as a few lines of shell with no new dependencies [(request)](source:s1).

The what:

`check-macos-display.sh` exits 0 off Darwin; on macOS it fails when `CGDisplayIsAsleep` reports the main display asleep or `ioreg -n Root -d1` shows a lock key.

Look for:
- Subtle: when `osascript` fails or prints nothing, `asleep` is empty and the check passes, so a broken probe never blocks a run that might work.
- The lock pattern `CGSSessionScreenIsLocked"=Yes` has no leading quote, so it matches both `"CGSSessionScreenIsLocked"` and `"kCGSSessionScreenIsLocked"` in any `IOConsoleUsers` entry.

Hunk refs for this concern:
- scripts/check-macos-display.sh