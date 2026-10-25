Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` and `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  06341f44bca0e9643f246ebca9f9f173697c5600

head               5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Named refs at pin: origin/refactor/native-util-module ... HEAD

Read the diff:

git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Review concern 02 of 04: Cached package list and one manifest parse (`package-cache`)

Part: Package list cache

The why:

The Shell called `package_library_list` twice per frame, each doing `opendir`, a JSON parse and asset `stat`s, and fonts and images re-parsed the manifest ([#54](source:s1)).

The what:

`package_library_list` copies from a list that publish, remove and `package_library_refresh` (called when the Shell opens) invalidate, and `bootstrap.c` hands one parsed manifest to `font_registry_load_package` and `image_registry_load_package`; the new harness checks each invalidation.

Look for:
- Subtle: set-active does not invalidate the cache; `is_active` is recomputed from `g_active_name` on every copy, so the flag is always current.
- Packages copied in outside the library (FTP, manual copy) appear only after the Shell is reopened, which calls `package_library_refresh`.
- The directory scan now runs under `g_mutex`, so a Shell frame that rescans can briefly block an upload publish, and the reverse.

Hunk refs for this concern:
- src/core/package_library.c
- src/core/package_library.h
- src/shell/shell.c
- src/core/bootstrap.c
- src/ui/fonts.c
- src/ui/fonts.h
- src/ui/images.c
- src/ui/images.h
- tests/package_library_harness.c
- CMakeLists.txt
- AGENTS.md