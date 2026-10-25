Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify a146e94023af512bbe5270801a47f0947fdcaf4d` in this repository.
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

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               a146e94023af512bbe5270801a47f0947fdcaf4d

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... a146e94023af512bbe5270801a47f0947fdcaf4d

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b a146e94023af512bbe5270801a47f0947fdcaf4d

Commits:
- a146e94 test: storage harness for round trip, cap, crash recovery and removal
- 02f7a05 docs: Deck App Storage glossary entry and SDK README section
- 291c09a feat(sdk): storage API for Deck Apps
- 31f691a feat(core): per-package Deck App Storage with path-free natives

Sources:
- ticket #49 Approved design: synchronous string `storage` in the SDK, one JSON file per package under deck-app-data, temp-and-rename writes, 1 MiB cap, path-free natives, removal deletes storage, replacement keeps it.
  https://github.com/matemolnar8/vitadeck/issues/49
- pr PR #84 Implements #49 as approved; lists the agent-check tiers that passed in Docker.
  https://github.com/matemolnar8/vitadeck/pull/84

The title:

feat(sdk): per-package storage API for Deck Apps

The why:

Deck Apps need data that survives restarts, and no global a Deck App reaches should accept a filesystem path ([#49](source:s1)).

The what (medium):

Adds Deck App Storage: a native per-package JSON store with atomic replace and a 1 MiB cap, two path-free natives, and a `storage` export in `@vitadeck/sdk`. It also replaces the path-taking file natives with `nativeEvalActiveDeckAppEntry()`.

## Review concerns

### 01 Native store and its harness (`native-store`)

`deck_app_storage.c` reads, writes and removes `<data root>/deck-app-data/<package>/storage.json`, `package_library_remove` calls its removal, and `storage_harness` drives both through crash states and the library.

[groups/native-store.md](groups/native-store.md)

### 02 Path-free natives and entry loading (`natives`)

`src/jslib/storage.c` wraps the native store as `nativeReadDeckAppStorage`/`nativeWriteDeckAppStorage`, and `nativeEvalActiveDeckAppEntry` in `jslib.c` lets `main.tsx` drop its JS manifest read.

Depends on:
- 01 Native store and its harness (`native-store`)

[groups/natives.md](groups/natives.md)

### 03 SDK `storage` and its docs (`sdk`)

`storage.ts` loads the map once from `nativeReadDeckAppStorage` and persists a copy on each change, `index.tsx` exports it, minesweeper uses it for a win count, and the README and `CONTEXT.md` describe it.

Depends on:
- 02 Path-free natives and entry loading (`natives`)

[groups/sdk.md](groups/sdk.md)