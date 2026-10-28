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

Review concern 01 of 04: vitadeck_core library and harnesses (`core`)

The why:

Every harness compiled its own copies of the sources: 289 objects from 60 unique files.

The what:

The app's sources minus main.c become one static library that the app and every harness link, which collapses the per-harness source lists into a foreach.

Look for:
- quickjs is linked next to vitadeck_core (VITADECK_CORE_LIBRARIES), not into it: with Unix Makefiles a link dependency would hold every core compile until the 10 s quickjs.c compile finishes.
- js_limits_vita_stack_harness compiles its own js_runtime.c and thread_posix.c with the Vita stack define; the static linker then never pulls those archive members. Same mechanism keeps upload_single_flight_harness's #include of http_server.c free of duplicate symbols.
- The big hunk also holds the JS custom command, the add_dependencies(vitadeck js) edge, the in-source guard, and the link_into_build symlinks; read it together with the js group.

Hunk refs for this concern:
- CMakeLists.txt@136+136
- CMakeLists.txt@157+153
- CMakeLists.txt@184+180
- CMakeLists.txt@236+230
- CMakeLists.txt@438+330