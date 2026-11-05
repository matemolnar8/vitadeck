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

Review concern 02 of 11: FTP and Vita3K lessons in the repo (`device-lessons`)

Part: Device lessons

The why:

The one-upload FTP rule and the Vita3K headless limits lived only in ubuntu-pc's agent memory, and a follow-up FTP listing crashed the Vita ([retro](source:s2)).

The what:

Footgun 6 in `AGENTS.md` becomes the single statement of the FTP rule, the build skill points at it, and the emulator skill gains the headless, `active-package.txt` and `config.yml` facts.

Hunk refs for this concern:
- AGENTS.md@51+51
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-emulator/SKILL.md