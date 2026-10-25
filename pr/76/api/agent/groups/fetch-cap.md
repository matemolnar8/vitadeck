Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` and `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 4d6d7c5a7816464240a173bb7185dcb1c8090429 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5cbf7dacf365fc2a6999c53b263160e1c53db5d8

head               4d6d7c5a7816464240a173bb7185dcb1c8090429

Named refs at pin: origin/perf/dirty-flag-and-caches ... origin/build/warnings-sanitizers-js-limits

Read the diff:

git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 4d6d7c5a7816464240a173bb7185dcb1c8090429

Review concern 04 of 05: fetch body cap (`fetch-cap`)

The why:

`fetch` had no response-size cap ([#53](source:s1)).

The what:

`fetch_write_cb` stops curl once the body would pass `FETCH_MAX_BODY_BYTES`, and `fetch_worker` turns that into a rejection with a fixed message that `CONTEXT.md` documents for authors.

Look for:
- A 200 response with a 5 MiB body now rejects with `fetch response body exceeds the 4 MiB limit` instead of resolving; the `body_too_large` check runs before `res != CURLE_OK`, so curl's own write-error text never reaches JS.

Hunk refs for this concern:
- src/jslib/fetch.c@9+9
- src/jslib/fetch.c@32+35
- src/jslib/fetch.c@53+57
- src/jslib/fetch.c@109+117
- CONTEXT.md