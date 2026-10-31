Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 07001bbdac339a0767b626376cda96d8530a1d37` and `git rev-parse --verify 600ca4ec1f763b6662f205fe8cb068bba809c1c1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 07001bbdac339a0767b626376cda96d8530a1d37 600ca4ec1f763b6662f205fe8cb068bba809c1c1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  07001bbdac339a0767b626376cda96d8530a1d37

head               600ca4ec1f763b6662f205fe8cb068bba809c1c1

Named refs at pin: 07001bbdac339a0767b626376cda96d8530a1d37 ... HEAD

Read the diff:

git diff --find-renames 07001bbdac339a0767b626376cda96d8530a1d37 600ca4ec1f763b6662f205fe8cb068bba809c1c1

Review concern 01 of 03: Publishable companion package (`package`)

The why:

The scaffold pins `^<companion version>`, so the companion has to be on npm at the version the SDK scaffolds; Máté [leaned toward lockstep](source:s2).

The what:

`package.json` drops `private` and moves to the SDK's 0.5.0, and the new `cli.test.ts` case fails when the two versions differ.

Look for:
- `repository.url` is required: npm Trusted Publishing rejects a GitHub publish whose `repository.url` does not match the repo.

Hunk refs for this concern:
- js/packages/host-companion/package.json
- js/packages/sdk/cli/cli.test.ts