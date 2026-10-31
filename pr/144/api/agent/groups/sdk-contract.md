Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify ae5083caa9370b5f5985de1de0aa147a1afecc2e` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               ae5083caa9370b5f5985de1de0aa147a1afecc2e

Named refs at pin: a447e3b9 ... ae5083ca

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e

Review concern 01 of 09: Wire contract and SDK client (`sdk-contract`)

The why:

Slice A: the contract every other piece speaks, and the client Deck Apps call ([#109](source:s1)).

The what:

`createHostClient` in `client.ts` turns a `defineHostPlugin` registry from `plugin.ts` into typed methods that POST the `protocol.ts` envelope and map every failure to a `HostResult`, and `watchHostEvents` keeps one `core.events` wait open per watcher; `plugin.ts` also gives authors typed `event<D>()` declarations, `implementHostPlugin` with a `start(emit)` lifecycle, `HostCommandError` and the `HostPluginManifest` contract, and `system-stats.ts` holds the built-in `systemStats` registry.

Look for:
- `implementHostPlugin` and `HostCommandError` sit in the same subpath Deck Apps bundle; rolldown tree-shakes them from apps that never call them.
- A throw from a `watchHostEvents` callback stops watching and fails the Deck App, like a throw from any other author callback; a `stop()` from a callback ends delivery before the next event or wait.

Hunk refs for this concern:
- js/packages/sdk/src/host-control/protocol.ts
- js/packages/sdk/src/host-control/plugin.ts
- js/packages/sdk/src/host-control/client.ts
- js/packages/sdk/src/host-control/index.ts
- js/packages/sdk/package.json
- js/packages/sdk/tsconfig.json
- js/packages/sdk/src/deck-app-globals.d.ts
- js/packages/sdk/test/host-control.test.ts
- js/packages/sdk/cli/package-format.test.ts
- js/packages/sdk/cli/test-project.ts
- js/packages/sdk/README.md
- tests/fixtures/host-control/capabilities.json
- tests/fixtures/host-control/command-version-mismatch.json
- tests/fixtures/host-control/echo.json
- tests/fixtures/host-control/invalid-payload.json
- tests/fixtures/host-control/link-invalid-url.json
- tests/fixtures/host-control/link-ok.json
- tests/fixtures/host-control/link-version-mismatch.json
- tests/fixtures/host-control/unknown-command.json
- tests/fixtures/host-control/unknown-plugin.json
- js/packages/sdk/src/host-control/system-stats.ts
- tests/fixtures/host-control/link-relink-unknown.json