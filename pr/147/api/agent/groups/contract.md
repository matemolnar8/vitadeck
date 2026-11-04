Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 05cd3b580fb6c9be2272de4548816decc5f3de40` and `git rev-parse --verify 92759426382c6cafaf057cbe8556ffe04ec30dd4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  05cd3b580fb6c9be2272de4548816decc5f3de40

head               92759426382c6cafaf057cbe8556ffe04ec30dd4

Named refs at pin: 05cd3b58 ... 92759426

Read the diff:

git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4

Review concern 02 of 06: `sounds` in the package contract, C and TypeScript (`contract`)

The why:

Footgun 2 in AGENTS.md: the manifest is produced by the CLI and validated natively, so both sides learn `sounds` together.

The what:

The CLI copies `sounds` to `sounds/` and types their names in `sound-names.d.ts`, and `package_manifest.c` accepts the same field, each through one shared asset routine instead of per-kind copies.

Attention (contract):
`schemaVersion` stays 1 although the manifest gains `sounds`. An older host skips the unknown key and loads the package, but `playSound` is then missing from its runtime SDK and throws. A package without sounds is byte-for-byte the same shape as before.

Hunk refs for this concern:
- src/core/package_manifest.c
- src/core/package_manifest.h
- tests/package_manifest_harness.c
- js/packages/sdk/cli/cli.ts
- js/packages/sdk/cli/package-format.test.ts
- js/packages/sdk/cli/test-project.ts
- js/packages/sdk/src/package-manifest.ts
- js/packages/sdk/src/types.ts