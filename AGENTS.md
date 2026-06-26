## Agent runbook

This is the shared instruction source for Codex, Cursor, and other repo-aware
agents. Prefer these commands over tool-specific rule files.

### Setup

- Use Node.js 22+ and pnpm from the `js/` workspace.
- Initialize submodules if `vendor/quickjs/quickjs.c` is missing:
  `git submodule update --init --recursive`.
- Install JavaScript dependencies once with `pnpm --dir js install`.
- Host native builds need CMake, raylib 5.5, curl, OpenSSL, and zlib. On macOS,
  install raylib with `brew install raylib`; on Linux, install raylib and the
  OpenGL/X11 development packages used by CI.
- Vita builds use Docker Compose. Start Docker Desktop before running the full
  check or any VPK task.

### Verification

Use `scripts/agent-check.sh` from the repo root.

- `scripts/agent-check.sh fast`: JS typecheck, JS lint, and C formatting check.
- `scripts/agent-check.sh host`: JS build, host CMake configure/build, and
  non-smoke CTest.
- `scripts/agent-check.sh smoke`: host build plus the visual smoke CTest.
- `scripts/agent-check.sh full`: fast, host, smoke, and Vita Docker build. If
  Docker is not running, this must fail with a clear Docker-daemon diagnostic.

Useful individual commands:

- JS build: `pnpm --dir js build`
- JS typecheck: `pnpm --dir js tsc`
- JS lint: `pnpm --dir js lint`
- JS format check: `pnpm --dir js format:check`
- C format check: `./scripts/clang-format.sh check`
- Host build: `cmake -S . -B out && cmake --build out`
- Host run: `cd out && ./vitadeck`
- Vita build:
  `docker-compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"`

On Linux/GCC, configure the host target with:

```sh
CC=gcc CXX=g++ cmake -S . -B out -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group"
```

### JavaScript and React

- Build the JS workspace before native builds after JS, TS, TSX, SDK CLI, or
  Deck App changes. Native CMake copies `js/dist/runtime` into the build tree.
- `@vitadeck/sdk` builds the `vitadeck` CLI at
  `js/packages/sdk/dist/cli.mjs`; after changing `js/packages/sdk/cli/`,
  `js/packages/sdk/cli/templates/`, or the CLI rolldown config, run the SDK
  build or the full JS build.
- Write idiomatic React for the React Compiler. Keep renders pure, follow the
  Rules of Hooks, do not mutate props/state/context, keep effect dependencies
  accurate, and avoid manual `useMemo`/`useCallback`/`memo` unless integrating
  with non-compiled code that needs referential equality.

### Native code style

- C sources under `src/` use `.clang-format`.
- Check formatting with `./scripts/clang-format.sh check`.
- Apply formatting with `./scripts/clang-format.sh format`.
- Run `cmake -S . -B out` so `out/compile_commands.json` exists for clangd.

### Issue tracker

GitHub Issues on `matemolnar8/vitadeck` are the triage surface. Use the `gh`
CLI for issue operations; external PRs are not a triage surface. See
`docs/agents/issue-tracker.md`.

### Triage labels

Canonical triage roles map 1:1 to GitHub label strings (`needs-triage`,
`needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`). See
`docs/agents/triage-labels.md`.

### Domain docs

This is a single-context repo. Read root `CONTEXT.md` and relevant ADRs in
`docs/adr/` before naming domain concepts or changing behavior. See
`docs/agents/domain.md`.
