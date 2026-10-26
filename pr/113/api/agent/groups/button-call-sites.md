Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3` and `git rev-parse --verify 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  27683e0ceec2ca0ab7c185c2e97d4f79787f16f3

head               584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Named refs at pin: 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 ... 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Read the diff:

git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 584319b0ab9f68fd731e2c56c4aef5fe85ed0a34

Review concern 05 of 05: Demo lists stay left-aligned; golden refresh (`button-call-sites`)

Part: Button align

The why:

The demo's section nav and Scroll row read as list items, and the smoke golden must match the new centred labels.

The what:

The demo nav and Scroll-section Buttons pass `align="left"`, and the Linux golden differs from the old one only inside the two smoke Button labels.

Depends on:
- 04 Button align end to end (`button-align`)

Hunk refs for this concern:
- js/examples/demo/src/App.tsx
- js/examples/demo/src/sections/ScrollSection.tsx
- tests/fixtures/smoke_golden.Linux.png