---
name: vitadeck-smoke
description: Run VitaDeck's Dockerized Linux smoke test or refresh its CI golden screenshot. Use when changing rendering, theme, Deck App output, smoke fixtures, or when CI smoke fails.
metadata:
  internal: true
---

# VitaDeck smoke

The Linux smoke path is single-source: local runs and GitHub Actions both use `scripts/smoke-docker.sh` against the `smoke` stage from `Dockerfile`. CI builds that stage with BuildKit's GitHub Actions cache, then runs the script with `VITADECK_SMOKE_NO_BUILD=1`.

## Test

From the repo root:

```sh
scripts/smoke-docker.sh test
```

Completion criterion: the script finishes with `ctest` passing and leaves the inspected screenshot at `out-smoke/smoke_screenshot.png`.

## Update Golden

After intentional visual changes:

```sh
scripts/smoke-docker.sh update-golden
```

Completion criterion: `tests/fixtures/smoke_golden.Linux.png` is updated by the Dockerized Linux harness. Review the resulting PNG before committing.

## Notes

- The script keeps Linux `node_modules` and the pnpm store in Docker volumes, so it does not replace the host install.
- Use `VITADECK_SMOKE_NO_BUILD=1` only after a separate cached image build has loaded `vitadeck-smoke:latest`.
