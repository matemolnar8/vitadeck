Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify f51a30c0b95cc848c7d61f96228f74cebaf580e1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 f51a30c0b95cc848c7d61f96228f74cebaf580e1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               f51a30c0b95cc848c7d61f96228f74cebaf580e1

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... f51a30c

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 f51a30c0b95cc848c7d61f96228f74cebaf580e1

Review concern 04 of 04: Noto Sans KR download, smoke line and docs (`asset`)

Part: Korean fallback

The why:

The Korean font ships like the other runtime fonts: downloaded, pinned and hash-checked, with its licence tracked [#95](source:s2).

The what:

`cmake/fonts.cmake` fetches the font into `VITADECK_RUNTIME_FONTS`, which the VPK and `upload_vita` already loop over, while the smoke line, golden and glossary show 한글.

Depends on:
- 02 Lazy fallback fonts gated by Unicode block (`lazy`)

Hunk refs for this concern:
- cmake/fonts.cmake
- CMakeLists.txt
- assets/fonts/NotoSansKR-LICENSE.txt
- js/examples/smoke/src/App.tsx
- tests/fixtures/smoke_golden.Linux.png
- CONTEXT.md
- AGENTS.md
- .agents/skills/vitadeck-build/SKILL.md
- js/packages/sdk/README.md