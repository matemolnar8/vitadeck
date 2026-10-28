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

Review concern 02 of 04: JS as a dependency, links instead of copies (`js`)

The why:

pnpm build either always ran (make js) or never ran (make run), and js_copy copied 25 MB of assets on every build.

The what:

The removed js_build, js_copy, and nested js targets are replaced by a js target over a custom command that vitadeck, stage_examples, run, and the smoke harnesses depend on on the host.

Look for:
- cmake_minimum_required moves to 3.14 for file(CREATE_LINK) and CONFIGURE_DEPENDS.
- The Vita branch only loses its js_copy dependency; it never runs pnpm and packs js/dist as before.

Depends on:
- 01 vitadeck_core library and harnesses (`core`)

Hunk refs for this concern:
- CMakeLists.txt@1+1
- CMakeLists.txt@458+350
- CMakeLists.txt@489+359
- CMakeLists.txt@589+457