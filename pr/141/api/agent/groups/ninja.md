Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 7539bc14c3481390b79fee499bc961194f4cd055` and `git rev-parse --verify ac07483a3197041d1727248664f137cdc6aedc8f` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 ac07483a3197041d1727248664f137cdc6aedc8f -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  7539bc14c3481390b79fee499bc961194f4cd055

head               ac07483a3197041d1727248664f137cdc6aedc8f

Named refs at pin: origin/main ... ac07483a

Read the diff:

git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 ac07483a3197041d1727248664f137cdc6aedc8f

Review concern 03 of 04: Ninja through CMake presets (`ninja`)

The why:

[Máté asked to use Ninja, then for presets](source:ninja) so the configure flags need no remembering.

The what:

`CMakePresets.json` names the one Ninja configure (`default`) that every script and CI job uses on macOS and Linux; the smoke image installs `ninja-build`, and `run` joins the console pool.

Look for:
- `run` is `USES_TERMINAL`: without the console pool Ninja buffers the app's stdout until it exits.
- The Vita Docker build and vita-build.yml stay on Make; the vitasdk image has no Ninja.
- `smoke-docker.sh` passes `-B "$BUILD_DIR"` after `--preset default`; the command-line build dir overrides the preset's `out/`.
- Linux loses `CC=gcc` and `-Wl,--start-group`: Ubuntu's `cc` is GCC, and the link resolves without the group.
- `CMakePresets.json` sets `cmakeMinimumRequired` 3.21 while `CMakeLists.txt` still accepts 3.14: only the preset path, not the Vita build, needs the newer CMake.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

Hunk refs for this concern:
- CMakePresets.json
- .github/workflows/ci.yml
- Dockerfile
- scripts/agent-check.sh
- scripts/smoke-docker.sh
- scripts/bench.sh
- CMakeLists.txt@589+457
- .gitignore