Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc` and `git rev-parse --verify 7dc51e9de8877808099e2cb24a87b3813fb3c270` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- 7dc51e9 test(upload): add upload parsing CTest harness
- 51a0576 fix(upload): JSON-escape strings placed into upload responses
- 1e57906 fix(upload): match HTTP headers case-insensitively at line starts
- 85b55a7 fix(upload): bounds-check zip offsets without 32-bit wraparound

Sources:
- ticket #43 Overflow-checked zip offsets, case-insensitive headers, JSON escaping, plus a harness with traversal and overflow fixtures.
  https://github.com/matemolnar8/vitadeck/issues/43
- pr PR #68 Stacked on #62; host tier passed in the Linux smoke Docker image.
  https://github.com/matemolnar8/vitadeck/pull/68

The title:

Harden zip bounds, header matching and JSON escaping

The why:

[Issue #43](source:s1) reports that 32-bit offset sums let an archive read out of bounds on the Vita, that lowercase `content-length`/`content-type` from Node `fetch` fail with 400/415, and that `package_name` reaches the JSON response unescaped.

The what (small):

Runtime Upload checks zip offsets without 32-bit wraparound, inflates entries in fixed chunks, matches HTTP headers case-insensitively at line starts, JSON-escapes response strings, and gains a CTest harness for all of it.

Look for:
- [The issue](source:s1) suggests `__builtin_add_overflow`; the change uses an `in_bounds(offset, len, size)` subtraction check instead, which needs no compiler builtin and gives the same guarantee.

## Review concerns

### 01 Zip offsets and per-entry allocation (`zip-bounds`)

Every attacker-controlled offset in `upload_archive_extract` now goes through `in_bounds`, and `write_deflated` streams into the output file instead of allocating the whole entry.

[groups/zip-bounds.md](groups/zip-bounds.md)

### 02 Case-insensitive header lookup (`headers`)

`http_server.c` drops its `strstr`-based `header_value` and reads `Content-Length`, `Content-Type` and the multipart boundary through `upload_header_value` from the new `http_parse.c`.

[groups/headers.md](groups/headers.md)

### 03 JSON-escaped responses (`json-escape`)

`send_json_error` and the success body in `handle_upload` pass every string through `upload_json_escape` into buffers sized at 6x the input.

Depends on:
- 02 Case-insensitive header lookup (`headers`)

[groups/json-escape.md](groups/json-escape.md)

### 04 upload_parsing_harness CTest (`harness`)

`tests/upload_parsing_harness.c` builds single-entry zips in memory with forged offsets and runs them through `upload_archive_extract`, and `CMakeLists.txt` registers it next to the other non-smoke harnesses.

[groups/harness.md](groups/harness.md)