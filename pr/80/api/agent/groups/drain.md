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

Review concern 02 of 02: Drain the body after 409 and 413 (`drain`)

The why:

Closing while the client still sends makes it see a reset instead of the JSON error ([#79](source:s2)).

The what:

`drain_rejected_request` runs after the 409 and 413 replies in `handle_upload`: it shuts down the write side, then reads up to one maximum-size request with a 2 second `SO_RCVTIMEO`, and the harness proves a 1MB body can finish sending.

Look for:
- Subtle: the 413 drain runs while that request still holds the ingest lock, so another upload during the drain (up to 16MB, or until 2 quiet seconds) gets 409.
- The reply goes out before the drain, so curl with `Expect: 100-continue` gets the 413 at once and never sends the body; the drain then ends on its EOF.
- `upload_once` in the harness now shuts down its write side, or every 413 case there would wait out the 2 second drain timeout.

Depends on:
- 01 Send without SIGPIPE (`sigpipe`)

Hunk refs for this concern:
- src/upload/http_server.c@8+8
- src/upload/http_server.c@23+25
- src/upload/http_server.c@83+97
- src/upload/http_server.c@329+357
- src/upload/http_server.c@340+369
- tests/upload_single_flight_harness.c@74+74
- tests/upload_single_flight_harness.c@104+143
- tests/upload_single_flight_harness.c@131+177