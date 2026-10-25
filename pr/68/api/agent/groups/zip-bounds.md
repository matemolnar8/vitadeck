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

Review concern 01 of 04: Zip offsets and per-entry allocation (`zip-bounds`)

The why:

[#43](source:s1) shows `central_offset + central_size`, `local_offset + 30` and `data_offset + compressed_size` wrapping on ARM32, plus one `arena_alloc` of up to 64MB per entry.

The what:

Every attacker-controlled offset in `upload_archive_extract` now goes through `in_bounds`, and `write_deflated` streams into the output file instead of allocating the whole entry.

Look for:
- Subtle: `local_offset + 30` was a `uint32_t` sum, so it wrapped on 64-bit hosts too; a central entry with `local_offset = 0xFFFFFFF0` segfaulted the old code and is now rejected with the invalid-local-data error.
- The unpacked-limit check now runs before `total_unpacked` is incremented, so an entry declaring `0xFFFFFFFF` bytes cannot wrap a 32-bit running total back under 64MB.
- A deflate stream that inflates past its declared `uncompressed_size` stops at the first 16KB chunk over the limit; the partial file stays in Runtime Upload Staging until the next `package_library_clear_staging`.

Hunk refs for this concern:
- src/upload/archive.c