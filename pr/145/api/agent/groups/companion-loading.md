Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify c0498393dd5911d602c63433160a3a7fa020e308` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 c0498393dd5911d602c63433160a3a7fa020e308 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               c0498393dd5911d602c63433160a3a7fa020e308

Named refs at pin: 04760101 ... c0498393

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 c0498393dd5911d602c63433160a3a7fa020e308

Review concern 02 of 06: Companion: built-ins and loading Host Plugin Files (`companion-loading`)

The why:

A globally installed companion has to load a plugin from a file rather than a package name, and a rebuilt file has to replace the running one.

The what:

`loadHostPluginFile` imports a `.vdplugin` through a content-hashed `.mjs` copy and checks the default export's shape, and the server's new `setPlugins` swaps every plugin but `core`, while `systemStats` becomes a built-in implementation beside `core`.

Look for:
- Subtle: `HostCommandError` is matched by `name` and a wire `code`, not `instanceof`, because each `.vdplugin` bundles its own copy of the class; any thrown `Error` named `HostCommandError` with a wire code now maps to that code.
- Each distinct plugin build leaves a `<sha256>.mjs` in `os.tmpdir()/vitadeck-host-companion/`; the copy is what Node imports, since it rejects the `.vdplugin` extension and bundled CommonJS needs a file URL for `createRequire`.
- Plugin ids must match `/^[A-Za-z0-9][A-Za-z0-9_-]{0,126}$/` at load; an id like `../x` is rejected because `install` names the file after it.

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
- js/packages/host-companion/test/fixtures/plugin-file.ts