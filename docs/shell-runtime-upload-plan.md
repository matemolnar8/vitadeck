# VitaDeck Shell and Runtime Upload Plan

## Goal

Iteration 3 adds a **VitaDeck Shell** for on-device management: a persistent **Installed Deck App Library**, choosing the **Active Deck App**, **Runtime Upload** over HTTP while a dedicated upload mode is open, and **System Input** to move between the **Deck App** and the Shell. Package shape and build flow remain as in [Deck App Packages Plan](./deck-app-packages-plan.md).

Authoritative vocabulary and edge-case decisions live in [CONTEXT.md](../CONTEXT.md); this plan is the iteration scope and acceptance checklist.

## Prerequisites (Iteration 2)

- **Deck App Package Directory** (`.vdapp`) with **Deck App Package Manifest** (`manifest.json`, `schemaVersion`, `name`, `entry`, **`version`** for display).
- VitaDeck loads **`runtime.js`** and the selected package separately.
- SDK produces zip-suitable package trees for later transport.

## Iteration 3 Choices

### Shell and library

- **Installed Deck App Library** holds multiple packages; entries persist across VitaDeck restarts until **Installed Deck App Removal**.
- **Shell Home Screen** lists installed apps with **Deck App Package Version**, sorted alphabetically by manifest **`name`** (case-insensitive), tie-break **Deck App Package Name**.
- **Runtime Upload** is a **primary** **Shell Home** action via **Shell Upload Entry** (normal **Shell Face Input**, not a hidden or Deck App–hosted flow).
- Changing the **Active Deck App** at runtime triggers a **Deck App Runtime Restart** (fresh JS context). **Runtime Upload** does not auto-activate; activation stays explicit.
- **Installed Deck App Removal** cannot target the **Active Deck App**. **Runtime Upload Replacement** may overwrite the active package in place, then **Deck App Runtime Restart**.

### System Input

- **Start Input** (Vita **Start**; macOS **Host Start Mapping** F1) toggles between **Deck App** and **VitaDeck Shell** except on **Shell Upload Screen**, where **Start** does not dismiss upload mode.
- **Shell Face Input**: Cross confirms; Circle backs or confirms **Shell Upload Cancel** when that control is focused.
- macOS: **Host Shell Confirm Mapping** (Enter) and **Host Shell Back Mapping** (Backspace) apply only while the Shell has focus. **Shell Upload Cancel** stays on-screen only at first (no required keyboard shortcut).

### Runtime Upload listener

- **Runtime Upload Listener** runs only while **Shell Upload Screen** is open and binding succeeded; otherwise no HTTP server for upload.
- Bind **all interfaces**; default TCP port **8787**, try up to **10** consecutive ports. **Runtime Upload URL** on the device reflects the bound **IP:PORT**.
- If no port binds: **Shell Upload Screen** shows bind failure, no URL, **Shell Upload Cancel** returns to **Shell Home** (no mandatory in-screen retry).

### Runtime Upload HTTP contract

- Documented surface: **`GET /`** (**Runtime Upload Web UI**) plus same-origin static assets the page needs; **`POST /upload`** only for ingest. Other paths **404**; non-**POST** on **`/upload`** **405**. No health/version HTTP endpoint in this iteration.
- **`POST /upload`**: `multipart/form-data` with file field **`archive`**, or raw body **`application/zip`**. Responses always JSON (`Content-Type: application/json`).
  - Success **200**: `ok: true`, `packageName` (**Deck App Package Name**), `version` (**Deck App Package Version**).
  - Failure: `ok: false`, `code`, `message`; typical statuses **400**, **409** (**Runtime Upload Single-Flight**), **413**, **415**, **422**.
- **Runtime Upload Single-Flight**: one ingest pipeline at a time (receive → staging → validate → publish or fail); overlapping **POST** → **409**.

### Package ingest

- **Runtime Upload Archive**: zip with exactly one top-level `.vdapp` directory; layout and limits per **CONTEXT.md** (**Runtime Upload Limits**: 16MB uploaded, 64MB unpacked, 256 entries initially).
- **Runtime Upload Staging** for unpack/validate; publish into library when validation succeeds; **Runtime Upload Replacement** when **Deck App Package Name** matches an existing install.
- Successful publish closes **Shell Upload Screen**, stops listener, returns **Render Surface** to **Deck App**. Failed validation keeps upload screen open for retry. **Shell Upload Cancel** stops listener and returns to **Shell Home** without closing Shell.
- Initial trust model: LAN only; **Upload Pairing** is future optional hardening.

## Runtime Upload Archive

Same logical contents as a built **Deck App Package Directory**, wrapped for transport:

```text
example-archive.zip
  my-deck-app.vdapp/
    manifest.json
    app.js
    …
```

Exactly one top-level `*.vdapp` directory; no other top-level entries.

## Native / host notes

- Exact on-disk layout for multiple packages and staging is implementation-defined; behavior must match **CONTEXT.md** (persistence, atomic publish when practical, fail closed on limits/validation).
- Vita packaging should mirror the same logical capabilities as the desktop target where feasible.

## Not In Iteration 3

- **Upload Pairing** or other upload authentication.
- Health/readiness HTTP endpoints beyond the **Runtime Upload HTTP Contract**.
- In-screen retry for listener bind failure (user may leave via **Shell Upload Cancel** and open upload again from **Shell Home**).
- Deck App–hosted upload UI or marketplace.
- New public **Deck App** APIs for storage, networking, or arbitrary npm dependencies beyond iteration 2 rules.
- Strong sandboxing of Deck App code from host globals.

## Acceptance Criteria

- From **Shell Home**, user can open **Runtime Upload** (**Shell Upload Entry**) and see **Runtime Upload URL** when bind succeeds, or a clear error when all ports fail.
- Browser can open **Runtime Upload Web UI**, upload a valid **Runtime Upload Archive**, and receive JSON success; device shows install outcome consistent with **CONTEXT.md** (success closes upload screen and stops listener; failure keeps screen open).
- `curl` (or equivalent) can **POST** `application/zip` or `multipart/form-data` with field **`archive`** and parse JSON responses.
- Second overlapping **POST /upload** during ingest receives **409** and JSON failure.
- New install appears in **Installed Deck App Library** after restart; **Runtime Upload Replacement** updates existing package by **Deck App Package Name**; replacing **Active Deck App** triggers **Deck App Runtime Restart**.
- **Installed Deck App Removal** works for non-active entries and is refused for **Active Deck App**.
- **Start Input** toggles Deck App / Shell as specified; **Shell Upload Screen** does not close on **Start** alone.
- **Shell Home** list order matches manifest **`name`** rules in **CONTEXT.md**.
