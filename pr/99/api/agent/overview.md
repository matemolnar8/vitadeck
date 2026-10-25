Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d` in this repository.
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

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... chore/vendor-patches-and-upload-assets

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 d1eab1fd4fcce3dafda1cee8e6ff7f7b24aec32d

Commits:
- d1eab1f build(vita): upload_vita also uploads assets/emoji
- 2ba66c5 chore(vendor): record every local patch to vendored code

Sources:
- ticket #87 Record every vendor patch in one place; check whether the stb_ds fix can live in src/.
  https://github.com/matemolnar8/vitadeck/issues/87
- ticket #93 upload_vita only sends the binary, so emoji are missing without a full VPK install.
  https://github.com/matemolnar8/vitadeck/issues/93

The title:

Record vendored patches; upload_vita syncs emoji assets

The what (small):

Two stories: `vendor/PATCHES.md` records every local patch to vendored code, and the Vita `upload_vita` target now also uploads `assets/emoji`.

## Review concerns

### 01 Vendor patch inventory (`patches`)

`vendor/PATCHES.md` becomes the single record, and the jimp README and ADR 0003 now point to it instead of keeping their own list.

[groups/patches.md](groups/patches.md)

### 02 stb_ds.h back to upstream's trailing newline (`stb-ds`)

The only edit restores the final newline, so `diff` against upstream v0.67 shows just the `VitaDeck:` seed change.

Depends on:
- 01 Vendor patch inventory (`patches`)

[groups/stb-ds.md](groups/stb-ds.md)

### 03 upload_vita uploads assets/emoji (`emoji-upload`)

Configure writes `upload_emoji.curl` listing every PNG, and a stamp-guarded custom command that runs it becomes a dependency of `upload_vita`; the build skill documents the stamp.

[groups/emoji-upload.md](groups/emoji-upload.md)