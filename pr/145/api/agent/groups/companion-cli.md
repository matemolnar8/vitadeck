Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Named refs at pin: 04760101 ... a90a3cf6

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Review concern 03 of 06: Companion CLI: vitadeck-host-companion install, remove and hot reload (`companion-cli`)

The why:

This is the user's side of [simpler plugin loading](source:s1) and the [vitadeck-host-companion name](source:s1).

The what:

`start` loads the built-ins, every package in `~/.vitadeck/plugins/` and each `--plugin` package or folder of packages, watches those folders recursively and calls `setPlugins` after changes, while `install` stages an archive or folder in `.install-*`, validates it and renames it to `<id>.vdplugin`, and `remove` deletes that folder.

Look for:
- Paths resolve against `INIT_CWD` only when `npm_lifecycle_event` is `host-companion` (the repo's own script); a Deck App's `host` script and a global install resolve against the working directory.
- A `--plugin` package wins over an installed plugin with the same id, so an author can run a build of a plugin they also installed; a reload that throws logs `Kept the previous plugins` and keeps serving the old set.
- `--plugin` decides by name: a path ending in `.vdplugin` is one package, a `.vdplugin.zip` is refused with a pointer to `install`, and anything else is a folder of packages, which is how the scaffold's `--plugin ./dist` picks up a renamed app, answering [Codex's review](source:s3).
- An existing config with the old `plugins` field still loads; `readConfig` ignores the field, and `systemStats` is loaded as a built-in anyway.

Depends on:
- 02 Companion: built-ins and loading Host Plugin Packages (`companion-loading`)

Hunk refs for this concern:
- js/packages/host-companion/src/cli.ts
- js/packages/host-companion/src/config.ts
- js/packages/host-companion/src/link.ts
- js/packages/host-companion/test/cli.test.ts
- js/packages/host-companion/test/link.test.ts