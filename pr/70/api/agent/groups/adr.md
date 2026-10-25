Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5

Named refs at pin: origin/main ... HEAD

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5

Review concern 01 of 02: ADR 0004: slices, topology, parser, deferrals (`adr`)

The why:

[#45](source:s1) requires an ADR before the native slice and asks for the work to be re-sliced into three PRs.

The what:

The ADR's topology choice (Vita calls the companion over existing `fetch()`) is what lets its parser section reject picohttpparser and its delivery section keep slice (c) small.

Look for:
- Subtle: the ADR claims `header_value` in `src/upload/http_server.c` matches header names case-sensitively and that Node's `fetch` sends `content-length`; a Node `fetch` POST to the upload server today would therefore get a 400 for a missing Content-Length.
- The link listener is Shell-gated and stops after a successful link, which reverses #16's always-on shared LAN listener; the Runtime Upload lifecycle stays as `CONTEXT.md` describes it.
- The ADR makes a credential decision a prerequisite for the first command that acts on the computer, because the companion accepts commands from any LAN peer in v1.

Hunk refs for this concern:
- docs/adr/0004-host-control-sliced-delivery.md