# Run smoke integration test on Linux CI with xvfb

The smoke harness drives a real **Deck App** through `deck_bootstrap`, raylib rendering, and golden screenshot comparison. GitHub Actions macOS runners cannot initialize an NSGL OpenGL context for raylib (`NSGL: Failed to find a suitable pixel format`), so the visual smoke test segfaults there even when the native build succeeds.

CI therefore splits host verification by platform: the macOS `build` job compiles the host target and runs `ctest -E smoke_harness` (unit harnesses only). A separate Ubuntu `smoke` job builds the host target and runs `ctest -R smoke_harness` under `xvfb-run`.

Golden screenshots are per-platform (`tests/fixtures/smoke_golden.$(uname -s).png`) because font rasterization differs across OSes. `tests/run_smoke_test.sh` bootstraps a missing golden on first local run; committed Linux goldens are the CI baseline for the smoke job.

**Considered options:** run smoke on macOS with hidden/minimized raylib windows (rejected — NSGL still fails on CI); drop visual regression and only assert the instance tree (rejected — loses render-path coverage); use a single cross-platform golden with high pixel tolerance (rejected — masks real regressions).

**Consequences:** do not re-enable `smoke_harness` in the macOS `ctest` step without a working headless GL path. When updating goldens, regenerate on the target platform and commit the matching `smoke_golden.*.png`. The smoke job uploads `out/smoke_screenshot.png` as a CI artifact for inspection.
