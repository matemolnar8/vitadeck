Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3` and `git rev-parse --verify 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  27683e0ceec2ca0ab7c185c2e97d4f79787f16f3

head               584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Named refs at pin: 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 ... 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Read the diff:

git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Review concern 02 of 05: Text defaults (`text-defaults`)

Part: Text defaults

The why:

Black text is invisible on the default OLED theme and the 8px offset and 30px size are out of line with the other elements ([#102](source:s1)).

The what:

The SDK `Text` component fills `color` from `useTheme()`, while the reconciler and the native fallback in `fonts.h` move the size to 20 and the position to 0.

Look for:
- The colour default lives in the SDK component; a raw `vita-text` host element without `color` still draws black from the native fallback in `render.c`.
- `fontSize={0}` now falls back to 20 natively instead of 30, matching the JS default.

Hunk refs for this concern:
- js/packages/sdk/src/index.tsx@47+47
- js/packages/sdk/src/vitadeck-host-types.ts@9+9
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@92+92
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@180+180
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@315+316
- src/ui/fonts.h
- js/packages/sdk/README.md