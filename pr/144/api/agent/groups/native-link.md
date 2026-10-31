Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify fbb6af02ce01caf22a295e56ad95c7aacba7f803` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f fbb6af02ce01caf22a295e56ad95c7aacba7f803 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               fbb6af02ce01caf22a295e56ad95c7aacba7f803

Named refs at pin: a447e3b9 ... fbb6af02

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f fbb6af02ce01caf22a295e56ad95c7aacba7f803

Review concern 06 of 09: LAN Listener, link store and native getter (`native-link`)

The why:

Slice C: VitaDeck accepts links, keeps them across restarts, and lets Deck Apps read them ([#111](source:s3)). Links share the listener with Runtime Upload and start by default, as Máté asked after testing ([session](source:s5)).

The what:

`src/net/lan_listener.c`, the former `src/upload/http_server.c`, adds a `POST /link` route next to `/upload` on 8787+ and `main.c` starts it at boot; `link.c` validates a link and stores up to 4 through `vd_replace_file`, and `nativeGetHostControlLinks` in `jslib/host_control.c` hands the list to the SDK client.

Look for:
- Every client socket now gets a 15 s `SO_RCVTIMEO` per read, uploads included, so a client that goes quiet frees its handler slot; a slow but steady 16 MiB upload is unaffected.
- `lan_listener_stop` shuts client sockets down while holding the listener mutex, so a handler cannot close its fd and let a Deck App `fetch` reuse that number first.
- A link no longer stops the listener; a rejected or accepted link only leaves a `Link rejected: …` or `Linked to …` message for the Shell.

Depends on:
- 05 Shared native helpers moved out of upload and storage (`native-shared`)

Hunk refs for this concern:
- src/net/lan_listener.c
- src/net/lan_listener.h
- src/upload/http_server.c
- src/upload/http_server.h
- src/main.c
- tests/upload_single_flight_harness.c
- src/host_control/link.c
- src/host_control/link.h
- src/jslib/host_control.c
- src/jslib/jslib.c
- src/jslib/jslib_internal.h
- src/core/bootstrap.c
- js/packages/runtime/src/runtime-native.d.ts
- tests/host_control_link_harness.c
- CMakeLists.txt