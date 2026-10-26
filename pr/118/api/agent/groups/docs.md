Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 81705ef32a1e4968734146a11cbd9567880f5f7e` and `git rev-parse --verify 6afe270d3820e867d31a904d54495e96f4218c2d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 6afe270d3820e867d31a904d54495e96f4218c2d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  81705ef32a1e4968734146a11cbd9567880f5f7e

head               6afe270d3820e867d31a904d54495e96f4218c2d

Named refs at pin: 81705ef32a1e4968734146a11cbd9567880f5f7e ... 6afe270d3820e867d31a904d54495e96f4218c2d

Read the diff:

git diff --find-renames 81705ef32a1e4968734146a11cbd9567880f5f7e 6afe270d3820e867d31a904d54495e96f4218c2d

Review concern 05 of 05: Author docs for Text in Stack (`docs`)

The why:

The SDK README, the `Stack` JSDoc and `CONTEXT.md` said Text was not laid out ([#104](source:s1)).

The what:

`CONTEXT.md` gains a **Text** entry that defines the measured size and the Rect stacking rule, and the README Layout section and `Stack` JSDoc say that Text flows.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md
- js/packages/sdk/src/index.tsx