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

Review concern 06 of 07: Releasing doc (`docs`)

The why:

Máté needs the two release steps and the one-time npm setup written down ([brief](source:s2)).

The what:

`docs/releasing.md` lists the version-bump PR, the tag push and the Trusted Publisher setup, and `AGENTS.md` links it.

Hunk refs for this concern:
- docs/releasing.md
- AGENTS.md