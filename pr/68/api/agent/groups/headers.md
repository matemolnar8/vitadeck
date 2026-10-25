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

Review concern 02 of 04: Case-insensitive header lookup (`headers`)

The why:

[#43](source:s1): Node `fetch` and undici send lowercase `content-length` and `content-type`, which the case-sensitive `strstr` lookup rejected.

The what:

`http_server.c` drops its `strstr`-based `header_value` and reads `Content-Length`, `Content-Type` and the multipart boundary through `upload_header_value` from the new `http_parse.c`.

Look for:
- Behavior change no source names: `content_type_is` compares the media type before `;` exactly, so `multipart/form-data; boundary=application/zip` now takes the multipart path where the old `strstr` picked the raw-zip path.
- `upload_content_length` returns -1 for `Content-Length: 12abc`, so such a request now gets 400 instead of being read as 12 bytes.
- A `Content-Type` value longer than 255 bytes does not fit the copy buffer and is treated as absent, giving 415.

Hunk refs for this concern:
- src/upload/http_parse.c
- src/upload/http_parse.h
- src/upload/http_server.c@6+6
- src/upload/http_server.c@155+161
- src/upload/http_server.c@195+180
- src/upload/http_server.c@241+226
- src/upload/http_server.c@251+236
- src/upload/http_server.c@293+278
- CMakeLists.txt@130+130