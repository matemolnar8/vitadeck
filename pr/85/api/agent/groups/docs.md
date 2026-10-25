Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Review concern 04 of 04: README and upload skill (`docs`)

The why:

[#51](source:s1) asks for a "Develop on a Vita" README section and for the upload skill to point to `dev`.

The what:

The SDK README gains a "Develop on a Vita" section and a `create --force` line, and the `vitadeck-vdapp-upload` skill leads with `dev` and keeps the script and curl as the fallback.

Hunk refs for this concern:
- js/packages/sdk/README.md
- .agents/skills/vitadeck-vdapp-upload/SKILL.md