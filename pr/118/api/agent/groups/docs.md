Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47` and `git rev-parse --verify 109d347e5c7b68a18469c3a8cfffda8db7e201c1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e933a90f2ad45d2abbbbfac04a7b9a987bbad47

head               109d347e5c7b68a18469c3a8cfffda8db7e201c1

Named refs at pin: 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 ... 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Read the diff:

git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1

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