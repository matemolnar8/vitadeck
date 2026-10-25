Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 12bf1a46251dc1b0ba311c85fd44f69530f780f2` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               12bf1a46251dc1b0ba311c85fd44f69530f780f2

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... chore/ci-tests-and-build-mode

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 12bf1a46251dc1b0ba311c85fd44f69530f780f2

Commits:
- 12bf1a4 Make pnpm build the single production build the native app embeds
- ffee181 Default vitadeck build to production; add --dev flag
- ffc8954 Run SDK CLI tests, lint and JS format check in CI and agent-check fast

Sources:
- ticket #57 Add the missing JS checks to CI and agent-check fast, default author builds to production with --dev, use one build mode in CMake, align reconciler types.
  https://github.com/matemolnar8/vitadeck/issues/57
- pr PR #63 Implements #57 items 1-3; leaves reconciler types at 0.28.9.
  https://github.com/matemolnar8/vitadeck/pull/63

The title:

Run CLI tests, lint and format check in CI; production builds by default

The why:

The only JS test, `lint` and `format:check` ran nowhere, shipped `.vdapp`s ran React in dev mode on the Vita, and the embedded runtime's mode depended on which build script ran last ([#57](source:s1)).

The what (small):

CI and `agent-check fast` now run `test:cli`, `lint` and `format:check`. `vitadeck build` produces production bundles unless `--dev` is passed. `pnpm build` is the single production build that CMake, CI and agent-check share.

Look for:
- Not done: [#57](source:s1) asks to align `@types/react-reconciler` with `react-reconciler` 0.29, but the catalog keeps 0.28.9. No 0.29 types are published; 0.28.x is the newest line for the React 18 reconciler, and 0.31+ type the React 19 reconciler.

## Review concerns

### 01 JS checks in CI and agent-check fast (`ci-checks`)

The CI `build` job and `run_fast` in `scripts/agent-check.sh` call the same three `pnpm --dir js` scripts, and the new root `test:cli` script forwards to `@vitadeck/sdk`.

[groups/ci-checks.md](groups/ci-checks.md)

### 02 `vitadeck build` defaults to production, `--dev` opts out (`cli-dev-flag`)

`main` in `cli.ts` accepts `--dev` and passes it as `BuildOptions.dev`, which picks the `process.env.NODE_ENV` define; the new tests build a fixture in both modes and check the inlined string.

[groups/cli-dev-flag.md](groups/cli-dev-flag.md)

### 03 One production build for the embedded runtime (`embedded-build`)

The runtime `rolldown.config.js` defaults to production, so `build:prod` is deleted and CMake's `js_build` and `js` targets call `pnpm build` like CI and agent-check.

[groups/embedded-build.md](groups/embedded-build.md)