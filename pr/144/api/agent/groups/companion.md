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

Review concern 02 of 08: Host Companion CLI (`companion`)

The why:

Slice B: the long-lived listener on the computer that performs the link and runs plugin commands ([#110](source:s2)).

The what:

`cli.ts` runs `start`, which serves `POST /command` from `server.ts` over the plugins `plugin.ts` loads, and `link`, which POSTs the link payload from `link.ts` and records the Vita in `config.ts`.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

Hunk refs for this concern:
- js/packages/host-companion/package.json
- js/packages/host-companion/tsconfig.json
- js/packages/host-companion/src/cli.ts
- js/packages/host-companion/src/config.ts
- js/packages/host-companion/src/core.ts
- js/packages/host-companion/src/index.ts
- js/packages/host-companion/src/link.ts
- js/packages/host-companion/src/plugin.ts
- js/packages/host-companion/src/server.ts
- js/packages/host-companion/test/fixtures/greeter-plugin.ts
- js/packages/host-companion/test/link.test.ts
- js/packages/host-companion/test/server.test.ts
- js/packages/host-companion/README.md
- js/package.json
- js/pnpm-workspace.yaml