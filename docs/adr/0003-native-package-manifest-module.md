# Use a native Deck App Package Manifest module

Deck App Package Manifest parsing and validation should move into a native `package_manifest` module first, without trying to share package-format code with the TypeScript SDK yet. The module will use a vendored, locally patched snapshot of `tsoding/jim`'s `jimp.h` parser because its immediate-mode style matches the small manifest reader we want; `sheredom/json.h` remains the fallback if `jimp.h` becomes too costly to maintain.

The native module owns manifest validation for package name, `schemaVersion`, `entry`, Deck App Package Version, declared fonts, declared images, safe relative paths, supported asset extensions, and required package files. `package_library`, `font_registry`, and `image_registry` should call this module rather than carrying their own JSON scanners, while `bootstrap` stays a startup dedup module and does not coordinate package-format details.

**Considered options:** use one cross-language package-format source of truth now (rejected as too much tooling for the current need); preserve the hand-written JSON scanners (rejected because it keeps low locality); use `jsmn` (rejected because token walking is less readable for this module); use `sheredom/json.h` first (kept as backup because it compiled cleanly on Vita but is less aligned with the desired immediate-mode reader).

**Consequences:** vendor `jimp.h` under a subdirectory with provenance notes and the Vita warning fixes. Add a focused `package_manifest` CTest harness for manifest acceptance and rejection cases. Keep TypeScript SDK package-format cleanup as an explicit follow-up rather than mixing it into this native refactor.
