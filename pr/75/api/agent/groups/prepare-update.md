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

Review concern 04 of 04: prepareUpdate payload is used (`prepare-update`)

Part: Reconciler updates

The why:

`prepareUpdate` computed a diff that `commitUpdate` discarded, so every parent re-render re-sent all props natively ([#54](source:s1)).

The what:

`diffProps` sets `native` when a non-handler, non-children prop changed by value and `handlers` when a handler identity changed, and `commitUpdate` runs only the work those flags name.

Look for:
- Subtle: a parent passing `color={{ r: 1, g: 2, b: 3, a: 255 }}` inline used to trigger `nativeUpdateRect` every render; now two equal colors compare equal and no update is committed.
- A new `onPress` closure each render now calls only `updateHandlers`, not the native update.
- Native updates still send every prop: the `nativeUpdate*` functions in `src/jslib/instance.c` take full prop sets.

Hunk refs for this concern:
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts