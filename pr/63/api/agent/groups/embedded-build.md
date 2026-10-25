Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 12bf1a46251dc1b0ba311c85fd44f69530f780f2` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 12bf1a46251dc1b0ba311c85fd44f69530f780f2 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 03 of 03: One production build for the embedded runtime (`embedded-build`)

The why:

CMake used `build:prod` while CI and agent-check used `build`, so the runtime in the native build depended on which ran last ([#57](source:s1)).

The what:

The runtime `rolldown.config.js` defaults to production, so `build:prod` is deleted and CMake's `js_build` and `js` targets call `pnpm build` like CI and agent-check.

Look for:
- `pnpm --dir js build` now emits a minified production `runtime.js`; any `NODE_ENV` other than `development` (including unset or `test`) yields production, and `NODE_ENV=development pnpm --dir js build` is the only way to get the dev runtime.

Hunk refs for this concern:
- js/packages/runtime/rolldown.config.js
- js/packages/runtime/package.json
- CMakeLists.txt
- js/package.json