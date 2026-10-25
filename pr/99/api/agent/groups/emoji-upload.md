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

Review concern 03 of 03: upload_vita uploads assets/emoji (`emoji-upload`)

Part: Emoji upload

The why:

After a code-only upload a Vita without a prior VPK install renders emoji as missing ([#93](source:s2)).

The what:

Configure writes `upload_emoji.curl` listing every PNG, and a stamp-guarded custom command that runs it becomes a dependency of `upload_vita`; the build skill documents the stamp.

Look for:
- Subtle: the emoji step is a dependency of `upload_vita`, so it runs before the `destroy` command while the old app is still running.
- Subtle: the stamp only tracks this build directory and Vita IP, so removing the app from the Vita and then running `upload_vita` skips the emoji until `out-vita/upload_emoji-<ip>.stamp` is deleted.
- The PNG list is globbed at configure time, so images downloaded after configure are not uploaded until CMake reconfigures.

Hunk refs for this concern:
- CMakeLists.txt
- .agents/skills/vitadeck-build/SKILL.md