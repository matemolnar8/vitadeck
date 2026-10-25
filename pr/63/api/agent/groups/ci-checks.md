Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 12bf1a46251dc1b0ba311c85fd44f69530f780f2` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 12bf1a46251dc1b0ba311c85fd44f69530f780f2 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               12bf1a46251dc1b0ba311c85fd44f69530f780f2

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... chore/ci-tests-and-build-mode

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 12bf1a46251dc1b0ba311c85fd44f69530f780f2

Review concern 01 of 03: JS checks in CI and agent-check fast (`ci-checks`)

The why:

`test:cli`, `lint` and `format:check` ran nowhere ([#57](source:s1)).

The what:

The CI `build` job and `run_fast` in `scripts/agent-check.sh` call the same three `pnpm --dir js` scripts, and the new root `test:cli` script forwards to `@vitadeck/sdk`.

Look for:
- Subtle: root `lint` and `lint:fix` now run `examples/chat/scripts/ensure-genesys-local.mjs` first, which copies `genesys.local.example.ts` to the gitignored `genesys.local.ts` when it is missing; without it, `lint` fails on a fresh checkout with no-unsafe-* errors in `examples/chat`.

Hunk refs for this concern:
- .github/workflows/ci.yml
- scripts/agent-check.sh
- AGENTS.md
- js/package.json