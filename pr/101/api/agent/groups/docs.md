Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5` and `git rev-parse --verify 1ac414061e4d8ecdbe718effe4daca0449592e05` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 1ac414061e4d8ecdbe718effe4daca0449592e05 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5

head               1ac414061e4d8ecdbe718effe4daca0449592e05

Named refs at pin: 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 ... feat/demo-deck-app

Read the diff:

git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 1ac414061e4d8ecdbe718effe4daca0449592e05

Review concern 06 of 06: README (`docs`)

The why:

The brief asks for a README in the example and one line under "Start building".

The what:

The example README maps each section to its file and explains how to run it on the host and with `vitadeck dev`; the root README links it.

Hunk refs for this concern:
- README.md
- js/examples/demo/README.md