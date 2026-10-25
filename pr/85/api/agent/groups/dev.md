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

Review concern 01 of 04: `vitadeck dev` loop (`dev`)

The why:

The command [#51](source:s1) asks for: rebuild on save and upload to a running VitaDeck.

The what:

`dev` watches the project with recursive `fs.watch`, calls the in-process `build()` (which now returns its output paths), and `postArchive` turns each HTTP reply or socket error into one log line; `dev.test.ts` drives it against a `node:http` stub.

Look for:
- `build()` no longer prints `Built …`/`Wrote …`; the `build` command in `main` prints them from the returned paths, so `vitadeck build` output is unchanged and `dev` stays at one line per cycle.
- Subtle: `cycle()` reads `vitadeck.config.json` before `build()` so `outDir` is ignored before the first build writes to it; without that the first upload would trigger a second one.
- A 409, `ECONNRESET`, `EPIPE` or `UND_ERR_SOCKET` all count as busy; a truncated reply body is swallowed and only the status is used.

Hunk refs for this concern:
- js/packages/sdk/cli/cli.ts@1+1
- js/packages/sdk/cli/cli.ts@374+376
- js/packages/sdk/cli/dev.test.ts