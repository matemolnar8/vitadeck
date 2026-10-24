Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 318fcb8ebeeae91f24b52e267baedc1a50415461` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 318fcb8ebeeae91f24b52e267baedc1a50415461 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               318fcb8ebeeae91f24b52e267baedc1a50415461

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 318fcb8ebeeae91f24b52e267baedc1a50415461

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 318fcb8ebeeae91f24b52e267baedc1a50415461

Review concern 03 of 08: Delete the third-party skills and their setup docs (`skill-removal`)

Part: Agents runbook

The why:

The [PR](source:s1) removes the mattpocock suite and `frontend-design`; `docs/agents/` and `skills-lock.json` entries existed only for those skills.

The what:

Every file here is a whole-file deletion of imported skill text, plus `skills-lock.json` losing their entries; nothing in the repo referenced them after the AGENTS.md rewrite.

Depends on:
- 01 AGENTS.md rewrite (`agents-md`)

Hunk refs for this concern:
- .agents/skills/ask-matt/SKILL.md
- .agents/skills/code-review/SKILL.md
- .agents/skills/codebase-design/DEEPENING.md
- .agents/skills/codebase-design/DESIGN-IT-TWICE.md
- .agents/skills/codebase-design/SKILL.md
- .agents/skills/diagnosing-bugs/SKILL.md
- .agents/skills/diagnosing-bugs/scripts/hitl-loop.template.sh
- .agents/skills/domain-modeling/ADR-FORMAT.md
- .agents/skills/domain-modeling/CONTEXT-FORMAT.md
- .agents/skills/domain-modeling/SKILL.md
- .agents/skills/frontend-design/LICENSE.txt
- .agents/skills/frontend-design/SKILL.md
- .agents/skills/grill-me/SKILL.md
- .agents/skills/grill-with-docs/SKILL.md
- .agents/skills/grilling/SKILL.md
- .agents/skills/handoff/SKILL.md
- .agents/skills/implement/SKILL.md
- .agents/skills/improve-codebase-architecture/HTML-REPORT.md
- .agents/skills/improve-codebase-architecture/SKILL.md
- .agents/skills/prototype/LOGIC.md
- .agents/skills/prototype/SKILL.md
- .agents/skills/prototype/UI.md
- .agents/skills/research/SKILL.md
- .agents/skills/setup-matt-pocock-skills/SKILL.md
- .agents/skills/setup-matt-pocock-skills/issue-tracker-github.md
- .agents/skills/setup-matt-pocock-skills/issue-tracker-gitlab.md
- .agents/skills/setup-matt-pocock-skills/issue-tracker-local.md
- .agents/skills/tdd/SKILL.md
- .agents/skills/tdd/mocking.md
- .agents/skills/tdd/tests.md
- .agents/skills/teach/GLOSSARY-FORMAT.md
- .agents/skills/teach/LEARNING-RECORD-FORMAT.md
- .agents/skills/teach/MISSION-FORMAT.md
- .agents/skills/teach/RESOURCES-FORMAT.md
- .agents/skills/teach/SKILL.md
- .agents/skills/to-issues/SKILL.md
- .agents/skills/to-prd/SKILL.md
- .agents/skills/triage/AGENT-BRIEF.md
- .agents/skills/triage/OUT-OF-SCOPE.md
- .agents/skills/triage/SKILL.md
- .agents/skills/writing-great-skills/GLOSSARY.md
- .agents/skills/writing-great-skills/SKILL.md
- docs/agents/issue-tracker.md
- skills-lock.json