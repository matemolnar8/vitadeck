Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 4d13a180782047fd4564b62704c0469b4e25bcd1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 4d13a180782047fd4564b62704c0469b4e25bcd1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               4d13a180782047fd4564b62704c0469b4e25bcd1

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... 4d13a180782047fd4564b62704c0469b4e25bcd1

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 4d13a180782047fd4564b62704c0469b4e25bcd1

Review concern 01 of 03: Locking inside the package library (`library`)

The why:

The active name and path are shared by the upload, main, and JS threads ([issue #42](source:s1)).

The what:

`g_mutex` guards `g_active_name`/`g_active_path`, and every public `package_library_*` function reads or writes them only under it, using the new unlocked helpers `read_package_info` and `set_active_locked`.

Look for:
- Subtle: `package_library_publish_package` now checks for a missing Active Deck App after the renames, under the lock, not before validating the upload. A Shell set-active that lands during an upload's manifest read now wins over the upload.
- Perf: publish holds `g_mutex` across two `rename` calls and the `active-package.txt` write, so the per-frame `package_library_has_active_deck_app` on the main thread waits for them. Deleting the `.replacing` backup and reading manifests stay outside the lock.
- `package_library_set_active` and `package_library_remove` hold the lock across the manifest read and `remove_tree`. Both run on the main thread, so only the upload and JS threads can wait on them.
- `package_library_remove` now runs `safe_package_name` before comparing with the active name, so a NULL name returns an error instead of crashing in `strcmp`.
- The roots stay unlocked `const char *`. They are written only in `package_library_init`, before the JS and upload threads start. `package_library_root()` had no callers and is removed.

Hunk refs for this concern:
- src/core/package_library.c
- src/core/package_library.h