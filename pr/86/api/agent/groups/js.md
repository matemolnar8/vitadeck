Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 6ec5488d92569d22ca087ea754dd8d016282272b` and `git rev-parse --verify d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  6ec5488d92569d22ca087ea754dd8d016282272b

head               d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

Named refs at pin: 6ec5488 ... d3d67e5

Read the diff:

git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

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