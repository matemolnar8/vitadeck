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

Review concern 04 of 04: Docs, skills, and bench (`docs`)

The why:

AGENTS.md and the build and manual-testing skills described js_copy, js_build, and pnpm as a manual prerequisite of host builds.

The what:

The docs now say the host build reruns pnpm itself, is configured with `cmake --preset default`, and loops on `ninja -C out run`, while the Vita build still needs a manual pnpm build.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

Hunk refs for this concern:
- AGENTS.md
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-manual-testing/SKILL.md