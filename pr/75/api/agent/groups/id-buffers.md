Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` and `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  06341f44bca0e9643f246ebca9f9f173697c5600

head               5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Named refs at pin: origin/refactor/native-util-module ... HEAD

Read the diff:

git diff --find-renames 06341f44bca0e9643f246ebca9f9f173697c5600 5cbf7dacf365fc2a6999c53b263160e1c53db5d8

Review concern 03 of 04: Caller buffers for hit and scroll ids (`id-buffers`)

Part: Allocation-free input

The why:

`instance_hit_test` allocated a copy of the hit id every frame ([Máté's note](source:s2)).

The what:

`instance_hit_test`, `instance_scroll_at` and `instance_scroll_for_descendant` copy into a `VD_INSTANCE_ID_MAX` buffer under the snapshot lock, `input.c` passes stack buffers, and native create calls reject ids that would not fit.

Look for:
- Breaking for oversize ids only: a create call with an id of 64+ characters now logs a warning and creates nothing; runtime ids are at most 52 characters.
- Touch still `strdup`s the scroll id once per press into `touch_scroll_id`, which outlives the frame.

Hunk refs for this concern:
- src/ui/input.c
- src/ui/instance_tree.c@1+1
- src/ui/instance_tree.c@357+369
- src/ui/instance_tree.c@554+571
- src/ui/instance_tree.h@4+4
- src/ui/instance_tree.h@84+87
- src/ui/instance_tree.h@105+109
- src/jslib/instance.c