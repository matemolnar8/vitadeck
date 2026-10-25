Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify a146e94023af512bbe5270801a47f0947fdcaf4d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b a146e94023af512bbe5270801a47f0947fdcaf4d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               a146e94023af512bbe5270801a47f0947fdcaf4d

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... a146e94023af512bbe5270801a47f0947fdcaf4d

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b a146e94023af512bbe5270801a47f0947fdcaf4d

Review concern 01 of 03: Native store and its harness (`native-store`)

The why:

The design puts one JSON file per package under `deck-app-data` and asks for writes that a crash cannot truncate ([#49](source:s1)).

The what:

`deck_app_storage.c` reads, writes and removes `<data root>/deck-app-data/<package>/storage.json`, `package_library_remove` calls its removal, and `storage_harness` drives both through crash states and the library.

Look for:
- Subtle: every write moves `storage.json` to `storage.json.bak` before renaming the temp file in, even on POSIX, because the Vita's rename does not replace; a crash between the two renames leaves only the backup, and `deck_app_storage_read` returns the previous value from it.
- When the package directory is removed but its storage directory is not, `package_library_remove` returns false with "Could not remove Deck App Storage.", so the Shell shows an error for a package that is already gone from the list.

Hunk refs for this concern:
- src/core/deck_app_storage.h
- src/core/deck_app_storage.c
- src/core/package_library.h
- src/core/package_library.c
- CMakeLists.txt
- tests/storage_harness.c