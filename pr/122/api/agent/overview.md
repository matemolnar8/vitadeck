Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 46e4bf3923487584c5fc6b0b43f586d245f9409d` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- 46e4bf3 feat(dev): run the Vita build in the Vita3K emulator

Sources:
- pr PR #122 Vita3K script, emulator skill, staging script takes the data root.
  https://github.com/matemolnar8/vitadeck/pull/122
- transcript Claude Code session · Oct 26 Máté asked for an emulator that runs VitaDeck on macOS and Windows, easy to run during development, then for the emulator usage as its own skill with docs for setting it up on another machine.

The title:

Run the Vita build in the Vita3K emulator

The why:

Testing the Vita build needed the device; this runs it in Vita3K on macOS, Windows and Linux ([PR #122](source:s1)).

The what (small):

`scripts/vita3k.sh` builds the VPK and runs it with the example Deck Apps in an isolated Vita3K setup, and a new `vitadeck-emulator` skill documents setting it up on a new machine.

Look for:
- Not tried on Windows: the Git Bash path conversion (`cygpath -m`) and the `winget` install line are untested ([PR #122](source:s1)).

## Review concerns

### 01 Staging takes a data root (`staging`)

`stage-example-apps.sh` now takes the data root, and the `stage_examples` CMake target passes `<build>/vitadeck-data`, so host staging lands in the same place as before.

[groups/staging.md](groups/staging.md)

### 02 scripts/vita3k.sh (`script`)

The script finds Vita3K, writes a config whose `pref-path` points at `~/.vitadeck/vita3k/fs`, stages the examples there, and passes the VPK to Vita3K, which installs and runs it.

Depends on:
- 01 Staging takes a data root (`staging`)

[groups/script.md](groups/script.md)

### 03 vitadeck-emulator skill (`docs`)

The new skill holds new machine setup, controls, agent driving tips and the known Vita3K gaps; `AGENTS.md` and `vitadeck-build` only point to it.

Depends on:
- 02 scripts/vita3k.sh (`script`)

[groups/docs.md](groups/docs.md)