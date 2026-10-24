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

Review concern 06 of 08: Install comprehende and pages, wire them into the PR workflow (`comprehende-skills`)

Part: Comprehende PR workflow

The why:

Máté asked for comprehende reviews hosted on GitHub Pages as part of every PR; this is the installed skill text and the lock entries that `skills` manages.

The what:

`.agents/skills/comprehende/` is the upstream skill copied verbatim by `npx skills add`, `.agents/skills/pages/SKILL.md` is the upstream pages skill edited to publish from a comprehende checkout with `--repo` pointing here, and `skills-lock.json` records both; AGENTS.md's Pull requests section calls them.

Look for:
- Subtle: `pages/SKILL.md` differs from the upstream file its `skills-lock.json` hash describes. `npx skills update` would overwrite the `--repo` instructions with the comprehende-local wording.
- The pages skill depends on a local checkout of `matemolnar8/comprehende` with `pnpm install` done; nothing in this repo runs the publish or the gh-pages prune workflow.

Hunk refs for this concern:
- .agents/skills/comprehende/SKILL.md
- .agents/skills/comprehende/references/example.md
- .agents/skills/comprehende/references/review.schema.json
- .agents/skills/pages/SKILL.md
- skills-lock.json
- AGENTS.md