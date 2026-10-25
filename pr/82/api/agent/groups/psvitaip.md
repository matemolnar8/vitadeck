Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify 9d671384dbb819d16682adc129c7e1e2ea8547a5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 9d671384dbb819d16682adc129c7e1e2ea8547a5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               9d671384dbb819d16682adc129c7e1e2ea8547a5

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... feat/release-automation

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b 9d671384dbb819d16682adc129c7e1e2ea8547a5

Review concern 05 of 07: PSVITAIP from the environment (`psvitaip`)

The why:

A personal IP was baked into `CMakeLists.txt` ([#52](source:s1)).

The what:

`PSVITAIP` comes from `$ENV{PSVITAIP}` at configure time, a `require_psvitaip` target fails `upload_vpk` and `upload_vita` when it is empty, Compose passes it into the container, and the skills and `AGENTS.md` say how to set it.

Look for:
- Subtle: an existing `out-vita` with the old cached `PSVITAIP=192.168.1.177` no longer uploads; the plain `set()` shadows the cache entry, so `cmake --build out-vita --target run` fails with the unset message until `PSVITAIP` is exported and CMake re-runs.
- `upload-vdapp.sh` with no `--ip` and no `PSVITAIP` now exits 1 instead of reading `out-vita/CMakeCache.txt`.

Hunk refs for this concern:
- CMakeLists.txt@40+40
- CMakeLists.txt@384+397
- CMakeLists.txt@393+416
- CMakeLists.txt@415+439
- docker-compose.yml
- .agents/skills/vitadeck-build/SKILL.md
- .agents/skills/vitadeck-vdapp-upload/SKILL.md
- .agents/skills/vitadeck-vdapp-upload/scripts/upload-vdapp.sh
- AGENTS.md