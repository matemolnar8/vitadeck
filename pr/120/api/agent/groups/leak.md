Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 538d2833a7caafed2a5042946bfa378f95553eb0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               538d2833a7caafed2a5042946bfa378f95553eb0

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0

Review concern 04 of 04: Free deleted raw text; consistent child links (`leak`)

Part: Raw text leak

The why:

The sanitized smoke run on the demo reported raw text leaks, then a use-after-free once raw text was freed; the user asked to [commit the fix too](source:s1).

The what:

`unlink_child` runs before every append, insert, remove and destroy, so `native_destroy_instance` can safely free its raw text children and clear its other children's `parent`.

Look for:
- Production code in a test PR: every raw text node React deleted used to stay allocated and registered, on the Vita too; it is now freed in `native_remove_child` or when its parent is destroyed.
- Behavior change: appending or inserting an already attached child now moves it, where before it stayed at its old index too and was listed twice.
- Subtle: `native_remove_child` ignores its `parentId` argument and unlinks from `child->parent`, or the root when `parent` is NULL.

Hunk refs for this concern:
- src/jslib/instance.c