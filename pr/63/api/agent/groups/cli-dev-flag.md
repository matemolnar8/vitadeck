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

Review concern 02 of 03: `vitadeck build` defaults to production, `--dev` opts out (`cli-dev-flag`)

The why:

Author builds defaulted to `NODE_ENV=development`, so shipped `.vdapp`s ran React dev mode on the Vita ([#57](source:s1)).

The what:

`main` in `cli.ts` accepts `--dev` and passes it as `BuildOptions.dev`, which picks the `process.env.NODE_ENV` define; the new tests build a fixture in both modes and check the inlined string.

Look for:
- Breaking: the CLI no longer reads `NODE_ENV` from the environment, so an author running `NODE_ENV=development vitadeck build` now gets a production bundle; only `--dev` gives development.

Hunk refs for this concern:
- js/packages/sdk/cli/cli.ts
- js/packages/sdk/cli/package-format.test.ts
- js/packages/sdk/README.md
- .agents/skills/vitadeck-deck-app-init/SKILL.md