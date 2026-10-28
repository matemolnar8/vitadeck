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

Review concern 03 of 08: system-stats plugin (`system-stats`)

The why:

The first plugin proves the plugin split and gives the demo live gauges without acting on the computer ([PR #144](source:s4)).

The what:

`index.ts` is the registry a Deck App bundles and `companion.ts` implements its one `read` command with `node:os`, reading `vm_stat` on macOS.

Look for:
- On macOS `memoryUsedPercent` counts free, inactive and speculative pages from `vm_stat` as available; elsewhere it uses `os.freemem()`, which reads `MemAvailable` on Linux.

Depends on:
- 02 Host Companion CLI (`companion`)

Hunk refs for this concern:
- js/host-plugins/system-stats/package.json
- js/host-plugins/system-stats/src/companion.ts
- js/host-plugins/system-stats/src/index.ts
- js/host-plugins/system-stats/tsconfig.deck-app.json
- js/host-plugins/system-stats/tsconfig.json