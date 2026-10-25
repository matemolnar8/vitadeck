Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Review concern 05 of 07: Runtime: dispatch and autoFocus (`runtime`)

The why:

Connects native events to the SDK: focus and blur to Button handlers, Pad Buttons to `usePadButton` subscribers.

The what:

`input.ts` maps the renamed native events to `onFocus`/`onBlur`/press handlers and forwards Pad Buttons to `dispatchPadButton`, while the reconciler passes `focusColor` natively, skips `autoFocus` in `diffProps`, and requests Focus from `commitMount`.

Look for:
- `finalizeInitialChildren` used to return true for every instance; now only autoFocus Buttons get a `commitMount`, which was a no-op for everything else.

Depends on:
- 04 SDK: FocusProps and usePadButton (`sdk`)

Hunk refs for this concern:
- js/packages/runtime/src/input.ts
- js/packages/runtime/src/main.tsx
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts