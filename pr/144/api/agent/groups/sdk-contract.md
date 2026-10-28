Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify eb61ca83421e50b68494e84294baac13b7c346d1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               eb61ca83421e50b68494e84294baac13b7c346d1

Named refs at pin: a447e3b9 ... HEAD

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1

Review concern 01 of 08: Wire contract and SDK client (`sdk-contract`)

The why:

Slice A: the contract every other piece speaks, and the client Deck Apps call ([#109](source:s1)).

The what:

`createHostClient` in `client.ts` turns a `defineHostPlugin` registry from `plugin.ts` into typed methods that POST the `protocol.ts` envelope and map every failure to a `HostResult`, checked against the shared fixtures.

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