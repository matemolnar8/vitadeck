# VitaDeck coding standards

Review a VitaDeck diff against these rules. They are judgment calls; formatting, types, lint and tests are `scripts/agent-check.sh`'s job. Every rule applied is the bar: a finding names the rule it breaks.

## Hit every surface

The common defect is a change that works on the host and breaks the Vita, or works in one example and breaks the SDK contract.

- **Targets.** Host (macOS/Linux) and Vita share `src/`. Anything touching threads, filesystem, networking, or GL belongs behind `src/platform/` or needs a Vita Docker build to prove it.
- **Author contract.** `@vitadeck/sdk` and `@vitadeck/host-companion` are published. A change to either package's exports, types, CLI flags, scaffold template, or manifest shape is a breaking change for authors. Bump the Deck App Package Schema Version when the manifest shape changes.
- **Manifest contract.** `src/core/package_manifest.c`, the SDK CLI that writes the manifest, and the `package_manifest` harness change together (footgun 2 in `AGENTS.md`).
- **Inputs.** Vita buttons and host keyboard mappings (F1 Start, Enter confirm, Backspace back) stay in sync in `src/ui/input.c` and in `CONTEXT.md`.
- **Reverse states.** Upload needs cancel, install needs removal, Shell open needs Shell close. Check the failure paint, not just success.

## Decisions and docs

- **ADRs.** Read the `docs/adr/` files for the areas the diff touches. A diff that contradicts one is a finding unless the same PR rewrites that ADR.
- **Docs in place.** If a change makes `CONTEXT.md`, an ADR, the SDK README, or a skill inaccurate, the PR fixes it in place, without appending a second account.
- **Glossary.** User-facing and Shell-facing names use the terms in `CONTEXT.md` and none of its "Avoid" words.
