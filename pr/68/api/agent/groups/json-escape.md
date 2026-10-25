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

Review concern 03 of 04: JSON-escaped responses (`json-escape`)

The why:

[#43](source:s1): `package_name` is injected into the JSON response unescaped.

The what:

`send_json_error` and the success body in `handle_upload` pass every string through `upload_json_escape` into buffers sized at 6x the input.

Depends on:
- 02 Case-insensitive header lookup (`headers`)

Hunk refs for this concern:
- src/upload/http_parse.c
- src/upload/http_parse.h
- src/upload/http_server.c@120+122
- src/upload/http_server.c@392+377