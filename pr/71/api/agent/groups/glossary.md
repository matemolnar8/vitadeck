Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 3ad801f013a9deced2cca7d5928b544f9aa18750` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 3ad801f013a9deced2cca7d5928b544f9aa18750 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               3ad801f013a9deced2cca7d5928b544f9aa18750

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... chore/docs-and-hygiene

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 3ad801f013a9deced2cca7d5928b544f9aa18750

Review concern 01 of 07: Glossary matches the shipped Shell and upload flow (`glossary`)

Part: Glossary

The why:

[#59](source:s1) says `CONTEXT.md` still calls Runtime Upload future work and claims Button is the only interactive element.

The what:

Each Shell, upload-listener, and input statement is rewritten in place to describe what `src/shell/shell.c`, `src/ui/input.c`, and `src/core/package_library.c` do, with no new terms.

Look for:
- Subtle: **Shell Upload Screen** keeps its name but now means the running state of the Upload row on the Shell Home Screen, because no separate upload view exists in `shell.c`.
- The listener now documents as running until Shell Upload Cancel or exit, including while the Shell is hidden; `hide_shell_to_deck` and `activate_package` never call `upload_server_stop`.
- Activation is no longer always explicit: `package_library_publish_package` activates the uploaded package when no Active Deck App is set, and startup activates the first library entry.

Hunk refs for this concern:
- CONTEXT.md