Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               2fe57cb3eabcebb47c176abe8ff80f04cb372bfc

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/upload-handler-reaping

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc

Review concern 01 of 02: Handler slots and reaping (`reaping`)

The why:

[Handler threads were only joined at stop](source:s1), so the fixed 16-entry handler array filled up for good.

The what:

`handler_thread` marks its `VdUploadHandler` slot done under the mutex, and `server_thread` calls `reap_finished_handlers` to join done slots before choosing a free slot for the next client.

Look for:
- Subtle: a handler can set `done` before `server_thread` stores its `thread` pointer. That is safe because only `server_thread` reads or writes `.thread` and reaps on its next accept.
- Finished handlers stay unjoined until the next `accept` or `upload_server_stop`, so at most 16 exited threads wait to be joined while the listener is idle.
- When all 16 slots hold live clients, a new connection is closed without a response, as it was before.

Hunk refs for this concern:
- src/upload/http_server.h@10+10
- src/upload/http_server.c@24+24
- src/upload/http_server.c@420+443
- src/upload/http_server.c@438+464
- src/upload/http_server.c@445+473
- src/upload/http_server.c@456+491
- src/upload/http_server.c@465+501
- src/upload/http_server.c@556+584