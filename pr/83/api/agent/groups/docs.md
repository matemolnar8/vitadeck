Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Review concern 07 of 07: Glossary and SDK README (`docs`)

The why:

AGENTS.md requires host mappings in `CONTEXT.md` to match `src/ui/input.c`, and [the approval](source:s1) asks for every new input to be listed there.

The what:

`CONTEXT.md` adds **Focus**, **Pad Button**, **Host Pad Button Mapping**, the Select System Input and one Host mappings table that the SDK README links to from its new focus section.

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md