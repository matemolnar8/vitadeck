Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a526711c80571b41bcfc9b420317b70597c23ede` and `git rev-parse --verify 2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c` in this repository.
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
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a526711c80571b41bcfc9b420317b70597c23ede

head               2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c

Named refs at pin: origin/main ... origin/t3/agent-check-node-sdk

Read the diff:

git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede 2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c

Commits:
- 2c2415d docs(emulator): the Vita build needs Node 24.16+
- 020c1b5 fix(scripts): agent-check also checks the engines.node minimum
- a526711 Merge pull request #136 from matemolnar8/t3/drop-pr-attribution-clause
- d6edb09 fix(scripts): agent-check runs on the pinned Node and builds JS before tsc
- aca107f docs(agents): drop the PR attribution clause

Sources:
- transcript Claude session · Oct 27 Retro on the unattended threads behind PRs #127–#135 lists six vitadeck guardrail fixes.
- pr PR #137 Node pin, JS build before tsc, docker compose docs.
  https://github.com/matemolnar8/vitadeck/pull/137

The title:

fix(scripts): agent-check runs on the pinned Node and builds JS before tsc

The why:

Fresh worktrees started on Node 22 with no built SDK and failed `tsc` until someone fixed them by hand, and docs called a `docker-compose` binary that is not installed [retro item 4](source:s1).

The what (small):

`scripts/agent-check.sh` reruns itself on the `.nvmrc` Node, installs, and builds JS before `tsc`; `pnpm install` fails on an old Node; docs say `docker compose`.

Look for:
- `.nvmrc` and `engines.node` already existed before this PR; the fix makes the check script and `pnpm install` enforce them rather than adding a new pin.

## Review concerns

### 01 Run on the pinned Node (`node`)

`use_pinned_node` checks the running Node against the `.nvmrc` major and the `engines.node` minimum before `fast`, `host` and `full`, and `engineStrict` makes `pnpm install` enforce `engines.node`.

[groups/node.md](groups/node.md)

### 02 Install and build JS before tsc (`build`)

`run_fast` and `run_host` call `install_js`, and `run_fast` runs `pnpm --dir js build` before the typecheck.

Depends on:
- 01 Run on the pinned Node (`node`)

[groups/build.md](groups/build.md)

### 03 docker compose, not docker-compose (`compose`)

The `vitadeck-build` skill and `AGENTS.md` switch every `docker-compose run` to `docker compose run`, and `run_vita` drops its fallback.

[groups/compose.md](groups/compose.md)