Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 318fcb8ebeeae91f24b52e267baedc1a50415461` in this repository.
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
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               318fcb8ebeeae91f24b52e267baedc1a50415461

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 318fcb8ebeeae91f24b52e267baedc1a50415461

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 318fcb8ebeeae91f24b52e267baedc1a50415461

Commits:
- 318fcb8 Address Codex review: staging target and separate pnpm commands
- cf4f997 Add comprehende and pages skills to the PR workflow
- c7e17ff Rewrite AGENTS.md and keep only VitaDeck skills
- a11a9f2 Apply oxfmt to JS workspace

Sources:
- pr PR #38 Rewrite AGENTS.md around VitaDeck, drop the unused mattpocock skills, fix the kept skills, and format the JS workspace.
  https://github.com/matemolnar8/vitadeck/pull/38
- pr-comment chatgpt-codex-connector on PR #38
  https://github.com/matemolnar8/vitadeck/pull/38#discussion_r4368155279
- pr-comment chatgpt-codex-connector on PR #38
  https://github.com/matemolnar8/vitadeck/pull/38#discussion_r4368155288
- pr-comment cursor on PR #38
  https://github.com/matemolnar8/vitadeck/pull/38
- commit Address Codex review: staging target and separate pnpm commands

The title:

Rewrite AGENTS.md and keep only VitaDeck skills

The why:

The Matt Pocock skill suite is [no longer in use](source:s1), and AGENTS.md only pointed at it.

The what (medium):

AGENTS.md becomes a VitaDeck-specific runbook, the third-party skill suite and its setup docs leave the repo, the five kept skills get corrected, comprehende and pages join the PR workflow, and the JS workspace gets an oxfmt pass.

Look for:
- The [PR description](source:s1) says `skills-lock.json` is removed. At head it is modified: it lists `comprehende` and `pages` from `matemolnar8/comprehende`, nothing else.
- The [PR description](source:s1) predates the last two commits. It does not mention the comprehende and pages skills, the new review step in the Pull requests section of AGENTS.md, or the [Codex fixes](source:s5).
- The [Vita VPK check failed](source:s4) on this branch. The same job fails on `main` at 1f305ea: the Dockerfile clones vitaGL master, which calls `shark_set_shader_association_path`, missing from the vitaShaRK in the pinned vitasdk image. Nothing in this diff touches the Dockerfile.
- Manual step: the hosted review URL returns 404 until GitHub Pages is enabled on the repo (Settings, Pages, Deploy from a branch, `gh-pages`, `/`).

## Review concerns

### 01 AGENTS.md rewrite (`agents-md`)

AGENTS.md is one rewritten file; its Pull requests section now asks for the comprehende review that the Comprehende PR workflow part installs.

[groups/agents-md.md](groups/agents-md.md)

### 02 Corrections to the five kept skills (`kept-skills`)

Four SKILL.md files change a few lines each so their commands and claims match the code at head.

Depends on:
- 01 AGENTS.md rewrite (`agents-md`)

[groups/kept-skills.md](groups/kept-skills.md)

### 03 Delete the third-party skills and their setup docs (`skill-removal`)

Every file here is a whole-file deletion of imported skill text, plus `skills-lock.json` losing their entries; nothing in the repo referenced them after the AGENTS.md rewrite.

Depends on:
- 01 AGENTS.md rewrite (`agents-md`)

[groups/skill-removal.md](groups/skill-removal.md)

### 04 domain.md deleted in both copies (`mirrors-1`)

The same deletion twice; read neither.

Depends on:
- 03 Delete the third-party skills and their setup docs (`skill-removal`)

[groups/mirrors-1.md](groups/mirrors-1.md)

### 05 triage-labels.md deleted in both copies (`mirrors-2`)

The same deletion twice; read neither.

Depends on:
- 03 Delete the third-party skills and their setup docs (`skill-removal`)

[groups/mirrors-2.md](groups/mirrors-2.md)

### 06 Install comprehende and pages, wire them into the PR workflow (`comprehende-skills`)

`.agents/skills/comprehende/` is the upstream skill copied verbatim by `npx skills add`, `.agents/skills/pages/SKILL.md` is the upstream pages skill edited to publish from a comprehende checkout with `--repo` pointing here, and `skills-lock.json` records both; AGENTS.md's Pull requests section calls them.

[groups/comprehende-skills.md](groups/comprehende-skills.md)

### 07 oxfmt line wrapping (`oxfmt`)

Every hunk rejoins parameter lists, call arguments, JSX props, or a union type onto one line under oxfmt's print width, and `js/packages/sdk/README.md` loses a trailing blank line.

[groups/oxfmt.md](groups/oxfmt.md)

### 08 Import line reformats (`mechanical`)

One-line import reformats with no symbol change; skip them.

Depends on:
- 07 oxfmt line wrapping (`oxfmt`)

[groups/mechanical.md](groups/mechanical.md)