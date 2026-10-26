Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 613ea51e22a4e3ffb93b28da7f52c16830b91e83` and `git rev-parse --verify e2a3b22bb6662ae3fc7556a750655d76a2dda7f9` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 613ea51e22a4e3ffb93b28da7f52c16830b91e83 e2a3b22bb6662ae3fc7556a750655d76a2dda7f9 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  613ea51e22a4e3ffb93b28da7f52c16830b91e83

head               e2a3b22bb6662ae3fc7556a750655d76a2dda7f9

Named refs at pin: 613ea51e22a4e3ffb93b28da7f52c16830b91e83 ... HEAD

Read the diff:

git diff --find-renames 613ea51e22a4e3ffb93b28da7f52c16830b91e83 e2a3b22bb6662ae3fc7556a750655d76a2dda7f9

Review concern 01 of 02: Input time budget (`budget`)

The why:

Bounding per-iteration input work is [the fix the ticket proposes](source:s1).

The what:

`process_input_events` stamps a deadline 16 ms out before the first pop and breaks after any event that ends past it, so one event always runs.

Look for:
- Subtle: a `pressstart` and its `pressend` can now land in different iterations, with timers, microtasks and a swap between them; the pressed state can paint before the release is handled.

Hunk refs for this concern:
- src/jslib/jslib.c