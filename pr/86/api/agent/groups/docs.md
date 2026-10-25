Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 6ec5488d92569d22ca087ea754dd8d016282272b` and `git rev-parse --verify d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  6ec5488d92569d22ca087ea754dd8d016282272b

head               d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

Named refs at pin: 6ec5488 ... d3d67e5

Read the diff:

git diff --find-renames 6ec5488d92569d22ca087ea754dd8d016282272b d3d67e5780fccc6aeb1ca3cd56276f72dd1ceddc

Review concern 07 of 07: Glossary, host mappings and SDK README (`docs`)

The why:

AGENTS.md asks to keep CONTEXT.md, the Host mappings table and the SDK README accurate when input changes.

The what:

CONTEXT.md adds **Text Input**, its relationships and three host mapping rows, and the SDK README adds a Text input section; wording only.

Depends on:
- 05 SDK TextInput and runtime wiring (`js`)

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md