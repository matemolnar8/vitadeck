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

Review concern 02 of 06: Companion: built-ins and loading Host Plugin Packages (`companion-loading`)

The why:

A globally installed companion has to load a plugin from a package folder rather than a package name, the package has to carry files besides code, and a rebuilt package has to replace the running one.

The what:

`loadHostPluginPackage` reads a `.vdplugin` folder's manifest and imports its entry by file URL with a content-hash query, `extractHostPluginArchive` unpacks a `.vdplugin.zip`, and the server's new `setPlugins` swaps every plugin but `core`, while `systemStats` becomes a built-in implementation beside `core`.

Look for:
- Subtle: `HostCommandError` is matched by `name` and a wire `code`, not `instanceof`, because each package bundles its own copy of the class; any thrown `Error` named `HostCommandError` with a wire code now maps to that code.
- The entry is imported as `<file URL>?v=<sha256 of the entry>`, so a rebuilt entry loads fresh, but a rebuild that changes only `files/` keeps the cached module; a plugin must read `files/` when a command runs.
- `extractHostPluginArchive` rejects archives with more than one top-level folder (after dropping `__MACOSX/`), backslashes, empty segments and `..`, and writes no file modes, so a shipped script loses its exec bit.
- Plugin ids must match `/^[A-Za-z0-9][A-Za-z0-9_-]{0,126}$/` at load; an id like `../x` is rejected because `install` names the folder after it.

Depends on:
- 01 SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

Hunk refs for this concern:
- js/packages/host-companion/src/plugin.ts
- js/packages/host-companion/src/server.ts
- js/packages/host-companion/src/core.ts
- js/packages/host-companion/src/system-stats.ts
- js/packages/host-companion/src/index.ts
- js/packages/host-companion/test/server.test.ts
- js/packages/host-companion/test/fixtures/greeter-plugin.ts
- js/packages/host-companion/test/fixtures/greeting.cjs
- js/packages/host-companion/test/fixtures/greeting.d.cts
- js/packages/host-companion/test/fixtures/plugin-package.ts