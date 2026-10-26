Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               33b2ca703b09bf38890ad2ab3d952fdc75edd9b5

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... t3/tech-debt-117

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee 33b2ca703b09bf38890ad2ab3d952fdc75edd9b5

Review concern 04 of 05: shell_input_harness (`harness`)

The why:

Covers what [the issue lists](source:s1): restart, removal confirm/cancel and upload cancel.

The what:

The harness publishes two packages into a temp `VITADECK_DATA_ROOT`, stubs `input_ignore_held_pad_buttons`, and walks the Shell through open/hide, restart, activation, removal and the Upload listener with single-button `VdShellInput` frames; CMake builds and registers it with the real package library and upload server.

Look for:
- The upload test binds a real listener on 8787-8796, so a running host listener only pushes it to the next port.

Depends on:
- 01 Shell input snapshot (`snapshot`)
- 02 Shell message cleared on hide (`message`)
- 03 Upload server mutex freed on shutdown (`mutex`)

Hunk refs for this concern:
- tests/shell_input_harness.c
- CMakeLists.txt