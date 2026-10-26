Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 989f1225eecb81c2eb269979162851e409cb484c` in this repository.
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

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               989f1225eecb81c2eb269979162851e409cb484c

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... 989f1225eecb81c2eb269979162851e409cb484c

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 989f1225eecb81c2eb269979162851e409cb484c

Commits:
- 989f122 fix(scripts): match the screen lock key with or without its k prefix
- c30cfe7 fix(scripts): fail fast when the macOS display is asleep or locked

Sources:
- transcript T3 Code session · Oct 26 Máté asks for a shared startup check in the macOS GUI scripts that exits with one clear line when the display is asleep or the screen is locked, and does nothing on other platforms.
- pr PR #131 Adds the check and proves it with PATH stubs fed captured ioreg output.
  https://github.com/matemolnar8/vitadeck/pull/131

The title:

Fail fast when the macOS display is asleep or locked

The why:

When the macOS display sleeps or the session locks, the host segfaults at window creation and Vita3K runs hang until a one-hour timeout, and unattended agent threads lose hours to that [according to the request](source:s1).

The what (small):

A new `scripts/check-macos-display.sh` exits non-zero with one line when the macOS display sleeps or the screen is locked, and the Vita3K, bench, and host `run` entry points call it before they build or open a window.

Look for:
- The request suggested `pmset -g powerstate IODisplayWranglerDevice` or `ioreg` for display power [(request)](source:s1). The display check calls `CGDisplayIsAsleep(CGMainDisplayID())` through `osascript -l JavaScript` instead. On macOS 26.7.1 that `pmset` query prints "Internal failure", Apple Silicon has no `IODisplayWrangler`, and GLFW makes the same `CGDisplayIsAsleep` call [(PR)](source:s2).
- The host CMake `run` target also calls the check; the request asked for `vita3k.sh` and `bench.sh` "at least" [(request)](source:s1).

## Review concerns

### 01 The shared display check (`check`)

`check-macos-display.sh` exits 0 off Darwin; on macOS it fails when `CGDisplayIsAsleep` reports the main display asleep or `ioreg -n Root -d1` shows a lock key.

[groups/check.md](groups/check.md)

### 02 Entry points call the check (`callers`)

`vita3k.sh run|launch`, `bench.sh` for every target except `vita`, and the host CMake `run` target call the check before they build or open a window.

Depends on:
- 01 The shared display check (`check`)

[groups/callers.md](groups/callers.md)