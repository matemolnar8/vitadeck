Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Named refs at pin: 04760101 ... a90a3cf6

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Review concern 01 of 06: SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

The why:

Authors write both halves of a plugin against `@vitadeck/sdk/host-control` alone, so a plugin needs no companion package; the other groups build on these exports.

The what:

`implementHostPlugin`, `HostCommandError`, `ImplementedHostPlugin` and the `HostPluginManifest` contract move into the SDK's `plugin.ts`, and the `systemStatsPlugin` registry moves from `js/host-plugins` into `system-stats.ts`, all re-exported from `index.ts`.

Look for:
- `implementHostPlugin` now ships inside every Deck App bundle that imports `@vitadeck/sdk/host-control`, though rolldown tree-shakes it from apps that never call it.

Hunk refs for this concern:
- js/packages/sdk/src/host-control/plugin.ts
- js/packages/sdk/src/host-control/index.ts
- js/packages/sdk/src/host-control/system-stats.ts