Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify 49e3d7cc896384082f20e925674c8237418bfa6f` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 49e3d7cc896384082f20e925674c8237418bfa6f -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               49e3d7cc896384082f20e925674c8237418bfa6f

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... chore/upgrade-pnpm-js-deps

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 49e3d7cc896384082f20e925674c8237418bfa6f

Review concern 02 of 03: Babel 8 through a local rolldown plugin (`babel`)

The why:

The follow-up asks for Babel 8 if it can be made to work [per the follow-up](source:t2); `@rollup/plugin-babel` 7.1.0 peers `@babel/core ^7`, so every npm or pnpm install of the SDK warned.

The what:

`babel()` in `sdk/cli/babel.ts` runs `transformAsync` with the presets pinned to Babel 7 output, and both the CLI's `vitadeck build` and the runtime's `rolldown.config.js` call it instead of `@rollup/plugin-babel`.

Look for:
- Subtle: Babel 8 always keeps uninitialized class fields, so `Response` in `runtime.js` now declares `status; ok; statusText; headers;` before its constructor assigns them; an author subclass that redeclares a base-class field now resets it to `undefined`.
- `runtime: "classic"`, `development: false` and `onlyRemoveTypeImports: false` restore Babel 7 defaults; without them app bundles import `react/jsx-runtime` and keep type-only imports, and the example `.vdapp` bundles are byte-identical with them.
- Preset names still resolve from the build's working directory, as they did through `@rollup/plugin-babel`, so `test-project.ts` keeps linking the presets into the fake author project.

Depends on:
- 01 pnpm 12 and Node 24 (`toolchain`)

Hunk refs for this concern:
- js/packages/sdk/cli/babel.ts
- js/packages/sdk/cli/cli.ts@4+4
- js/packages/sdk/cli/cli.ts@416+416
- js/packages/runtime/rolldown.config.js@1+1
- js/packages/runtime/rolldown.config.js@26+26
- js/packages/runtime/package.json
- js/packages/sdk/package.json
- js/packages/sdk/rolldown.cli.config.mjs
- js/pnpm-workspace.yaml
- js/package.json