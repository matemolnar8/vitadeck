Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify e03f214f037ea36a7095a5c1e5722284ee6df983` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 e03f214f037ea36a7095a5c1e5722284ee6df983 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               e03f214f037ea36a7095a5c1e5722284ee6df983

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... e03f214

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 e03f214f037ea36a7095a5c1e5722284ee6df983

Review concern 01 of 08: AGENTS.md rewrite (`agents-md`)

Part: Agents runbook

The why:

The old file was a generic command list plus pointers to the removed skills. The [PR](source:s1) replaces it with how VitaDeck works, where code lives, footguns, surfaces to check, and verification tiers.

The what:

AGENTS.md is one rewritten file; its Pull requests section now asks for the comprehende review that the Comprehende PR workflow part installs.

Look for:
- The How it works paragraph now says a plain `cmake --build out` copies the runtime only and that `stage_examples` (or `run`) stages the examples, resolving [the Codex comment](source:s2).
- The Individual commands line lists `pnpm --dir js tsc`, `lint`, `format:check`, and `build` as four full commands instead of a `|`-joined shorthand, resolving [the Codex comment](source:s3).
- The Footguns section claims `js/examples/chat/src/genesys/genesys.local.ts` is gitignored and holds credentials. Check `.gitignore` before trusting the never-commit rule.

Hunk refs for this concern:
- AGENTS.md