Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify af68c987ab0099cbee319e81439cf2eb5c7a2064` in this repository.
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

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               af68c987ab0099cbee319e81439cf2eb5c7a2064

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... af68c987ab0099cbee319e81439cf2eb5c7a2064

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d af68c987ab0099cbee319e81439cf2eb5c7a2064

Commits:
- af68c98 fix(jslib): handle timers that clear themselves from their callback

Sources:
- ticket #40 After the call, re-look-up the id and only reschedule or free when the entry still exists; extend the timer harness.
  https://github.com/matemolnar8/vitadeck/issues/40
- pr PR #61 Dup func during the call, re-look-up after, stop reusing cleared ids; five new harness scenarios.
  https://github.com/matemolnar8/vitadeck/pull/61

The title:

fix(jslib): handle timers that clear themselves from their callback

The why:

A timer callback that clears its own id makes `run_timeouts` re-add an interval with a freed `JSValue` or double free a one-shot ([#40](source:s1)).

The what (small):

`run_timeouts` now holds its own reference to the callback during the call and re-looks-up the timer id afterwards, and `set_timeout_impl` no longer hands a just-cleared id back out.

## Review concerns

### 01 run_timeouts survives callbacks that mutate the timer map (`run-timeouts`)

`run_timeouts` calls a dup of the entry's `func`, then re-looks-up the id and reschedules in place or frees, and the harness drives self-clearing, sibling-clearing, and timer-creating callbacks through it.

[groups/run-timeouts.md](groups/run-timeouts.md)

### 02 Cleared ids are not reused immediately (`id-reuse`)

`set_timeout_impl` post-increments `timeout_id_counter`, so the next timer gets a fresh id even when the previous one was cleared.

Depends on:
- 01 run_timeouts survives callbacks that mutate the timer map (`run-timeouts`)

[groups/id-reuse.md](groups/id-reuse.md)