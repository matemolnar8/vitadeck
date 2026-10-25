Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8c25053b52e85f60efa5027dffa62bae617dbee1` and `git rev-parse --verify 0f7b8aa7abe6171068e8a6195e17d7545a857733` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8c25053b52e85f60efa5027dffa62bae617dbee1

head               0f7b8aa7abe6171068e8a6195e17d7545a857733

Named refs at pin: 8c25053b52e85f60efa5027dffa62bae617dbee1 ... 0f7b8aa7abe6171068e8a6195e17d7545a857733

Read the diff:

git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733

Review concern 05 of 07: Timers example uses Stack (`example`)

The why:

[The brief](source:s2) asks for Stack in a non-smoke example.

The what:

A column Stack replaces the hand-computed row `y` values, and a row Stack replaces each pair of button `x` values, landing on the same integer positions.

Depends on:
- 04 SDK Stack component and reconciler case (`sdk-runtime`)

Hunk refs for this concern:
- js/examples/timers/src/App.tsx