Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` and `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2d9850830f3b8f421e81f7d6750564e89db51ccc 8944d5cdb7fc7fc450617cca3781bb2cf1808471 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2d9850830f3b8f421e81f7d6750564e89db51ccc

head               8944d5cdb7fc7fc450617cca3781bb2cf1808471

Named refs at pin: origin/feat/text-input ... feat/stack-layout

Read the diff:

git diff --find-renames 2d9850830f3b8f421e81f7d6750564e89db51ccc 8944d5cdb7fc7fc450617cca3781bb2cf1808471

Review concern 04 of 07: SDK Stack component and reconciler case (`sdk-runtime`)

The why:

Authors reach Stack only through `@vitadeck/sdk`; the change is additive.

The what:

`Stack` and `VitaStackProps` join the SDK, a shared `Position` type makes `x`/`y` optional, and the reconciler maps `vita-stack` props to native arguments while defaulting every `x`/`y` to 0.

Depends on:
- 03 Native create and update for Stack (`bindings`)

Hunk refs for this concern:
- js/packages/sdk/src/vitadeck-host-types.ts
- js/packages/sdk/src/index.tsx
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts