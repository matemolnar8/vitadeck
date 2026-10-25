Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 7cb2348a222dddbcef0fe877543e49f8ac5c57a5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 7cb2348a222dddbcef0fe877543e49f8ac5c57a5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               7cb2348a222dddbcef0fe877543e49f8ac5c57a5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... fix/upload-sigpipe-and-drain

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 7cb2348a222dddbcef0fe877543e49f8ac5c57a5

Review concern 01 of 02: Send without SIGPIPE (`sigpipe`)

The why:

Writing to a client that hung up raised SIGPIPE and killed the host ([#64](source:s1)).

The what:

`send_response` now goes through `send_all`, which passes `VD_MSG_NOSIGNAL` from `src/platform/socket.h` and stops at the first failed send.

Look for:
- On Vita `VD_MSG_NOSIGNAL` is 0 because the Vita headers have no `MSG_NOSIGNAL` and sceNet never raises SIGPIPE; only the Docker Vita build proves this compiles.
- When the 200 reply fails to send, the package is still published and the Shell still sees the success; only the client misses the JSON.

Hunk refs for this concern:
- src/platform/socket.h
- src/upload/http_server.c@8+8
- src/upload/http_server.c@72+75
- src/upload/http_server.c@83+97
- tests/upload_single_flight_harness.c@142+197
- tests/upload_single_flight_harness.c@155+229