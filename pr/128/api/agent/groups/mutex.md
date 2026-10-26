Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               33b2ca703b09bf38890ad2ab3d952fdc75edd9b5

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... t3/tech-debt-117

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5

Review concern 03 of 05: Upload server mutex freed on shutdown (`mutex`)

The why:

The new harness runs under LeakSanitizer in the CI smoke job, which flagged the mutex `upload_server_init` allocates and nothing freed.

The what:

`shell_shutdown` destroys the upload server mutex after `upload_server_stop` has joined the listener and handler threads.

Look for:
- Production change from a test PR: `main.c` calls `shell_shutdown` once at exit, so the app now frees the mutex there too; a second call is harmless because `upload_server_stop` early-returns and `vd_mutex_destroy(NULL)` does nothing.

Hunk refs for this concern:
- src/shell/shell.c@235+198