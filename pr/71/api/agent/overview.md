Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 3ad801f013a9deced2cca7d5928b544f9aa18750` in this repository.
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

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               3ad801f013a9deced2cca7d5928b544f9aa18750

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... chore/docs-and-hygiene

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 3ad801f013a9deced2cca7d5928b544f9aa18750

Commits:
- 3ad801f docs(context): match the glossary to the shipped Shell and Runtime Upload
- c31472d refactor(sdk): delete the unused light theme
- 6536e27 chore(lint): make oxlint ignores consistent and lint chat scripts
- 460ca98 chore(examples): drop the empty @vitadeck/examples workspace package
- 088279f refactor(examples): drop manual memoization in minesweeper
- bc980ef docs(sdk): document vitadeck.config.json, assets, and generated types
- 86594d0 chore(docs): delete stray scroll-container explainer

Sources:
- ticket #59 Lists stale glossary lines, a stray HTML doc, missing SDK README topics, manual memoization, duplicated example configs, a lint ignore gap, and the dead light theme.
  https://github.com/matemolnar8/vitadeck/issues/59
- pr PR #71 One commit per issue item, with the glossary changes listed against src/.
  https://github.com/matemolnar8/vitadeck/pull/71

The title:

Fix stale glossary, document SDK config, prune dead theme and files

The why:

[#59](source:s1) reports that `CONTEXT.md` still calls Runtime Upload future work, that the SDK README omits config, assets and generated types, and that dead files, manual memoization, and a lint inconsistency remain.

The what (medium):

A hygiene pass over several independent stories: `CONTEXT.md` corrected against the shipped code, a new SDK README section on project configuration, removal of the light theme, a stray HTML doc, and an empty workspace package, memo-free minesweeper, and a consistent oxlint ignore list.

Look for:
- [#59](source:s1) names four identical example configs; the per-example `package.json`, `tsconfig.json` and `vitadeck-env.d.ts` stay because they already extend the SDK preset and mirror the author scaffold. Only `js/examples/package.json` is removed.
- [#59](source:s1) also asks the SDK README to cover build mode; this PR leaves the Build section alone because open PR #63 adds the `--dev` flag there.

## Review concerns

### 01 Glossary matches the shipped Shell and upload flow (`glossary`)

Each Shell, upload-listener, and input statement is rewritten in place to describe what `src/shell/shell.c`, `src/ui/input.c`, and `src/core/package_library.c` do, with no new terms.

[groups/glossary.md](groups/glossary.md)

### 02 SDK README documents project configuration (`sdk-readme`)

The new section restates the rules `readConfig`, `readFonts`, `readImages` and `writeFontTypes` in `cli/cli.ts` enforce, for authors.

[groups/sdk-readme.md](groups/sdk-readme.md)

### 03 Delete the unused light theme (`theme`)

`ThemeProvider` in `theme.tsx` serves one constant `Theme`, so `index.tsx` stops exporting `ThemeName` and the smoke fixture prints the literal `dark`.

[groups/theme.md](groups/theme.md)

### 04 Minesweeper without useMemo/useCallback (`minesweeper`)

Each memoized callback and derived value in `MinesweeperDeckApp` becomes a plain function or expression with the same body.

[groups/minesweeper.md](groups/minesweeper.md)

### 05 Drop the empty examples workspace package (`examples-workspace`)

Removing `js/examples/package.json` goes with removing its `examples` entry from `pnpm-workspace.yaml` and its lockfile importer.

[groups/examples-workspace.md](groups/examples-workspace.md)

### 06 Consistent oxlint ignores; lint chat scripts (`lint`)

The `**/rolldown*.config.*` ignore replaces two patterns, and a `scripts/*.mjs` override lets the chat script be linted, which forces its `import.meta.dirname` fix.

[groups/lint.md](groups/lint.md)

### 07 Delete scroll-container.html (`stray-doc`)

A 678-line AI-generated explainer from 7241e81 is deleted; nothing in the repo links to it, so the deletion is safe to skim.

[groups/stray-doc.md](groups/stray-doc.md)