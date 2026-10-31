Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify ae5083caa9370b5f5985de1de0aa147a1afecc2e` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               ae5083caa9370b5f5985de1de0aa147a1afecc2e

Named refs at pin: a447e3b9 ... ae5083ca

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e

Review concern 03 of 09: Events from the computer by long-polling (`events`)

The why:

Máté asked for a better host→Vita direction than polling ([session](source:s6)): events reach the Vita within one round trip while the Vita keeps making only outbound calls.

The what:

`events.ts` numbers each plugin's last 100 events and holds a `core.events` wait until that plugin's next one or 25 s, and `fetch.c` aborts pending fetches through curl's progress callback when a Deck App stops, which `js_limits_harness` proves with a server that never answers.

Look for:
- A wait with no `after`, or an `after` ahead of the log (a companion restart), answers at once with `next` and starts watching from now; events are best effort across restarts and beyond 100 per plugin.
- `fetch_shutdown` sets `cancelled` under `fetch_mutex` before joining, and the progress callback, called about once a second while curl connects or waits, returns non-zero to abort; a Deck App reload with a held wait takes about a second instead of up to 25 s.
- Every idle watching Deck App costs one request at the companion, renewed every 25 s, and one native fetch thread on the Vita.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)
- 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

Hunk refs for this concern:
- js/packages/host-companion/src/events.ts
- src/jslib/fetch.c@39+39
- src/jslib/fetch.c@90+92
- src/jslib/fetch.c@110+125
- src/jslib/fetch.c@335+353
- tests/js_limits_harness.c@8+8
- tests/js_limits_harness.c@84+88
- tests/js_limits_harness.c@110+159