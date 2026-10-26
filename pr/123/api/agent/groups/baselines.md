Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 37890a509945b80e7dbaeab3a5a87c58234b5e14` and `git rev-parse --verify 7be7157d78cab3a7b95cfa1b4f4f64424b107fb0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 7be7157d78cab3a7b95cfa1b4f4f64424b107fb0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  37890a509945b80e7dbaeab3a5a87c58234b5e14

head               7be7157d78cab3a7b95cfa1b4f4f64424b107fb0

Named refs at pin: 37890a509945b80e7dbaeab3a5a87c58234b5e14 ... t3/perf-benchmark-suite

Read the diff:

git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 7be7157d78cab3a7b95cfa1b4f4f64424b107fb0

Review concern 04 of 05: Recorded baselines (`baselines`)

Part: Benchmark Mode

The why:

The request asks for a baseline on the emulator and the device ([request](source:s2)).

The what:

Generated `result.json` and `meta.json` from one Vita3K run on an i9 MacBook Pro and one device run; skim the meta commit and machine, not the numbers.

Depends on:
- 03 Runner, report and docs (`tooling`)

Hunk refs for this concern:
- tests/bench/baselines/vita/meta.json
- tests/bench/baselines/vita/result.json
- tests/bench/baselines/vita3k/meta.json
- tests/bench/baselines/vita3k/result.json