Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` and `git rev-parse --verify 4968a00c087f538de9251d93c083192cee84bddf` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5fbdbe10b4830a6e343e9abe46f88aad89d5284a

head               4968a00c087f538de9251d93c083192cee84bddf

Named refs at pin: origin/t3/emoji-pack ... t3/audio-playback

Read the diff:

git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf

Review concern 02 of 07: `sounds` in the package contract, C and TypeScript (`contract`)

Part: Deck App Sounds

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