Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify 96448b7b14c1ae72218b1b92b451d0ca6ed90434` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 96448b7b14c1ae72218b1b92b451d0ca6ed90434 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               96448b7b14c1ae72218b1b92b451d0ca6ed90434

Named refs at pin: 04760101 ... 96448b7b

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 96448b7b14c1ae72218b1b92b451d0ca6ed90434

Review concern 06 of 06: Docs: ADR 0004, glossary and READMEs (`docs`)

The why:

AGENTS.md asks that a change fix the ADR, CONTEXT.md and READMEs it makes inaccurate, in place.

The what:

ADR 0004's plugin section is rewritten for built-ins, Host Plugin Files and the two CLIs, CONTEXT.md adds **Host Plugin File**, and the companion, SDK and demo READMEs switch to the published install and the `--host-plugin` workflow.

Depends on:
- 05 Companion package, workspace and the demo app (`packaging`)

Hunk refs for this concern:
- docs/adr/0004-host-control-sliced-delivery.md
- CONTEXT.md
- AGENTS.md
- js/packages/host-companion/README.md
- js/packages/sdk/README.md
- js/examples/host-control/README.md