# PROTOTYPE — Media Remote Deck App

**Question:** Can a Deck App act like a Stream Deck / media remote over Host Control?

**Run (desktop dev loop):**

```bash
pnpm --dir js/examples/_prototype-media-remote run:host
```

Requires a built `out/vitadeck`, Host Control companion, and `xdotool` on Linux.

## Verdict

_(fill in after trying on real hardware)_

- [ ] Grid UX feels usable on 960×544
- [ ] Media keys reach the active player on Linux/macOS
- [ ] Latency acceptable for rapid button taps
- [ ] `host.capabilities` greys out unsupported pads (not implemented in this throwaway)

## What we learned (agent pass)

- **Branch discovery:** Host Control lives on `host-control-companion`, not `main`. An agent starting on `main` sees stale `dist/` artifacts and no `createHostControlClient` in source — easy to conclude the feature does not exist.
- **Typed `client.command`:** A grid of pads cannot call `client.command(pad.command)` with a union command name; each pad needs a closure (`run: () => client.command("host.media.play_pause")`) or a switch. Worth a short SDK doc snippet.
- **Media commands did not exist:** The prototype added `host.media.*` to the registry + companion (`xdotool` / `osascript`). Second-iteration material should land in the planning doc.
- **Capabilities vs UI:** `host.capabilities.commands` lists OS-supported commands only; the prototype does not grey out unsupported pads yet.
- **One-command run:** `pnpm run:host` in this folder builds, installs to `vitadeck-data`, seeds `host-control-url.txt`, starts companion, launches `out/vitadeck` — still heavy without a prior native build.
