Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 96ddad42a99c962958f50f86b75a371d80cb28b7` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 96ddad42a99c962958f50f86b75a371d80cb28b7 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               96ddad42a99c962958f50f86b75a371d80cb28b7

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 96ddad42a99c962958f50f86b75a371d80cb28b7

Review concern 03 of 03: Glossary lines (`glossary`)

The why:

The issue asks to update the Shell glossary entries ([issue](source:s1)).

The what:

`Deck App Runtime Restart` gains the Shell confirm as a trigger, and `Shell Home Screen` says what confirming a package and the Active Deck App does.

Hunk refs for this concern:
- CONTEXT.md