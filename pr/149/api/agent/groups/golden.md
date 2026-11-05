Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 77464ae1945fca65d55985db4fc3f4aa3bc38bff` and `git rev-parse --verify 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  77464ae1945fca65d55985db4fc3f4aa3bc38bff

head               2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Named refs at pin: 77464ae1945fca65d55985db4fc3f4aa3bc38bff ... 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Read the diff:

git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Review concern 03 of 03: Shell Home golden (`golden`)

The why:

Máté asked for a Shell golden once Shell text went through the text pipeline, so a regression in Shell rendering fails smoke.

The what:

`smoke_harness.c` opens the Shell over the Failure Screen and screenshots it as `15-shell-home`, and `run_smoke_test.sh` seeds a Host Control Link named `Máté’s 日本 PC` so the golden covers Latin-1, punctuation and the CJK fallback font.

Depends on:
- 02 Shell draws through text_draw (`shell`)

Hunk refs for this concern:
- .agents/skills/vitadeck-smoke/SKILL.md
- tests/fixtures/golden/Linux/15-shell-home.png
- tests/run_smoke_test.sh
- tests/smoke_harness.c