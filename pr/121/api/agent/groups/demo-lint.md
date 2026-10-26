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

Review concern 03 of 03: Demo app effects and the failure golden (`demo-lint`)

The why:

After merging `main`, oxlint 1.89's `exhaustive-effect-dependencies` rule flags two demo effects that used state only to re-run.

The what:

`TimersSection`'s hide timer reads the `toast` it hides, `FetchSection`'s retry calls `requestRepo()` from the press handler, and the failure-screen golden picks up the stack locations those bundles now produce.

Look for:
- Subtle: a double press on "Fetch again" runs one request, because the handler returns while `request.status` is `loading`; before, each press bumped `attempt` and the effect cleanup dropped the older response.
- A retry that resolves after the section unmounts calls `setRequest` on an unmounted component, which React 18 ignores; only the initial load keeps the `ignore` guard.
- `13-failure-screen.png` changes only in the stack trace text: `runtime.js:1:16040` becomes `runtime.js:24:9124` because rolldown 1.2 keeps license comments, and the demo's `app.js` line moves with the `FetchSection` edit.

Depends on:
- 02 Catalog bumps and the new lint findings (`deps`)

Hunk refs for this concern:
- js/examples/demo/src/sections/FetchSection.tsx@30+30
- js/examples/demo/src/sections/FetchSection.tsx@84+87
- js/examples/demo/src/sections/TimersSection.tsx
- tests/fixtures/golden/Linux/13-failure-screen.png