---
name: vitadeck-smoke
description: Run VitaDeck's Dockerized Linux smoke test or refresh its CI golden screenshots. Use when changing rendering, theme, fonts, the demo Deck App, or when CI smoke fails.
metadata:
  internal: true
---

# VitaDeck smoke

The smoke harness boots the demo Deck App (`js/examples/demo`), presses R through every section, then presses its "Throw an error" Button, comparing a screenshot of each page against `tests/fixtures/golden/Linux/`. `tests/fixtures/demo_stubs.js` is prepended to the demo bundle to pin `Date`, `getTime()` and `fetch`, so the Timers and fetch pages are stable.

The Linux smoke path is single-source: local runs and GitHub Actions both use `scripts/smoke-docker.sh` against the `smoke` stage from `Dockerfile`. CI builds that stage with BuildKit's GitHub Actions cache, then runs the script with `VITADECK_SMOKE_NO_BUILD=1`.

## Test

From the repo root:

```sh
scripts/smoke-docker.sh test
```

Completion criterion: the script finishes with `ctest` passing and leaves the screenshots at `out-smoke/smoke-screenshots/`.

## Update Golden

After intentional visual changes:

```sh
scripts/smoke-docker.sh update-golden
```

Completion criterion: `tests/fixtures/golden/Linux/` is rewritten by the Dockerized Linux harness. Review the changed PNGs before committing. A new or renamed demo section also needs its entry in `PAGES` in `tests/smoke_harness.c`.

## Notes

- The script keeps Linux `node_modules` and the pnpm store in Docker volumes, so it does not replace the host install.
- Use `VITADECK_SMOKE_NO_BUILD=1` only after a separate cached image build has loaded `vitadeck-smoke:latest`.
