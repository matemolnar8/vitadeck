Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 282d57c499f34473e74cbd281d12ce11fe79eadc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               282d57c499f34473e74cbd281d12ce11fe79eadc

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 282d57c

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc

Review concern 02 of 09: Corrections to the five kept skills (`kept-skills`)

Part: Agents runbook

The why:

Each kept skill was checked against CMakeLists, scripts, `src/`, and the SDK CLI; the [PR](source:s1) lists the stale claims found.

The what:

Four SKILL.md files change a few lines each so their commands and claims match the code at head.

Look for:
- Subtle: `vitadeck-manual-testing` replaces copying one `chat.vdapp` with `cmake --build out --target stage_examples`, which copies every example that has a `dist/*.vdapp` and exits 1 when none is built. `active-package.txt` is still written by hand.
- `vitadeck-manual-testing` drops the claim that the on-device upload URL can omit the port; `src/upload/http_server.c` formats the URL with the bound port.

Depends on:
- 01 AGENTS.md rewrite (`agents-md`)

Hunk refs for this concern:
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-deck-app-init/SKILL.md
- .agents/skills/vitadeck-manual-testing/SKILL.md
- .agents/skills/vitadeck-vdapp-upload/SKILL.md