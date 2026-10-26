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

Review concern 03 of 05: Upload server mutex freed on shutdown (`mutex`)

The why:

The new harness runs under LeakSanitizer in the CI smoke job, which flagged the mutex `upload_server_init` allocates and nothing freed.

The what:

`shell_shutdown` destroys the upload server mutex after `upload_server_stop` has joined the listener and handler threads.

Look for:
- Production change from a test PR: `main.c` calls `shell_shutdown` once at exit, so the app now frees the mutex there too; a second call is harmless because `upload_server_stop` early-returns and `vd_mutex_destroy(NULL)` does nothing.

Hunk refs for this concern:
- src/shell/shell.c@235+198