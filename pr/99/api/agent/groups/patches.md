Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... chore/vendor-patches-and-upload-assets

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d

Review concern 01 of 03: Vendor patch inventory (`patches`)

Part: Vendor patches

The why:

Patched vendor files drift from upstream and make updates risky, so every patch is recorded in one place ([#87](source:s1)).

The what:

`vendor/PATCHES.md` becomes the single record, and the jimp README and ADR 0003 now point to it instead of keeping their own list.

Hunk refs for this concern:
- vendor/PATCHES.md
- vendor/jimp/README.md
- docs/adr/0003-native-package-manifest-module.md