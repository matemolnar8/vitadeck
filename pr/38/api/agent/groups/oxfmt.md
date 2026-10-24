Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify e03f214f037ea36a7095a5c1e5722284ee6df983` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 e03f214f037ea36a7095a5c1e5722284ee6df983 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               e03f214f037ea36a7095a5c1e5722284ee6df983

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... e03f214

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 e03f214f037ea36a7095a5c1e5722284ee6df983

Review concern 07 of 08: oxfmt line wrapping (`oxfmt`)

Part: oxfmt pass

The why:

`pnpm --dir js format:check` failed on `main`; the [PR](source:s1) runs the formatter in its own commit.

The what:

Every hunk rejoins parameter lists, call arguments, JSX props, or a union type onto one line under oxfmt's print width, and `js/packages/sdk/README.md` loses a trailing blank line.

Hunk refs for this concern:
- js/examples/chat/src/genesys/copilot.ts
- js/examples/timers/src/App.tsx
- js/packages/runtime/src/input.ts
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts
- js/packages/sdk/README.md
- js/packages/sdk/src/vitadeck-host-types.ts