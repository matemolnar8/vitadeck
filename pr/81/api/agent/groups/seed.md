Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b ad548e1b0ea1602e4dd8ba2ce1203b20e39ef1fe

Review concern 03 of 04: stb_ds stops writing its global seed (`seed`)

Part: Hash seed race

The why:

[Both threads write `stbds_hash_seed` when creating maps](source:s2).

The what:

The vendored `stbds_make_hash_index` copies the global seed into each new table and no longer advances it, so after startup the global is only read.

Look for:
- Every hash map now starts with the same seed `0x31415926`; hashing stays consistent because each table already stores its own `t->seed`.

Hunk refs for this concern:
- vendor/stb_ds.h