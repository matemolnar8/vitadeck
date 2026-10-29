Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 82350502701f2d0b032d12bca4aea02df9bf4dee` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 82350502701f2d0b032d12bca4aea02df9bf4dee -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               82350502701f2d0b032d12bca4aea02df9bf4dee

Named refs at pin: a447e3b9 ... 82350502

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 82350502701f2d0b032d12bca4aea02df9bf4dee

Review concern 08 of 09: host-control example Deck App (`demo`)

The why:

The [session](source:s5) asked for a demo Deck App showcasing Host Control.

The what:

`App.tsx` polls `linkedHosts()` and each host's built-in `systemStats.read`, imported from the SDK, lists the computers with live CPU, and shows the selected one's capabilities, gauges and an Echo round trip, plus the latest `core` notice it watches with `watchHostEvents`.

Look for:
- After a `timeout` the demo polls that computer every 30 s instead of every second, because the native fetch keeps running up to its 30 s curl cap after the SDK gives up at 5 s.

Depends on:
- 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

Hunk refs for this concern:
- js/examples/host-control/README.md
- js/examples/host-control/package.json
- js/examples/host-control/src/App.tsx
- js/examples/host-control/src/vitadeck-env.d.ts
- js/examples/host-control/tsconfig.json
- js/examples/host-control/vitadeck.config.json
- README.md