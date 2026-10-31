Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 70e9ba2bb5603104bab02541ecf29a039796344b` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 70e9ba2bb5603104bab02541ecf29a039796344b -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               70e9ba2bb5603104bab02541ecf29a039796344b

Named refs at pin: a447e3b9 ... 70e9ba2b

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 70e9ba2bb5603104bab02541ecf29a039796344b

Review concern 05 of 09: Shared native helpers moved out of upload and storage (`native-shared`)

The why:

The link listener reuses Runtime Upload's request reading, the manifest's jimp parsing, and Deck App Storage's safe file replace, so [#111](source:s3) moves them where both callers reach them.

The what:

Upload's HTTP helpers move to `src/net/http.c`, its LAN address detection to `src/platform/lan_address_*`, jimp and JSON escaping to `src/core/json.c`, and Deck App Storage's replace-file logic to `vd_replace_file` in `src/core/util.c`.

Look for:
- A Deck App Storage write that fails to move the old file aside now reports "Could not write Deck App Storage." instead of "Could not replace Deck App Storage."; the `.tmp` and `.bak` names stay the same, so a backup left by an older build is still recovered.
- `http_read_request` always NUL-terminates the body it returns, so the link parser can hand it to jimp; Runtime Upload ignores the extra byte.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

Hunk refs for this concern:
- src/net/http.c
- src/net/http.h
- src/upload/http_parse.c
- src/upload/http_parse.h
- src/platform/lan_address.h
- src/platform/lan_address_posix.c
- src/platform/lan_address_vita.c
- src/core/json.c
- src/core/json.h
- src/core/package_manifest.c
- src/core/util.c
- src/core/util.h
- src/core/deck_app_storage.c
- tests/upload_parsing_harness.c