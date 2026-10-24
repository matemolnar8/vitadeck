Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 282d57c499f34473e74cbd281d12ce11fe79eadc` in this repository.
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

head               282d57c499f34473e74cbd281d12ce11fe79eadc

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 282d57c

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc

Commits:
- 282d57c Build vitaShaRK from source and pin vitaGL in the Vita image
- e03f214 Prune gh-pages reviews on PR close and daily
- b07fc28 Make the pages skill self-contained
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
- commit Build vitaShaRK from source and pin vitaGL in the Vita image

The title:

Rewrite AGENTS.md and keep only VitaDeck skills

The why:

The Matt Pocock skill suite is [no longer in use](source:s1), and AGENTS.md only pointed at it.

The what (medium):

AGENTS.md becomes a VitaDeck-specific runbook, the third-party skill suite and its setup docs leave the repo, the five kept skills get corrected, comprehende and a repo-owned pages skill with its publish script and prune workflow join the PR workflow, the JS workspace gets an oxfmt pass, and the Vita Docker image is repaired.

Look for:
- The [PR description](source:s1) says `skills-lock.json` is removed. At head it is modified: it lists only `comprehende` from `matemolnar8/comprehende`; `pages` is no longer tracked by the `skills` CLI.
- The [PR description](source:s1) does not say that the Dockerfile change is unrelated feature-wise to the AGENTS.md rewrite; it is here because the PR's own CI was red.
- The [Vita VPK check failed](source:s4) on this branch and on `main` at 1f305ea for the vitaGL/vitaShaRK mismatch; the Vita image fix part addresses it. Confirm the `vpk` job is green at head before merging.
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

### 06 Install comprehende, add a self-contained pages skill, wire both into the PR workflow (`comprehende-skills`)

`.agents/skills/comprehende/` is the upstream skill copied verbatim by `npx skills add` and recorded in `skills-lock.json`; `.agents/skills/pages/` is a repo-owned skill whose `scripts/pages-review.ts` publishes a folder onto this repo's `gh-pages` branch, `.github/workflows/pages-reviews.yml` runs its `prune` on PR close and daily, and AGENTS.md's Pull requests section calls the two skills.

[groups/comprehende-skills.md](groups/comprehende-skills.md)

### 07 oxfmt line wrapping (`oxfmt`)

Every hunk rejoins parameter lists, call arguments, JSX props, or a union type onto one line under oxfmt's print width, and `js/packages/sdk/README.md` loses a trailing blank line.

[groups/oxfmt.md](groups/oxfmt.md)

### 08 Import line reformats (`mechanical`)

One-line import reformats with no symbol change; skip them.

Depends on:
- 07 oxfmt line wrapping (`oxfmt`)

[groups/mechanical.md](groups/mechanical.md)

### 09 Vita image: build vitaShaRK from source, pin vitaGL (`vita-image`)

The Dockerfile clones vitaShaRK at df24065 and runs `make install` into `$VITASDK` before vitaGL, and vitaGL moves from `--depth=1` master to a checkout of 76f43e6.

[groups/vita-image.md](groups/vita-image.md)