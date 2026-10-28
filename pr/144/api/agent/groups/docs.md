Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify eb61ca83421e50b68494e84294baac13b7c346d1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               eb61ca83421e50b68494e84294baac13b7c346d1

Named refs at pin: a447e3b9 ... HEAD

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f eb61ca83421e50b68494e84294baac13b7c346d1

Review concern 08 of 08: Glossary, ADR and checks (`docs`)

The why:

Docs that the new design made inaccurate are fixed in place, and the companion tests join the `fast` tier ([#110](source:s2)).

The what:

`CONTEXT.md` adds the Host Control Plugin, Link Listener and Shell terms, ADR 0004 is rewritten for plugins and several links, `AGENTS.md` lists the new code, and `agent-check.sh fast` runs `test:host-companion`.

Hunk refs for this concern:
- CONTEXT.md
- docs/adr/0004-host-control-sliced-delivery.md
- AGENTS.md
- scripts/agent-check.sh