Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995` and `git rev-parse --verify b0a30605c0c10bd3393bb263b9ce286e06f29813` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 b0a30605c0c10bd3393bb263b9ce286e06f29813 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995

head               b0a30605c0c10bd3393bb263b9ce286e06f29813

Named refs at pin: 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 ... t3/retro-nav-docs

Read the diff:

git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 b0a30605c0c10bd3393bb263b9ce286e06f29813

Review concern 01 of 11: gui-drive.sh and the manual-testing skill (`gui-drive`)

Part: GUI driving

The why:

Agents on ubuntu-pc, over SSH and on the MacBook Pro had no tmux or computerUse, so each one worked out window driving from scratch ([retro](source:s2)).

The what:

The skill's run, drive and recording steps now point at `scripts/gui-drive.sh`, which wraps xdotool and ffmpeg on Linux and a compiled Swift CGEvent helper on macOS.

Attention (tradeoff):
On macOS every `key` and `click` activates the vitadeck app with `activateIgnoringOtherApps`, taking focus from whatever the user is doing. Without it, CGEvent keys reach the frontmost app instead. The macOS path compiled and ran on the MacBook Pro, but keys and clicks there are untested.

Hunk refs for this concern:
- scripts/gui-drive.sh
- .agents/skills/vitadeck-manual-testing/SKILL.md