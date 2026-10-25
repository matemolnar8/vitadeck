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

Review concern 03 of 03: SDK `storage` and its docs (`sdk`)

The why:

The approved surface is a `storage` export from `@vitadeck/sdk` that keeps the map in memory and writes the whole JSON on every change ([#49](source:s1)).

The what:

`storage.ts` loads the map once from `nativeReadDeckAppStorage` and persists a copy on each change, `index.tsx` exports it, minesweeper uses it for a win count, and the README and `CONTEXT.md` describe it.

Look for:
- A stored file that is not valid JSON is logged with `console.warn` and loads as empty; the next `setItem` overwrites it, so the unreadable contents are lost.
- `setItem` copies the whole `Map` and stringifies the whole store on every call; the in-memory map changes only after the native write returns, so a rejected write leaves `getItem` unchanged.

Depends on:
- 02 Path-free natives and entry loading (`natives`)

Hunk refs for this concern:
- js/packages/sdk/src/storage.ts
- js/packages/sdk/src/index.tsx
- js/examples/minesweeper/src/App.tsx
- js/packages/sdk/README.md
- CONTEXT.md