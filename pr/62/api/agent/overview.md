Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc` in this repository.
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

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               2fe57cb3eabcebb47c176abe8ff80f04cb372bfc

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/upload-handler-reaping

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 2fe57cb3eabcebb47c176abe8ff80f04cb372bfc

Commits:
- 2fe57cb fix(upload): reap finished handler threads so the listener keeps accepting

Sources:
- ticket #41 handler_count only grows, so the 17th connection is shut down; reap finished handlers and clear last_message after the Shell reads it.
  https://github.com/matemolnar8/vitadeck/issues/41
- pr PR #62 Handler slots with a done flag, joined in the accept loop; take-style message API.
  https://github.com/matemolnar8/vitadeck/pull/62

The title:

Reap finished upload handler threads so the listener keeps accepting

The why:

[The Runtime Upload listener stops accepting after 16 connections](source:s1), and browsers reach that limit in normal use.

The what (small):

The Runtime Upload listener now joins finished handler threads before admitting new clients. The Shell now consumes the last upload message instead of re-reading it every frame.

## Review concerns

### 01 Handler slots and reaping (`reaping`)

`handler_thread` marks its `VdUploadHandler` slot done under the mutex, and `server_thread` calls `reap_finished_handlers` to join done slots before choosing a free slot for the next client.

[groups/reaping.md](groups/reaping.md)

### 02 Shell consumes the upload message (`message`)

`upload_server_last_message` becomes `upload_server_take_message`, which clears the message and returns whether there was one. `shell_update` copies the message only when that call returns true.

[groups/message.md](groups/message.md)