Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5` in this repository.
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

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5

Commits:
- 3bf3a04 feat(cli): vitadeck dev watches, builds and uploads to a running Vita

Sources:
- ticket #51 Approved design: watch, rebuild with NODE_ENV=development, POST to /upload, print one line; --help, --version, create refuses non-empty dirs.
  https://github.com/matemolnar8/vitadeck/issues/51
- pr PR #85 Implements #51 with a node:http stub test; no end-to-end run against the real listener.
  https://github.com/matemolnar8/vitadeck/pull/85

The title:

feat(cli): vitadeck dev watches, builds and uploads to a running Vita

The why:

Authors had no edit-and-see loop on the Vita; [issue #51](source:s1) asks for a watch, build and upload command.

The what (medium):

Adds `vitadeck dev <runtime-upload-url>`, which rebuilds the `.vdapp.zip` on every save and POSTs it to the Runtime Upload listener. It also adds `--help`, `--version`, and a non-empty-directory guard on `create`.

Look for:
- [#51](source:s1) lists `outDir`, `.vitadeck` and `node_modules` as ignored; the watcher also ignores `.git`, so commits and checkouts do not trigger uploads.
- [#51](source:s1) reports a connection reset like a 409; the code also retries once after 500 ms before printing the busy line, which the issue does not ask for.
- No end-to-end run against the native listener: [PR #85](source:s2) relies on the stub test and a raw TCP stub that truncates a 409.

## Review concerns

### 01 `vitadeck dev` loop (`dev`)

`dev` watches the project with recursive `fs.watch`, calls the in-process `build()` (which now returns its output paths), and `postArchive` turns each HTTP reply or socket error into one log line; `dev.test.ts` drives it against a `node:http` stub.

[groups/dev.md](groups/dev.md)

### 02 `--help`, `--version`, per-command flags, `create` guard (`cli-surface`)

`main` short-circuits on `--help`/`--version`, validates flags against `COMMAND_FLAGS` per command, and `create` calls `isNonEmptyDir` unless `--force`; `cli.test.ts` checks each.

[groups/cli-surface.md](groups/cli-surface.md)

### 03 Shared test fixtures (`fixtures`)

The project fixture and `execFileText` move verbatim from `package-format.test.ts` into `test-project.ts`, and `test:cli` runs `cli/*.test.ts`; the move is safe to skim.

[groups/fixtures.md](groups/fixtures.md)

### 04 README and upload skill (`docs`)

The SDK README gains a "Develop on a Vita" section and a `create --force` line, and the `vitadeck-vdapp-upload` skill leads with `dev` and keeps the script and curl as the fallback.

[groups/docs.md](groups/docs.md)