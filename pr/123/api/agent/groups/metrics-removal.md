Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 37890a509945b80e7dbaeab3a5a87c58234b5e14` and `git rev-parse --verify a724ceea7f72a91e7420366537b01c70afbb9126` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  37890a509945b80e7dbaeab3a5a87c58234b5e14

head               a724ceea7f72a91e7420366537b01c70afbb9126

Named refs at pin: 37890a509945b80e7dbaeab3a5a87c58234b5e14 ... t3/perf-benchmark-suite

Read the diff:

git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126

Review concern 05 of 05: Drop always-on reconciler timing (`metrics-removal`)

Part: Reconciler timing removal

The why:

The wrapper timed every host config call and nothing read the numbers ([commit](source:s3)).

The what:

`reconciler-metrics.ts` is deleted, the reconciler passes `hostConfig` straight to `Reconciler`, and `main.tsx` stops logging and exporting the metrics; the `hostConfig` hunk also declares the `benchActive` constant the hooks group uses.

Look for:
- Breaking: `main.tsx` no longer exports `metrics`, and the reconciler no longer exports `getMetrics` or `resetMetrics`; neither is part of `@vitadeck/sdk`.

Hunk refs for this concern:
- js/packages/runtime/src/reconciler-metrics.ts
- js/packages/runtime/src/main.tsx
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@4+4
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@384+383
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@547+550
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@564+565