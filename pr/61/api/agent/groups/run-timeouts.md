Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify af68c987ab0099cbee319e81439cf2eb5c7a2064` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d af68c987ab0099cbee319e81439cf2eb5c7a2064 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               af68c987ab0099cbee319e81439cf2eb5c7a2064

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... af68c987ab0099cbee319e81439cf2eb5c7a2064

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d af68c987ab0099cbee319e81439cf2eb5c7a2064

Review concern 01 of 02: run_timeouts survives callbacks that mutate the timer map (`run-timeouts`)

The why:

The issue asks `run_timeouts` to reschedule or free only entries that still exist after the callback ([#40](source:s1)).

The what:

`run_timeouts` calls a dup of the entry's `func`, then re-looks-up the id and reschedules in place or frees, and the harness drives self-clearing, sibling-clearing, and timer-creating callbacks through it.

Look for:
- Subtle: the `JS_DupValue` around `JS_Call` is not in the issue; QuickJS `JS_Call` takes no reference, so a callback that clears itself would otherwise free its own running closure.
- Intervals are now rescheduled by writing `next_scheduled_at` through a pointer taken after the call, not by `hmput` of a pre-call copy, so no pointer into `timeout_hm` lives across the callback.
- On `main` the new harness aborts with `gc_decref_child: Assertion 'p->ref_count > 0' failed` on the self-clearing scenarios.

Hunk refs for this concern:
- src/jslib/timeout.c@99+99
- src/jslib/timeout.c@116+118
- tests/timer_reload_harness.c