Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify c0498393dd5911d602c63433160a3a7fa020e308` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 c0498393dd5911d602c63433160a3a7fa020e308 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               c0498393dd5911d602c63433160a3a7fa020e308

Named refs at pin: 04760101 ... c0498393

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 c0498393dd5911d602c63433160a3a7fa020e308

Review concern 05 of 06: Companion package, workspace and the demo app (`packaging`)

The why:

The companion becomes something users install globally, and the old `js/host-plugins` package has nowhere left to live.

The what:

The companion's `package.json` gains the `vitadeck-host-companion` bin and a rolldown build that inlines the SDK so it has no dependencies, `js/host-plugins` leaves the workspace, and the demo Deck App and Shell hint switch to the built-in registry and the new command name.

Look for:
- The demo's `failureHint` loses its `unknown_plugin` branch, since `systemStats` is now always present on a current companion.
- The root `build` script now also builds the companion, so the host CMake build's JS step runs it too.

Depends on:
- 03 Companion CLI: vitadeck-host-companion install, remove and hot reload (`companion-cli`)

Hunk refs for this concern:
- js/packages/host-companion/package.json
- js/packages/host-companion/rolldown.config.mjs
- .gitignore
- js/package.json@5+5
- js/pnpm-workspace.yaml
- js/host-plugins/system-stats/package.json
- js/host-plugins/system-stats/tsconfig.deck-app.json
- js/examples/host-control/package.json
- js/examples/host-control/src/App.tsx
- src/shell/shell.c