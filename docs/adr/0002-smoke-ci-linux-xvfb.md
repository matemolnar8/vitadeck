# Run smoke integration test on Linux CI with xvfb

The smoke harness drives the demo **Deck App** through `bootstrap`, steps through every section with Pad Button input, and compares a screenshot of each against a golden. GitHub Actions macOS runners cannot initialize an NSGL OpenGL context for raylib (`NSGL: Failed to find a suitable pixel format`), so the visual smoke test segfaults there even when the native build succeeds.

CI therefore splits host verification by platform: the macOS `build` job compiles the host target and runs `ctest -LE smoke` (unit harnesses only; the harnesses that open a window carry the `smoke` label). A separate Ubuntu `smoke` job builds the Linux smoke image with BuildKit's GitHub Actions cache, then runs `VITADECK_SMOKE_NO_BUILD=1 scripts/smoke-docker.sh test` so the cached image and local script path stay aligned.

Golden screenshots are per-platform (`tests/fixtures/golden/$(uname -s)/`) because font rasterization differs across OSes. `scripts/smoke-docker.sh update-golden` refreshes the committed Linux goldens in the same Docker image that CI uses.

**Considered options:** run smoke on macOS with hidden/minimized raylib windows (rejected — NSGL still fails on CI); drop visual regression and only assert the instance tree (rejected — loses render-path coverage); use a single cross-platform golden with high pixel tolerance (rejected — masks real regressions).

**Consequences:** do not re-enable the `smoke` label in the macOS `ctest` step without a working headless GL path. When updating goldens, use the Dockerized smoke runner and commit the matching `tests/fixtures/golden/Linux/`. The smoke job uploads `out-smoke/smoke-screenshots/` as a CI artifact for inspection.
