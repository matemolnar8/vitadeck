Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 9d671384dbb819d16682adc129c7e1e2ea8547a5` in this repository.
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

head               9d671384dbb819d16682adc129c7e1e2ea8547a5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... feat/release-automation

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 9d671384dbb819d16682adc129c7e1e2ea8547a5

Commits:
- 9d67138 ci: align buildx, build-push and upload-artifact versions across workflows
- ffaebc7 docs: how to cut a release
- b977b46 ci(release): tag-triggered VPK release and npm trusted publishing
- 0b42e8b build: QuickJS CONFIG_VERSION from the submodule, PSVITAIP from the environment
- d6b935d build(vita): pin vitasdk base image, SDL and raylib-5.5-vita

Sources:
- ticket #52 Approved proposal: release.yml on bare semver tags reusing vita-build.yml, npm trusted publishing, pre-release on 0.x, pinned Docker image and clones, QuickJS version from submodule, PSVITAIP from env.
  https://github.com/matemolnar8/vitadeck/issues/52
- transcript Orchestrator brief · Oct 25 Implement #52 as approved; fail early on tag/version mismatch; align action versions; document the one-time npm setup.

The title:

Tag-triggered VPK release and npm trusted publishing

The why:

GitHub Releases and npm publishes are cut by hand, and the Vita image floats on unpinned `latest` and branch heads ([#52](source:s1)).

The what (medium):

Pushing a bare semver tag now builds the VPK, publishes `@vitadeck/sdk` over npm Trusted Publishing and creates the GitHub Release. The Vita image is pinned, QuickJS `CONFIG_VERSION` comes from the submodule, and `PSVITAIP` moves to the environment.

Look for:
- Manual step: release publishing fails until Máté adds a Trusted Publisher for `release.yml` on npmjs.com under `@vitadeck/sdk` settings ([#52](source:s1)).
- Done differently: the proposal checks the tag inside `publish`; the code runs it in its own `check-version` job before `vpk`, so a mismatch fails before the Docker build ([brief](source:s2)).
- Not in the proposal: `upload-vdapp.sh` drops its CMake cache and `CMakeLists.txt` IP fallbacks, since `CMakeLists.txt` no longer holds an IP.

## Review concerns

### 01 Release workflow (`release`)

`release.yml` checks the tag against the SDK version, calls `vita-build.yml` through its new `workflow_call` with the tag as `version`, then publishes the packed SDK and attaches the downloaded VPK to `gh release create`.

[groups/release.md](groups/release.md)

### 02 VPK version from the tag (`vpk-version`)

`VITADECK_VERSION=0.6.0` becomes `VITA_VERSION` `00.06` and anything not `X.Y.Z` is a configure error; the `PSVITAIP` line in the same hunk belongs to the env group.

Depends on:
- 01 Release workflow (`release`)

[groups/vpk-version.md](groups/vpk-version.md)

### 03 Pinned Vita image (`pins`)

The Dockerfile pins the base image by digest and checks out fixed SDL and raylib-5.5-vita commits like vitaGL, and the GHA cache scope moves to `v3` so no layer from the floating image is reused.

[groups/pins.md](groups/pins.md)

### 04 QuickJS CONFIG_VERSION from the submodule (`quickjs`)

CMake reads the first line of `vendor/quickjs/VERSION` into `CONFIG_VERSION` and reconfigures when that file changes.

[groups/quickjs.md](groups/quickjs.md)

### 05 PSVITAIP from the environment (`psvitaip`)

`PSVITAIP` comes from `$ENV{PSVITAIP}` at configure time, a `require_psvitaip` target fails `upload_vpk` and `upload_vita` when it is empty, Compose passes it into the container, and the skills and `AGENTS.md` say how to set it.

[groups/psvitaip.md](groups/psvitaip.md)

### 06 Releasing doc (`docs`)

`docs/releasing.md` lists the version-bump PR, the tag push and the Trusted Publisher setup, and `AGENTS.md` links it.

[groups/docs.md](groups/docs.md)

### 07 Aligned action versions (`actions`)

The `smoke` job in `ci.yml` moves to `setup-buildx-action@v4`, `build-push-action@v7` and `upload-artifact@v6`, the versions `vita-build.yml` already uses; the reader can skim it.

[groups/actions.md](groups/actions.md)