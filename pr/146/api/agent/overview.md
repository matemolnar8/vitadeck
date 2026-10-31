Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 07001bbdac339a0767b626376cda96d8530a1d37` and `git rev-parse --verify cf5576e78233c37c40a10851974f14c349e764e5` in this repository.
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

base (merge-base)  07001bbdac339a0767b626376cda96d8530a1d37

head               cf5576e78233c37c40a10851974f14c349e764e5

Named refs at pin: 07001bbdac339a0767b626376cda96d8530a1d37 ... HEAD

Read the diff:

git diff --find-renames 07001bbdac339a0767b626376cda96d8530a1d37 cf5576e78233c37c40a10851974f14c349e764e5

Commits:
- cf5576e7 build(release): publish @vitadeck/host-companion with the SDK

Sources:
- pr PR #146 Make the companion public, version it with the SDK, publish both from one tag.
  https://github.com/matemolnar8/vitadeck/pull/146
- transcript Claude Code session · Oct 31 Máté asks to ship the companion to npm with the SDK, leaning toward lockstep versions, with no tag push or npm publish by the agent.

The title:

Publish @vitadeck/host-companion with the SDK so scaffolded host plugins install

The why:

The SDK scaffold for `create --host-plugin` and `--host-plugin-only` adds a devDependency on `@vitadeck/host-companion`, which is private, so [`npm install` in a scaffolded project fails with a 404](source:s1).

The what (small):

The companion becomes a public npm package at the SDK's version, and the release workflow checks both versions against the tag and publishes the companion before the SDK.

Look for:
- Manual step: npm accepts a Trusted Publisher only for a package that exists, so Máté publishes the companion's first version by hand and then adds its Trusted Publisher, as [the PR says](source:s1).

## Review concerns

### 01 Publishable companion package (`package`)

`package.json` drops `private` and moves to the SDK's 0.5.0, and the new `cli.test.ts` case fails when the two versions differ.

[groups/package.md](groups/package.md)

### 02 Release workflow and runbook (`release`)

`release.yml` checks the tag against both manifests and publishes the companion before the SDK, and `docs/releasing.md` describes the same flow.

Depends on:
- 01 Publishable companion package (`package`)

[groups/release.md](groups/release.md)

### 03 Docs stop calling the companion unpublished (`docs`)

Each doc drops its "not on npm yet" note, ADR 0004 moves Publishing from Deferred into the lockstep decision, and the companion README switches to absolute links that work on npmjs.com.

Depends on:
- 01 Publishable companion package (`package`)

[groups/docs.md](groups/docs.md)