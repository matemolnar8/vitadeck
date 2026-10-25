Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc` and `git rev-parse --verify 7dc51e9de8877808099e2cb24a87b3813fb3c270` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc 7dc51e9de8877808099e2cb24a87b3813fb3c270 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2fe57cb3eabcebb47c176abe8ff80f04cb372bfc

head               7dc51e9de8877808099e2cb24a87b3813fb3c270

Named refs at pin: origin/fix/upload-handler-reaping ... fix/upload-parsing-hardening

Read the diff:

git diff --find-renames 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc 7dc51e9de8877808099e2cb24a87b3813fb3c270

Review concern 04 of 04: upload_parsing_harness CTest (`harness`)

The why:

[#43](source:s1) asks for a CTest harness covering request parsing and zip extraction, including traversal and overflow fixtures.

The what:

`tests/upload_parsing_harness.c` builds single-entry zips in memory with forged offsets and runs them through `upload_archive_extract`, and `CMakeLists.txt` registers it next to the other non-smoke harnesses.

Look for:
- The harness links the real `package_library.c` and points it at a `mkdtemp` directory through `VITADECK_DATA_ROOT`, then removes that directory with `rm -rf` at exit.

Hunk refs for this concern:
- tests/upload_parsing_harness.c
- CMakeLists.txt@195+196