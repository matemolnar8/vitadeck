Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 1f5d95410d4b49d25bbb16ce1fe7c806178b8bb4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 1f5d95410d4b49d25bbb16ce1fe7c806178b8bb4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               1f5d95410d4b49d25bbb16ce1fe7c806178b8bb4

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... refactor/native-globals-single-source

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 1f5d95410d4b49d25bbb16ce1fe7c806178b8bb4

Review concern 04 of 05: Host config without the unknown cast (`host-config`)

The why:

The cast hid missing `HostConfig` members, which surface only as `is not a function` on device ([#56](source:s1)).

The what:

`rawHostConfig` now `satisfies VitadeckHostConfig`, so the ten required members it lacked are implemented and `Reconciler(hostConfig)` needs no cast.

Look for:
- `scheduleTimeout` wraps `fn` in a void callback for the native `setTimeout`, and `getCurrentEventPriority` returns `DefaultEventPriority`; the root is created in legacy mode, so React does not ask for an event priority today.
- `hideInstance`/`unhideInstance` stay unimplemented: they are optional in the types and the host has no way to hide an instance, so a Suspense fallback still throws.

Hunk refs for this concern:
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts