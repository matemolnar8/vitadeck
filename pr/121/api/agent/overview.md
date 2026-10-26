Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5` and `git rev-parse --verify cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c` in this repository.
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

base (merge-base)  5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5

head               cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c

Named refs at pin: origin/main ... chore/upgrade-pnpm-js-deps

Read the diff:

git diff --find-renames 5f198a8e3549afcc5c075a4abfc8c75df9d3d7b5 cd5c2fe1bbe4968dc8f987f23161d8369f2b2f0c

Commits:
- cd5c2fe fix(demo): satisfy oxlint 1.89 effect-dependency rule; refresh failure golden
- cb6841d Merge remote-tracking branch 'origin/main' into chore/upgrade-pnpm-js-deps
- 005abbd chore(js): stay on Babel 7
- 49e3d7c chore(js): Node 24, Babel 8, latest oxlint/oxfmt
- 97038f3 chore(js): upgrade pnpm to 12.15.0 and dependencies within major

Sources:
- transcript Claude Code session · Oct 26 Máté asks to move packageManager to the latest pnpm, adapt config to pnpm 11/12 defaults, keep Docker/CI pins in sync, bump deps within their major, and list skipped majors in the PR.
- transcript Claude Code session · Oct 26, follow-up Máté asks for Node 24 (latest LTS), latest oxlint and oxfmt, Node 24 types, Babel 8 if it can be made to work, and React kept on 18 because React 19 needs a reconciler rewrite.

The title:

chore(js): upgrade pnpm and dependencies

The why:

Machines with pnpm 12 fail `pnpm --dir js build` with `ERR_PNPM_BAD_PM_VERSION` because `packageManager` pins pnpm 10.28.2 [per the request](source:t1).

The what (small):

The JS toolchain moves to pnpm 12.15.0 and Node 24, the catalog takes the latest oxlint, oxfmt and tsgolint plus in-range bumps with React and Babel on their current majors, and the code the new lint rules flag is fixed.

## Review concerns

### 01 pnpm 12 and Node 24 (`toolchain`)

`packageManager` and the Dockerfile pin pnpm 12.15.0, `.nvmrc` says 24 and CI reads it through `node-version-file`, and `allowBuilds: { esbuild: false }` keeps pnpm 12's strict build check from failing the install.

[groups/toolchain.md](groups/toolchain.md)

### 02 Catalog bumps and the new lint findings (`deps`)

The catalog bumps oxlint, oxlint-tsgolint, oxfmt, `@types/node`, `@types/react`, rolldown and tsx, and the three files oxlint now flags drop a redundant cast or move a `setState` out of an effect.

[groups/deps.md](groups/deps.md)

### 03 Demo app effects and the failure golden (`demo-lint`)

`TimersSection`'s hide timer reads the `toast` it hides, `FetchSection`'s retry calls `requestRepo()` from the press handler, and the failure-screen golden picks up the stack locations those bundles now produce.

Depends on:
- 02 Catalog bumps and the new lint findings (`deps`)

[groups/demo-lint.md](groups/demo-lint.md)