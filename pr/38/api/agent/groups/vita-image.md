Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 282d57c499f34473e74cbd281d12ce11fe79eadc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               282d57c499f34473e74cbd281d12ce11fe79eadc

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 282d57c

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc

Review concern 09 of 09: Vita image: build vitaShaRK from source, pin vitaGL (`vita-image`)

Part: Vita image fix

The why:

The Vita VPK job [failed on this branch](source:s4) and on `main`: vitaGL master calls `shark_set_shader_association_path`, which the vdpm vitaShaRK in `gnuton/vitasdk-docker` predates. [The commit](source:s6) installs vitaShaRK from source first.

The what:

The Dockerfile clones vitaShaRK at df24065 and runs `make install` into `$VITASDK` before vitaGL, and vitaGL moves from `--depth=1` master to a checkout of 76f43e6.

Look for:
- Breaking: `--depth=1` is gone from both clones, so the image build fetches full history for vitaShaRK and vitaGL. Slower first build; the layers cache afterwards.
- The CMake link list already names `vitashark`, so the newer `libvitashark.a` replaces the vdpm one at the same path; `cmake --build out-vita` produced `vitadeck.vpk` locally against this image.

Hunk refs for this concern:
- Dockerfile