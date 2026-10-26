Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 46e4bf3923487584c5fc6b0b43f586d245f9409d` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 46e4bf3923487584c5fc6b0b43f586d245f9409d -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               46e4bf3923487584c5fc6b0b43f586d245f9409d

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 46e4bf3923487584c5fc6b0b43f586d245f9409d

Review concern 02 of 03: scripts/vita3k.sh (`script`)

The why:

One command to build and run the Vita build in Vita3K during development ([session](source:s2)).

The what:

The script finds Vita3K, writes a config whose `pref-path` points at `~/.vitadeck/vita3k/fs`, stages the examples there, and passes the VPK to Vita3K, which installs and runs it.

Look for:
- Every launch rewrites `~/.vitadeck/vita3k/config.yml`, so settings changed in Vita3K's own UI are lost on the next run.
- Every launch replaces the staged example packages, so a Runtime Upload of an app with the same package name as an example (e.g. `timers.vdapp`) is overwritten by the local build on the next run.

Depends on:
- 01 Staging takes a data root (`staging`)

Hunk refs for this concern:
- scripts/vita3k.sh