Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e` and `git rev-parse --verify 9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e 9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e

head               9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d

Named refs at pin: 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e ... t3/tech-debt-117

Read the diff:

git diff --find-renames 3616d7cdbc3d10cb1a6e56fc68ebd4a1cbddb98e 9cf3bfdd0d86573d2ee442f914c142a8bfc74f4d

Review concern 01 of 05: Shell input snapshot (`snapshot`)

The why:

A harness can only drive the Shell headless once its logic stops calling raylib; [the issue asks for a snapshot like `src/ui/input.c`](source:s1).

The what:

`shell_poll_system_input` samples the same seven raylib bindings into a `VdShellInput` and hands it to the new `shell_apply_system_input`, which holds the unchanged Shell logic with `input->x` in place of the old `x_pressed()` helpers.

Hunk refs for this concern:
- src/shell/shell.h
- src/shell/shell.c@45+45
- src/shell/shell.c@259+224
- src/shell/shell.c@279+256
- src/shell/shell.c@292+269
- src/shell/shell.c@304+281