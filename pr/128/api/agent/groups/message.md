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

Review concern 02 of 05: Shell message cleared on hide (`message`)

The why:

[Item 3 of the issue](source:s1): "Restarted X." reappeared on the next Shell open.

The what:

`hide_shell_to_deck` clears `shell->message`, so `activate_package` no longer writes the success message it would immediately wipe.

Look for:
- Behavior: "Activated X." and "Restarted X." are gone entirely; a failed `package_library_set_active` still writes its error and keeps the Shell open, and an upload result that arrives while the Shell is hidden still shows on the next open, because only hiding clears the message.

Hunk refs for this concern:
- src/shell/shell.c@184+148
- src/shell/shell.c@191+156