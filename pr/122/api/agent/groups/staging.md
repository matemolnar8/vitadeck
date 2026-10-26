Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 46e4bf3923487584c5fc6b0b43f586d245f9409d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 46e4bf3923487584c5fc6b0b43f586d245f9409d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               46e4bf3923487584c5fc6b0b43f586d245f9409d

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 46e4bf3923487584c5fc6b0b43f586d245f9409d

Review concern 01 of 03: Staging takes a data root (`staging`)

The why:

The emulator keeps the VitaDeck Data Root at `ux0/data/vitadeck`, not under a build directory, so staging needs the data root itself.

The what:

`stage-example-apps.sh` now takes the data root, and the `stage_examples` CMake target passes `<build>/vitadeck-data`, so host staging lands in the same place as before.

Hunk refs for this concern:
- scripts/stage-example-apps.sh
- CMakeLists.txt