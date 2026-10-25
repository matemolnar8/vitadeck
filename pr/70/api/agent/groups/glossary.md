Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d e8c1d20c9b00b0194e26f0edc30c4ea64a746bb5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 02: Glossary: Host Control, Host Companion, Host Control Link (`glossary`)

The why:

The ADR names three user-visible concepts that `CONTEXT.md` does not define yet.

The what:

The new Language entries define the three terms, and the new Relationships bullets tie them to the existing Runtime Upload and Upload Pairing terms without changing those entries.

Look for:
- **Host Companion** reuses the word "host", which `AGENTS.md` uses for the desktop build of VitaDeck; the entry disambiguates in its definition and lists "host build" under _Avoid_.

Depends on:
- 01 ADR 0004: slices, topology, parser, deferrals (`adr`)

Hunk refs for this concern:
- CONTEXT.md