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

Review concern 05 of 05: Re-activation assertion (`reactivation`)

The why:

[Item 2 of the issue](source:s1): the old assertion only checked `items[1].is_active`.

The what:

After re-activating `b.vdapp`, the harness also checks `!items[0].is_active` and that `package_library_active_package_name` still returns `b.vdapp`.

Hunk refs for this concern:
- tests/package_library_harness.c