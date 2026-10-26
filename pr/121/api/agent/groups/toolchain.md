Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify 005abbd672770ef8f10dec20a6a5c37c61282cd8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 005abbd672770ef8f10dec20a6a5c37c61282cd8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               005abbd672770ef8f10dec20a6a5c37c61282cd8

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... chore/upgrade-pnpm-js-deps

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 005abbd672770ef8f10dec20a6a5c37c61282cd8

Review concern 01 of 02: pnpm 12 and Node 24 (`toolchain`)

The why:

pnpm 12 is the version on the dev machines [per the request](source:t1), and the follow-up asks for Node 24 [per the follow-up](source:t2).

The what:

`packageManager` and the Dockerfile pin pnpm 12.15.0, `.nvmrc` says 24 and CI reads it through `node-version-file`, and `allowBuilds: { esbuild: false }` keeps pnpm 12's strict build check from failing the install.

Look for:
- Subtle: `esbuild: false` keeps pnpm 10's behaviour, where esbuild's postinstall was already skipped; without the entry pnpm 12 stops `pnpm install` with `ERR_PNPM_IGNORED_BUILDS`.
- Subtle: `engines.node` is `>=24.16.0` because Node 24.16 is the first Node 24 whose bundled corepack (0.35) can launch pnpm 11+; older corepack crashes looking for `bin/pnpm.cjs`.
- The SDK README and the deck-app-init skill still say Node 22+ on purpose: only this repo's toolchain moves to Node 24, not the SDK's author contract.

Hunk refs for this concern:
- js/package.json
- Dockerfile
- js/pnpm-workspace.yaml
- AGENTS.md
- .nvmrc
- .github/workflows/ci.yml
- .github/workflows/release.yml
- .github/workflows/vita-build.yml
- .agents/skills/vitadeck-build/SKILL.md