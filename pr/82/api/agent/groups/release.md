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

Review concern 01 of 07: Release workflow (`release`)

The why:

A pushed tag cuts the release instead of hand steps ([#52](source:s1)).

The what:

`release.yml` checks the tag against the SDK version, calls `vita-build.yml` through its new `workflow_call` with the tag as `version`, then publishes the packed SDK and attaches the downloaded VPK to `gh release create`.

Look for:
- Subtle: `npm publish` runs before `gh release create`, so if the release step fails, re-running the job fails at publish because that npm version already exists; the release then needs a manual `gh release create`.

Hunk refs for this concern:
- .github/workflows/release.yml
- .github/workflows/vita-build.yml@6+6
- .github/workflows/vita-build.yml@61+67
- .github/workflows/vita-build.yml@72+80