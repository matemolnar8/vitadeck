---
name: pages
description: Host a static site on this repo's GitHub Pages. Use when uploading a folder with index.html, including a comprehende export, or when a pull request needs a hosted review URL.
---

# Pages

Copy a static site onto this repository's `gh-pages` branch so it has a public URL.

The publish script lives in [matemolnar8/comprehende](https://github.com/matemolnar8/comprehende) (`scripts/pages-review.ts`), not here. You need a local checkout of that repo with `pnpm install` done.

## Host

1. Confirm the folder contains `index.html`. Done when that file is present. If it is missing, use the built site (a comprehende `export` out dir).
2. Pick dest. A pull request they named, or `gh pr view --json number` when hosting that review, uses `--pr`. Anything else uses `--name` with a lowercase slug they named, or one you make. Done when you have exactly one of `--pr` or `--name`.
3. From the comprehende checkout, run `pnpm exec tsx scripts/pages-review.ts publish --repo <this repo root> --dir <folder>` with that dest. `--repo` must point at this VitaDeck work tree so the site lands on VitaDeck's `gh-pages`; `--dir` may live anywhere. Other flags: `pnpm exec tsx scripts/pages-review.ts --help`. Done when the command prints a URL under `https://matemolnar8.github.io/vitadeck/`.
4. Give them that URL.

A 404 at the URL means Pages is off. Enable once in the GitHub repo: Settings, Pages, Deploy from a branch, `gh-pages`, `/`. The publish already succeeded.
