Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify d1a60387997054feecfb96ad2f518d0f9cee05f0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               d1a60387997054feecfb96ad2f518d0f9cee05f0

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/deck-app-error-screen

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0

Review concern 03 of 04: Runtime error boundary (`boundary`)

The why:

`main.tsx` had [no error boundary](source:s1), so a render or effect error after boot unmounted the root.

The what:

`DeckAppErrorBoundary` wraps `ThemeProvider` and the Deck App, renders `null` after an error, and reports it through the new `nativeReportDeckAppError` global.

Look for:
- The reconciler callback is renamed to `onRecoverableError` because React passes it only errors it already recovered from; behavior is unchanged, it still logs.
- The boundary sends `error.stack` and falls back to `info.componentStack` only when the error has no stack.

Depends on:
- 02 Native exception reporters (`reporters`)

Hunk refs for this concern:
- js/packages/runtime/src/main.tsx
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts