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

Review concern 04 of 08: domain.md deleted in both copies (`mirrors-1`)

Part: Agents runbook

The why:

`docs/agents/domain.md` was a copy the setup skill wrote from its own `domain.md`; both go.

The what:

The same deletion twice; read neither.

Depends on:
- 03 Delete the third-party skills and their setup docs (`skill-removal`)

Hunk refs for this concern:
- .agents/skills/setup-matt-pocock-skills/domain.md
- docs/agents/domain.md