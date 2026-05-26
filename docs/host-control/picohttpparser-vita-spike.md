# picohttpparser — Vita / desktop compile spike

Status: **complete** (May 2026). Parse-only layer evaluation for replacing hand-rolled HTTP parsing in `src/upload/http_server.c`.

Upstream: [h2o/picohttpparser](https://github.com/h2o/picohttpparser) (MIT, two C files, no runtime deps).

---

## Executive summary

| Target | Result | Notes |
|--------|--------|-------|
| Desktop Linux (gcc) | **PASS** | Compiles, links, runs; `phr_parse_request` returns expected method/path |
| Vita cross-compile (`arm-vita-eabi-gcc`) | **PASS** | Docker `vitasdk` image; see §3 |
| Source review (Vita safety) | **PASS** | No sockets/threads/files; SSE is optional; ARM uses portable scalar path |
| **Recommendation** | **Safe to adopt** | Drop-in parse layer; no patches required for Vita |

---

## 1. Files vendored

```
vendor/picohttpparser/
  picohttpparser.c   (707 lines, master @ May 2026)
  picohttpparser.h   (96 lines)
  test_picohttp.c    (spike harness only; optional to keep)
```

Fetched from upstream `master` via raw GitHub URLs.

---

## 2. Desktop smoke test — PASS

### Command

```sh
cd vendor/picohttpparser
gcc -Wall -Wextra -O2 -o test_picohttp test_picohttp.c picohttpparser.c
./test_picohttp
```

### Output

```
method=POST path=/upload version=1.1 headers=2 consumed=67
```

### Scalar-path sanity (simulates ARM / no SSE)

Vita builds will not define `__SSE4_2__`; the parser falls back to a portable byte loop. Verified on x86 with SSE disabled:

```sh
gcc -Wall -Wextra -O3 -mno-sse4.2 -D__vita__ -o test_picohttp_scalar test_picohttp.c picohttpparser.c
./test_picohttp_scalar
```

Same output as above.

---

## 3. Vita cross-compile — PASS

### Docker (project standard)

```sh
docker compose run --rm vitasdk bash -lc \
  'cd vendor/picohttpparser && arm-vita-eabi-gcc -Wall -Wextra -O3 -D__vita__ \
   -o test_picohttp_vita test_picohttp.c picohttpparser.c && arm-vita-eabi-size test_picohttp_vita'
```

**Result (May 2026):** compile and link succeeded.

```
   text    data     bss     dec     hex filename
  45720    2488  284932  333140   51554 test_picohttp_vita
```

The spike binary includes `test_picohttp` and newlib startup; linked parser-only `.o` in Vitadeck will be much smaller.

**Full Vitadeck Vita target** (after CMake integration):

```sh
docker compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
```

---

## 4. Source review — Vita compatibility

### Headers / libc usage

| Include | Purpose | Vita newlib |
|---------|---------|-------------|
| `<stdint.h>` | fixed-width types | OK |
| `<sys/types.h>` | `ssize_t` | OK (POSIX types in newlib) |
| `<stddef.h>` | `size_t` | OK |
| `<string.h>` | `memmove`, `strlen` (test only) | OK |
| `<assert.h>` | internal state guard in chunked decoder | OK (`NDEBUG` strips in release) |

**Not used:** `stdio.h` (parser), `unistd.h`, sockets, `pthread`, signals, `FILE*`, heap allocation inside parser.

### Architecture-specific code

| Feature | Condition | Vita impact |
|---------|-----------|-------------|
| SSE4.2 fast path (`_mm_cmpestri`) | `#if __SSE4_2__` | **Not compiled** on ARM; scalar loop used |
| `__builtin_expect` | GCC ≥ 3 | OK on `arm-vita-eabi-gcc` |
| `__attribute__((aligned(16)))` | non-MSVC | OK on ARM GCC |
| `uint64_t` in chunked decoder | always | OK on 32-bit ARM |
| x86-only intrinsics | guarded by `__SSE4_2__` | Never included on Vita |

No `#ifdef __x86_64__`, no 64-bit-only assumptions, no inline asm.

### Threading / I/O / network

The library is **pure parse/decode** over caller-supplied buffers. Vitadeck would keep existing BSD socket accept/recv/send in `http_server.c` and call `phr_parse_request` on accumulated bytes — same integration pattern as today, but with robust header parsing and partial-request handling (`last_len` / `-2` return).

### Chunked transfer decoder

`phr_decode_chunked` operates in-place on a buffer; uses `memmove` and `assert` for corrupt-state detection. Suitable for Vita if chunked bodies are needed later; upload path today uses `Content-Length` only.

### Warnings observed (desktop)

None with `-Wall -Wextra -O2` or `-O3`.

---

## 5. Integration notes for Vitadeck

Current manual parsing in `src/upload/http_server.c` (~585 LOC) uses `sscanf`, header accumulation, and `\r\n\r\n` scanning. picohttpparser would replace **request-line + header** parsing only:

```c
struct phr_header headers[32];
size_t num_headers = 32;
int r = phr_parse_request(buf, len, &method, &method_len, &path, &path_len,
                          &minor_version, headers, &num_headers, last_len);
/* r > 0: bytes consumed; -2: partial; -1: error */
```

Suggested CMake (when adopted):

```cmake
add_library(picohttpparser OBJECT
  vendor/picohttpparser/picohttpparser.c)
target_include_directories(picohttpparser PUBLIC vendor/picohttpparser)
# link OBJECT into vitadeck or list .c in VITADECK_SOURCES
```

No extra link libraries. Object size is small (~few KB `.o`); negligible vs QuickJS/raylib.

---

## 6. Recommendation

**Safe to adopt** as a parse-only layer for Host Control / upload HTTP work.

- No Vita-specific patches required.
- MIT license compatible with Vitadeck.
- Confirmed desktop compile + run and **Vita cross-compile via Docker**.
- **Adopted** for shared LAN HTTP refactor: vendored under `vendor/picohttpparser/`.
- Next step: wire `phr_parse_request` into the shared listener refactor, then run full `out/` and `out-vita/` builds.
