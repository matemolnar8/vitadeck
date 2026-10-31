# Releasing

A release is a GitHub Release with `vitadeck.vpk` attached plus the matching `@vitadeck/sdk` and `@vitadeck/host-companion` on npm, all cut from one tag. The two packages share one version because the SDK scaffold (`create --host-plugin`) depends on the companion at its own version. Releases stay manual: [`release.yml`](../.github/workflows/release.yml) never creates tags.

1. Open a PR that bumps `version` in [`js/packages/sdk/package.json`](../js/packages/sdk/package.json) and [`js/packages/host-companion/package.json`](../js/packages/host-companion/package.json) to the same value, then merge it.
2. Tag the merge commit on `main` with the same bare version and push the tag:

   ```sh
   git tag 0.6.0 && git push origin 0.6.0
   ```

The workflow fails before building anything if the tag differs from either package version. Otherwise it builds the VPK (version `XX.YY` from the tag, patch dropped), publishes the companion and then the SDK with npm provenance, and creates the release with notes generated from the merged PR titles. While the major version is 0, the release is marked as a pre-release.

## One-time npm setup

npm authenticates the workflow with Trusted Publishing (OIDC), so the repo has no `NPM_TOKEN` secret. Each package needs a Trusted Publisher with owner `matemolnar8`, repository `vitadeck`, workflow filename `release.yml`, allowed to run `npm publish` (not only `npm stage publish`): on npmjs.com, open the package → Settings → Trusted Publisher → GitHub Actions, or run `npm trust github <package> --file release.yml --repo matemolnar8/vitadeck --allow-publish` (npm 11.15 or later, 2FA required).

A Trusted Publisher needs the package to exist, so for a new package `npm trust github` first publishes a `0.0.0-stage` placeholder; nothing is published by hand. `latest` points at that placeholder until the first release. A new Trusted Publisher must publish within 2 days, so add it shortly before pushing the tag.
