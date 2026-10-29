Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 0ec520145e1ff4657a25cec36682c2b86f5e6c15` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 0ec520145e1ff4657a25cec36682c2b86f5e6c15 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               0ec520145e1ff4657a25cec36682c2b86f5e6c15

Named refs at pin: a447e3b9 ... 0ec52014

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 0ec520145e1ff4657a25cec36682c2b86f5e6c15

Review concern 08 of 08: Glossary, ADR, agent skills and checks (`docs`)

The why:

Docs that the new design made inaccurate are fixed in place, and the companion tests join the `fast` tier ([#110](source:s2)).

The what:

`CONTEXT.md` adds the Host Control Plugin, Host Plugin Package, Link Listener and Shell terms, ADR 0004 is rewritten for plugins, built-ins, `.vdplugin` packages, standalone plugins, two CLIs and several links, the new public `vitadeck-host-control` skill and updates to the init, manual-testing, upload and build skills teach agents the feature, `AGENTS.md` lists the new code and skill, and `agent-check.sh fast` runs `test:host-companion`.

Hunk refs for this concern:
- CONTEXT.md
- docs/adr/0004-host-control-sliced-delivery.md
- AGENTS.md
- scripts/agent-check.sh
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-deck-app-init/SKILL.md@1+1
- .agents/skills/vitadeck-deck-app-init/SKILL.md@55+55
- .agents/skills/vitadeck-deck-app-init/SKILL.md@63+74
- .agents/skills/vitadeck-deck-app-init/SKILL.md@74+85
- .agents/skills/vitadeck-host-control/SKILL.md
- .agents/skills/vitadeck-manual-testing/SKILL.md@1+1
- .agents/skills/vitadeck-manual-testing/SKILL.md@22+22
- .agents/skills/vitadeck-manual-testing/SKILL.md@52+52
- .agents/skills/vitadeck-vdapp-upload/SKILL.md