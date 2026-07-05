# jimp.h

This directory vendors a snapshot of `jimp.h`, the immediate-mode JSON parser
prototype from `tsoding/jim`.

- Upstream: https://github.com/tsoding/jim
- Commit: `2890b452593c4a108d209db16d5a19c6ce5f9982`
- License: MIT, copied in `LICENSE`

Local patches:

- Cast the `isspace` argument to `unsigned char` so Vita GCC does not warn about
  char subscripts.
- Print pointer differences with `%td` so Vita GCC does not warn about the
  diagnostic column format.

VitaDeck uses this header only behind the native `package_manifest` module.
Do not expose `jimp` types through VitaDeck interfaces.
