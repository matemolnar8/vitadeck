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

Review concern 02 of 03: Path-free natives and entry loading (`natives`)

The why:

The design replaces `nativeReadTextFile`, `nativeEvalFile` and `nativeGetActiveDeckAppPath` with natives that take no path ([#49](source:s1)).

The what:

`src/jslib/storage.c` wraps the native store as `nativeReadDeckAppStorage`/`nativeWriteDeckAppStorage`, and `nativeEvalActiveDeckAppEntry` in `jslib.c` lets `main.tsx` drop its JS manifest read.

Look for:
- `register_js_storage` captures the Active Deck App's package name when the JS thread starts, so a write that lands after a Shell switch but before the restart still goes to the old package.
- A setItem past the cap throws `RangeError`; an I/O failure throws `InternalError`, so authors can tell them apart.
- The runtime no longer checks `schemaVersion`, `name` or `version` in JS; it relies on `package_manifest_read` in bootstrap, and a package that never registers now fails with "Deck App package did not register a component." without the package name.

Depends on:
- 01 Native store and its harness (`native-store`)

Hunk refs for this concern:
- src/jslib/storage.c
- src/jslib/jslib_internal.h
- src/jslib/jslib.c
- src/core/package_manifest.h
- src/core/package_manifest.c
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/main.tsx