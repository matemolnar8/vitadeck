Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 51440f28e10cf982764134aff0b4d94aa23743df` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 51440f28e10cf982764134aff0b4d94aa23743df -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               51440f28e10cf982764134aff0b4d94aa23743df

Named refs at pin: a447e3b9 ... 51440f28

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 51440f28e10cf982764134aff0b4d94aa23743df

Review concern 05 of 08: Link listener, store and native getter (`native-link`)

The why:

Slice C: VitaDeck accepts a link only while asked, keeps it across restarts, and lets Deck Apps read it ([#111](source:s3)).

The what:

`listener.c` accepts one `POST /link` on 8798+, `link.c` validates it and stores up to 4 links through `vd_replace_file`, and `nativeGetHostControlLinks` in `jslib/host_control.c` hands the list to the SDK client.

Depends on:
- 04 Shared native helpers moved out of upload and storage (`native-shared`)

Hunk refs for this concern:
- src/host_control/link.c
- src/host_control/link.h
- src/host_control/listener.c
- src/host_control/listener.h
- src/jslib/host_control.c
- src/jslib/jslib.c
- src/jslib/jslib_internal.h
- src/core/bootstrap.c
- js/packages/runtime/src/runtime-native.d.ts
- tests/host_control_link_harness.c
- CMakeLists.txt