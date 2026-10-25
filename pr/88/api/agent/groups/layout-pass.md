Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8c25053b52e85f60efa5027dffa62bae617dbee1` and `git rev-parse --verify 0f7b8aa7abe6171068e8a6195e17d7545a857733` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8c25053b52e85f60efa5027dffa62bae617dbee1

head               0f7b8aa7abe6171068e8a6195e17d7545a857733

Named refs at pin: 8c25053b52e85f60efa5027dffa62bae617dbee1 ... 0f7b8aa7abe6171068e8a6195e17d7545a857733

Read the diff:

git diff --find-renames 8c25053b52e85f60efa5027dffa62bae617dbee1 0f7b8aa7abe6171068e8a6195e17d7545a857733

Review concern 01 of 07: Layout pass and Stack instance type (`layout-pass`)

The why:

Enables the consumers: one function computes positions so render, hit testing and Focus agree.

The what:

`layout_tree` in `layout.c` fills the new `InstanceLayout` on `ReactInstance` from props, and `NT_STACK` plus a shared `StackProps` (which `NT_SCROLL` now also uses) feed its flow step.

Look for:
- Subtle: `layout_flow` reads each child's margin and offset from its layout before overwriting it, the main axis in the first pass and the cross axis in the second, so the order of the two loops matters.
- A Stack or Scroll with `width` or `height` 0 fits its content; a Scroll always receives a size from the SDK types, so only a Stack uses this.

Hunk refs for this concern:
- src/ui/layout.c
- src/ui/layout.h
- src/ui/instance_tree.h
- CMakeLists.txt@164+164
- CMakeLists.txt@283+284
- CMakeLists.txt@293+295
- CMakeLists.txt@354+355