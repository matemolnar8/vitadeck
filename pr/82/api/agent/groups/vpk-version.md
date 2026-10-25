Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 9d671384dbb819d16682adc129c7e1e2ea8547a5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 9d671384dbb819d16682adc129c7e1e2ea8547a5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               9d671384dbb819d16682adc129c7e1e2ea8547a5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... feat/release-automation

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 9d671384dbb819d16682adc129c7e1e2ea8547a5

Review concern 02 of 07: VPK version from the tag (`vpk-version`)

The why:

The Vita wants `XX.YY`, so the release tag maps to the VPK version ([#52](source:s1)).

The what:

`VITADECK_VERSION=0.6.0` becomes `VITA_VERSION` `00.06` and anything not `X.Y.Z` is a configure error; the `PSVITAIP` line in the same hunk belongs to the env group.

Look for:
- Untagged builds keep `VITA_VERSION` `01.00`, so CI and local VPKs report a higher version than 0.x releases such as `00.06`.

Depends on:
- 01 Release workflow (`release`)

Hunk refs for this concern:
- CMakeLists.txt@40+40