Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 4d13a180782047fd4564b62704c0469b4e25bcd1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 4d13a180782047fd4564b62704c0469b4e25bcd1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               4d13a180782047fd4564b62704c0469b4e25bcd1

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... 4d13a180782047fd4564b62704c0469b4e25bcd1

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 4d13a180782047fd4564b62704c0469b4e25bcd1

Review concern 02 of 03: Call sites copy into stack buffers (`callers`)

The why:

The accessors no longer return pointers, so each caller needs its own buffer.

The what:

`bootstrap.c` and `jslib.c` copy the active path into a `VD_PATH_MAX` buffer before using it.

Depends on:
- 01 Locking inside the package library (`library`)

Hunk refs for this concern:
- src/core/bootstrap.c
- src/jslib/jslib.c