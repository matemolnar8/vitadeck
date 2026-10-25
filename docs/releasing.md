# Releasing

A release is a GitHub Release with `vitadeck.vpk` attached plus the matching `@vitadeck/sdk` on npm, both cut from one tag. Releases stay manual: [`release.yml`](../.github/workflows/release.yml) never creates tags.

1. Open a PR that bumps `version` in [`js/packages/sdk/package.json`](../js/packages/sdk/package.json), then merge it.
2. Tag the merge commit on `main` with the same bare version and push the tag:

   ```sh
   git tag 0.6.0 && git push origin 0.6.0
   ```

The workflow fails before building anything if the tag differs from the SDK version. Otherwise it builds the VPK (version `XX.YY` from the tag, patch dropped), publishes the SDK with npm provenance, and creates the release with notes generated from the merged PR titles. While the major version is 0, the release is marked as a pre-release.

## One-time npm setup

npm authenticates the workflow with Trusted Publishing (OIDC), so the repo has no `NPM_TOKEN` secret. On npmjs.com, open `@vitadeck/sdk` → Settings → Trusted Publisher → GitHub Actions, and enter owner `matemolnar8`, repository `vitadeck`, workflow filename `release.yml`.
