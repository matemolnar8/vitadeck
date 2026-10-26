Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify e7d27d003d9cf91aaf9645382cb60facbc614bf0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               e7d27d003d9cf91aaf9645382cb60facbc614bf0

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... HEAD

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0

Review concern 04 of 04: One-time tests/ reformat (`reformat`)

Part: Format gate

The why:

The new gate fails until the existing harnesses match `.clang-format`.

The what:

Whitespace and line-wrap changes only from `clang-format -i`, in its own commit `a46f4ed`; the reader can skip them.

Depends on:
- 03 clang-format covers tests/ (`gate`)

Hunk refs for this concern:
- tests/glyph_cache_harness.c@269+269
- tests/glyph_cache_harness.c@317+318
- tests/glyph_cache_harness.c@428+438
- tests/glyph_cache_harness.c@611+626
- tests/input_harness.c
- tests/js_limits_harness.c
- tests/layout_harness.c
- tests/package_library_harness.c
- tests/smoke_harness.c
- tests/storage_harness.c
- tests/timer_reload_harness.c
- tests/upload_single_flight_harness.c