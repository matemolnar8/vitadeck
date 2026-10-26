# jimp.h

This directory vendors a snapshot of `jimp.h`, the immediate-mode JSON parser
prototype from `tsoding/jim`.

- Upstream: https://github.com/tsoding/jim
- Commit: `2890b452593c4a108d209db16d5a19c6ce5f9982`
- License: MIT, copied in `LICENSE`

Local patches: two Vita GCC warning fixes, recorded in `../PATCHES.md`.

VitaDeck uses this header only behind the native `package_manifest` module.
Do not expose `jimp` types through VitaDeck interfaces.
