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

Review concern 02 of 02: Shell consumes the upload message (`message`)

The why:

[Clear `last_message` after the Shell reads it](source:s1), so the upload message stops overwriting later Shell messages every frame.

The what:

`upload_server_last_message` becomes `upload_server_take_message`, which clears the message and returns whether there was one. `shell_update` copies the message only when that call returns true.

Hunk refs for this concern:
- src/upload/http_server.h@32+36
- src/upload/http_server.c@588+616
- src/shell/shell.c