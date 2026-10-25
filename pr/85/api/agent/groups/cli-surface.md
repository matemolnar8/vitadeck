Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 3bf3a046b18aa26d98e7d70a63e9f3ea9b102fe5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 04: `--help`, `--version`, per-command flags, `create` guard (`cli-surface`)

The why:

[#51](source:s1) bundles these CLI basics with `dev`.

The what:

`main` short-circuits on `--help`/`--version`, validates flags against `COMMAND_FLAGS` per command, and `create` calls `isNonEmptyDir` unless `--force`; `cli.test.ts` checks each.

Look for:
- Breaking: flags are now checked per command, so `vitadeck build --force` or `vitadeck create my-app --dev` now fail with `Unknown flag(s)` where they were silently accepted before.
- Any argument starting with `-` is now a flag, and `-h`/`-v` win anywhere on the line: `vitadeck create -v` prints the version instead of scaffolding.

Hunk refs for this concern:
- js/packages/sdk/cli/cli.ts@374+376
- js/packages/sdk/cli/cli.ts@448+451
- js/packages/sdk/cli/cli.test.ts