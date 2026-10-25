Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5` in this repository.
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

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5

Named refs at pin: origin/main ... HEAD

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5

Commits:
- e8c1d20 docs(adr): host control sliced delivery and topology

Sources:
- ticket #45 Close #16, land host control as contract, companion, and native-link PRs, and write an ADR on the second HTTP parser.
  https://github.com/matemolnar8/vitadeck/issues/45

The title:

docs(adr): host control sliced delivery and topology

The why:

Draft PR #16 grew too large to rebase, and [vendoring a second HTTP parser alongside the hand-rolled upload server needs an ADR](source:s1) before the native slice starts.

The what (small):

ADR 0004 records how Host Control lands as three PRs, recommends a link-once topology where the Vita calls the companion, and keeps one HTTP parser; `CONTEXT.md` gains three glossary terms that the ADR uses.

Look for:
- [#45](source:s1) asks to close #5 in favour of #45; this PR does not close #5, and #45 stays open until Máté confirms the recommendations.
- [#45](source:s1) asks to fold the useful #16 design docs into the ADR; the ADR summarises their conclusions in prose instead of copying their tables, so details like port defaults and the long-poll variant live only on the closed branch.
- The topology section is a recommendation pending confirmation, not a settled decision; the ADR says to rewrite it together with the HTTP parsing section if option B wins.

## Review concerns

### 01 ADR 0004: slices, topology, parser, deferrals (`adr`)

The ADR's topology choice (Vita calls the companion over existing `fetch()`) is what lets its parser section reject picohttpparser and its delivery section keep slice (c) small.

[groups/adr.md](groups/adr.md)

### 02 Glossary: Host Control, Host Companion, Host Control Link (`glossary`)

The new Language entries define the three terms, and the new Relationships bullets tie them to the existing Runtime Upload and Upload Pairing terms without changing those entries.

Depends on:
- 01 ADR 0004: slices, topology, parser, deferrals (`adr`)

[groups/glossary.md](groups/glossary.md)