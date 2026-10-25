Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` and `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

head               2d9850830f3b8f421e81f7d6750564e89db51ccc

Named refs at pin: origin/feat/focus-and-pad-input ... feat/text-input

Read the diff:

git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc

Review concern 01 of 07: Platform text input: IME dialog and host keyboard (`platform`)

The why:

The Vita IME and raylib keyboard code stays behind `src/platform/`, as AGENTS.md asks for platform quirks; the [proposal](source:s1) names this split.

The what:

`text_input.h` defines one begin/poll/cancel session that `text_input_vita.c` implements with `sceImeDialog` and `text_input_host.c` with raylib keys, and CMake picks one per target.

Look for:
- Subtle: the host toggles `SetExitKey(KEY_NULL)` while editing, so Escape cancels the edit instead of quitting; outside an edit Escape still closes the window as before.
- Subtle: Vita cancel calls `sceImeDialogAbort` and the dialog terminates on a later `vd_text_input_poll`; a `begin` before that returns false, so a Press during the abort starts no edit.
- `vd_text_input_apply_keys` is declared for both targets but only defined on the host; nothing on the Vita calls it.

Hunk refs for this concern:
- src/platform/text_input.h
- src/platform/text_input_host.c
- src/platform/text_input_vita.c
- CMakeLists.txt