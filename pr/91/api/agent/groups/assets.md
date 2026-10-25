Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2` and `git rev-parse --verify 8468e93ba1ac7e88961ea5137dc35753a0e46643` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2

head               8468e93ba1ac7e88961ea5137dc35753a0e46643

Named refs at pin: 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 ... HEAD

Read the diff:

git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643

Review concern 01 of 05: Twemoji download at configure time (`assets`)

The why:

Máté picked Twemoji (CC-BY 4.0) for emoji art in the [design](source:s1), and the [change request](source:s3) asks for the images to be downloaded by the build into a gitignored folder, not committed.

The what:

`cmake/twemoji.cmake` downloads the v17.0.3 tag zip once, checks its SHA-256, extracts `assets/72x72` into `assets/emoji` without `-fe0f` and writes a stamp, which `.gitignore` hides, the CI workflows cache and the VPK packs; the licence and attribution stay tracked.

Look for:
- A failed download or hash mismatch prints a CMake warning, writes no stamp and leaves the build to carry on; the next configure retries. A present stamp skips the network entirely, even when PNGs were deleted by hand.
- The download writes into the source tree, not the build directory, so `out`, `out-smoke`, `out-sanitize` and `out-vita` share one copy; Docker runs as root, which is why `smoke-docker.sh` now chowns `assets/emoji`.
- `VD_TWEMOJI_DOWNLOADED` makes the harness check a renamed file (`1f3f3-200d-1f308.png`) only when the configure found the stamp.

Hunk refs for this concern:
- CMakeLists.txt
- cmake/twemoji.cmake
- .gitignore
- assets/emoji/LICENSE.txt
- assets/emoji/README.md
- .github/workflows/ci.yml
- .github/workflows/vita-build.yml
- scripts/smoke-docker.sh