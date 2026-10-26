Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5` and `git rev-parse --verify cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5 cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5

head               cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c

Named refs at pin: origin/main ... chore/upgrade-pnpm-js-deps

Read the diff:

git diff --find-renames 5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5 cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c

Review concern 02 of 03: Catalog bumps and the new lint findings (`deps`)

The why:

The requests ask for in-range bumps [per the request](source:t1) and the latest oxlint, oxfmt and Node types [per the follow-up](source:t2); oxlint 1.89 with tsgolint 7 reports three new errors.

The what:

The catalog bumps oxlint, oxlint-tsgolint, oxfmt, `@types/node`, `@types/react`, rolldown and tsx, and the three files oxlint now flags drop a redundant cast or move a `setState` out of an effect.

Look for:
- In the timers example, the Start and Cancel handlers now set the status text; Cancel only shows "Cancelled" while a timeout is pending, as the effect did before.
- `rolldown` goes from `1.0.0-rc.18` to `1.2.15`; it is a runtime dependency of `@vitadeck/sdk`, so authors' `vitadeck build` bundles with it too.

Hunk refs for this concern:
- js/pnpm-workspace.yaml
- js/packages/sdk/cli/templates/scaffold/package.json
- js/examples/timers/src/App.tsx@34+34
- js/examples/timers/src/App.tsx@43+42
- js/examples/timers/src/App.tsx@83+81
- js/examples/timers/src/App.tsx@94+93
- js/packages/runtime/src/main.tsx
- js/packages/sdk/src/host-elements.tsx