Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` and `git rev-parse --verify 4968a00c087f538de9251d93c083192cee84bddf` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5fbdbe10b4830a6e343e9abe46f88aad89d5284a

head               4968a00c087f538de9251d93c083192cee84bddf

Named refs at pin: origin/t3/emoji-pack ... t3/audio-playback

Read the diff:

git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf

Review concern 07 of 07: Warning against probing the Vita (`probe-docs`)

Part: Vita probing warning

The why:

A bare connection takes VitaShell FTP and vitacompanion down until the Vita restarts, per the [commit](source:c-nc).

The what:

A new AGENTS.md footgun and matching lines in three skills tell agents to ask for the Vita IP and upload with curl instead of probing ports.

Hunk refs for this concern:
- AGENTS.md@50+51
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-emulator/SKILL.md
- .agents/skills/vitadeck-vdapp-upload/SKILL.md