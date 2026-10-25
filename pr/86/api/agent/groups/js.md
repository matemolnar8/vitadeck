Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` and `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

head               2d9850830f3b8f421e81f7d6750564e89db51ccc

Named refs at pin: origin/feat/focus-and-pad-input ... feat/text-input

Read the diff:

git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc

Review concern 05 of 07: SDK TextInput and runtime wiring (`js`)

The why:

Authors get the component from the [proposal](source:s1); the runtime maps it to the native calls.

The what:

`TextInput` renders `vita-text-input`, whose props the reconciler passes to `nativeCreateTextInput`/`nativeUpdateTextInput` and whose `onChange` handler `onTextInputFromNative` calls.

Look for:
- `autoFocus` now also schedules `commitMount` for `vita-text-input`; defaults are `RAYWHITE` fill, `BLACK` text, font size 20 and `maxLength` 256.

Depends on:
- 03 vita-text-input instance and rendering (`instance`)
- 04 Editing session in the input loop (`session`)

Hunk refs for this concern:
- js/packages/sdk/src/vitadeck-host-types.ts
- js/packages/sdk/src/index.tsx
- js/packages/runtime/src/input.ts
- js/packages/runtime/src/main.tsx
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts