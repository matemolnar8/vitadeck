Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 7539bc14c3481390b79fee499bc961194f4cd055` and `git rev-parse --verify da5356456e7647fa2d1b6538715d78881460edce` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 da5356456e7647fa2d1b6538715d78881460edce -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  7539bc14c3481390b79fee499bc961194f4cd055

head               da5356456e7647fa2d1b6538715d78881460edce

Named refs at pin: origin/main ... da535645

Read the diff:

git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 da5356456e7647fa2d1b6538715d78881460edce

Review concern 03 of 04: Ninja for host builds (`ninja`)

The why:

[Máté asked to use Ninja](source:ninja); it drops make 3.81's recursion overhead from the edit loop.

The what:

Every script and CI job that configures a host build passes `-G Ninja`, the smoke image installs `ninja-build`, and `run` joins the console pool.

Look for:
- `run` is `USES_TERMINAL`: without the console pool Ninja buffers the app's stdout until it exits.
- The Vita Docker build and vita-build.yml stay on Make; the vitasdk image has no Ninja.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

Hunk refs for this concern:
- .github/workflows/ci.yml
- Dockerfile
- scripts/agent-check.sh
- scripts/smoke-docker.sh
- scripts/bench.sh
- CMakeLists.txt@589+457