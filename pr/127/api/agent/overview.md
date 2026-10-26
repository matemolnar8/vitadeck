Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify e7d27d003d9cf91aaf9645382cb60facbc614bf0` in this repository.
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

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               e7d27d003d9cf91aaf9645382cb60facbc614bf0

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... HEAD

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0

Commits:
- e7d27d0 docs(text): describe the emoji cluster start without U+1F000
- c2bb5bd fix(text): classify emoji above U+1F000 by Emoji_Presentation
- a46f4ed style(tests): gate tests/ with clang-format and reformat once

Sources:
- ticket #119 Gate tests/ with clang-format and reformat once; classify emoji above U+1F000 by the Emoji property ranges like the table below it.
  https://github.com/matemolnar8/vitadeck/issues/119

The title:

Gate tests/ with clang-format; tighten the >= U+1F000 emoji classification

The why:

[`tests/*.c` has lines over 120 columns and drifts from the style](source:s1), and [every codepoint at or above U+1F000 counts as emoji, including Mahjong tiles and text-default symbols like U+1F170](source:s1).

The what (small):

Two follow-ups from #114: `scripts/clang-format.sh` now checks and formats `tests/`, with a one-time reformat; `vd_emoji_cluster` classifies codepoints above U+1F000 by Unicode 17 Emoji_Presentation instead of treating all of them as emoji.

Look for:
- Done differently: [the issue](source:s1) asks only to tighten the classification; the change also lets a following skin tone start a cluster, because without it fully qualified modifier sequences like U+1F575 U+1F3FB stop drawing as one emoji.
- CI runs no C format check; only `scripts/agent-check.sh fast` calls `scripts/clang-format.sh check`, so the `tests/` gate is local.

## Review concerns

### 01 Emoji_Presentation above U+1F000 (`classifier`)

`emoji_by_default` scans one sorted Emoji_Presentation table that now extends past U+1F000, and `vd_emoji_cluster` also starts a cluster when a skin tone follows, as the header comment now says.

[groups/classifier.md](groups/classifier.md)

### 02 Classification tests and Twemoji round-trip (`classifier-tests`)

New `expect_cluster` cases cover U+1F000, U+1F004, U+1F170 with and without U+FE0F, and skin-tone qualification, and the Twemoji round-trip stops inserting U+FE0F before a skin tone.

Depends on:
- 01 Emoji_Presentation above U+1F000 (`classifier`)

[groups/classifier-tests.md](groups/classifier-tests.md)

### 03 clang-format covers tests/ (`gate`)

Both `check` and `format` in `scripts/clang-format.sh` now find files under `src` and `tests`, and `AGENTS.md` and the `vitadeck-build` skill say the same.

[groups/gate.md](groups/gate.md)

### 04 One-time tests/ reformat (`reformat`)

Whitespace and line-wrap changes only from `clang-format -i`, in its own commit `a46f4ed`; the reader can skip them.

Depends on:
- 03 clang-format covers tests/ (`gate`)

[groups/reformat.md](groups/reformat.md)