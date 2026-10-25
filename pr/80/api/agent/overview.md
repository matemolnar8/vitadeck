Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 7cb2348a222dddbcef0fe877543e49f8ac5c57a5` in this repository.
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

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               7cb2348a222dddbcef0fe877543e49f8ac5c57a5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... fix/upload-sigpipe-and-drain

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 7cb2348a222dddbcef0fe877543e49f8ac5c57a5

Commits:
- 7cb2348 fix(upload): drain the body before closing a 409 or 413
- 26389be fix(upload): send without SIGPIPE and stop a reply on send failure

Sources:
- ticket #64 No SIGPIPE handling and unchecked send() in http_server.c.
  https://github.com/matemolnar8/vitadeck/issues/64
- ticket #79 Drain the body or reply with Connection: close and a short drain.
  https://github.com/matemolnar8/vitadeck/issues/79

The title:

fix(upload): ignore SIGPIPE and drain rejected request bodies

The why:

A client disconnecting mid-response can kill the whole VitaDeck process ([#64](source:s1)), and clients still sending an archive see a connection reset instead of the JSON error ([#79](source:s2)).

The what (small):

The Runtime Upload Listener sends with `MSG_NOSIGNAL` and checks every send, and after a 409 or 413 it keeps reading the request body before closing.

Look for:
- [#64](source:s1) suggests adding the scratch curl harness from #62 as a CTest; it is not added, because a CTest binding a LAN port can raise the macOS firewall prompt on dev machines. The socketpair harness covers both fixes instead.

## Review concerns

### 01 Send without SIGPIPE (`sigpipe`)

`send_response` now goes through `send_all`, which passes `VD_MSG_NOSIGNAL` from `src/platform/socket.h` and stops at the first failed send.

[groups/sigpipe.md](groups/sigpipe.md)

### 02 Drain the body after 409 and 413 (`drain`)

`drain_rejected_request` runs after the 409 and 413 replies in `handle_upload`: it shuts down the write side, then reads up to one maximum-size request with a 2 second `SO_RCVTIMEO`, and the harness proves a 1MB body can finish sending.

Depends on:
- 01 Send without SIGPIPE (`sigpipe`)

[groups/drain.md](groups/drain.md)