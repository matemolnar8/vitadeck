Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 7539bc14c3481390b79fee499bc961194f4cd055` and `git rev-parse --verify d741b11fc23f7c1d6ffcad842e17f18c22c44f08` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 d741b11fc23f7c1d6ffcad842e17f18c22c44f08 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  7539bc14c3481390b79fee499bc961194f4cd055

head               d741b11fc23f7c1d6ffcad842e17f18c22c44f08

Named refs at pin: origin/main ... d741b11f

Read the diff:

git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 d741b11fc23f7c1d6ffcad842e17f18c22c44f08

Review concern 03 of 04: Ninja through CMake presets (`ninja`)

The why:

[Máté asked to use Ninja, then for presets](source:ninja) so the configure flags need no remembering.

The what:

`CMakePresets.json` names the Ninja configure (`default`) and its GCC variant (`linux`), and every script and CI job that configures a host build calls a preset; the smoke image installs `ninja-build`, and `run` joins the console pool.

Look for:
- `run` is `USES_TERMINAL`: without the console pool Ninja buffers the app's stdout until it exits.
- The Vita Docker build and vita-build.yml stay on Make; the vitasdk image has no Ninja.
- `smoke-docker.sh` passes `-B "$BUILD_DIR"` after `--preset linux`; the command-line build dir overrides the preset's `out/`.

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