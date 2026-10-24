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

Review concern 06 of 09: Install comprehende, add a self-contained pages skill, wire both into the PR workflow (`comprehende-skills`)

Part: Comprehende PR workflow

The why:

Máté asked for comprehende reviews hosted on GitHub Pages as part of every PR; this is the installed skill text and the lock entries that `skills` manages.

The what:

`.agents/skills/comprehende/` is the upstream skill copied verbatim by `npx skills add` and recorded in `skills-lock.json`; `.agents/skills/pages/` is a repo-owned skill whose `scripts/pages-review.ts` publishes a folder onto this repo's `gh-pages` branch, `.github/workflows/pages-reviews.yml` runs its `prune` on PR close and daily, and AGENTS.md's Pull requests section calls the two skills.

Look for:
- `pages-review.ts` is 728 lines of new TypeScript with no dependencies: it runs `git worktree add` on a temp dir for `gh-pages`, copies the site under `pr/<n>/` or `site/<slug>/`, writes `published.json` and a root `index.html`, commits, and pushes with up to five fetch-reset retries. Nothing in this repo typechecks or lints it; it runs with `node --experimental-strip-types`.
- Subtle: `ensureGitIdentity` sets `user.name`/`user.email` on the work tree when they are unset, and the worktree gets `commit.gpgsign false`. Commits on `gh-pages` are unsigned even when the main checkout signs.
- `pages-reviews.yml` checks out the default branch, so it only works once this PR merges; until then a closed PR keeps its `pr/<n>/` folder. It needs no install step because the script has no dependencies.

Hunk refs for this concern:
- .agents/skills/comprehende/SKILL.md
- .agents/skills/comprehende/references/example.md
- .agents/skills/comprehende/references/review.schema.json
- .agents/skills/pages/SKILL.md
- skills-lock.json
- AGENTS.md
- .agents/skills/pages/scripts/pages-review.ts
- .github/workflows/pages-reviews.yml