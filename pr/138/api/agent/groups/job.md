Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a526711c80571b41bcfc9b420317b70597c23ede` and `git rev-parse --verify a36879835bbd7c67c3b55491523c8dbaf305f373` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede a36879835bbd7c67c3b55491523c8dbaf305f373 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a526711c80571b41bcfc9b420317b70597c23ede

head               a36879835bbd7c67c3b55491523c8dbaf305f373

Named refs at pin: origin/main ... origin/t3/ci-c-format

Read the diff:

git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede a36879835bbd7c67c3b55491523c8dbaf305f373

Review concern 01 of 01: Pinned clang-format job (`job`)

The why:

CI lost its format step in [8cb20a5](source:s3) when clang-format was missing on the macOS runner.

The what:

The Ubuntu `c-format` job installs a pinned clang-format with `pipx` and runs the same script as the `fast` tier, whose not-found hint and `AGENTS.md` name that version.

Look for:
- A local clang-format of another major (MacPorts or Homebrew upgrade) can pass locally and fail this job, or the reverse; `AGENTS.md` now says to match 21.1.8.

Hunk refs for this concern:
- .github/workflows/ci.yml
- AGENTS.md
- scripts/clang-format.sh