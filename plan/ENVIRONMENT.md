# ENVIRONMENT — Pinned Versions & Verified Setup Commands

> **Protocol conformance amendment (D-001o, 2026-10-31):** this file was
> reviewed against planning-protocol **v3.6** (qualitycoding/skills). It gains
> §0.1 (environment-class probe), corrected pins and setup commands (§1.3.6 pin
> verification), §1.5 (ephemeral-environment protections), and a per-runner-class
> T-050 target table. The review is `reviews/PROTOCOL_CONFORMANCE_v36.md`.

## 0.1 Environment-class probe (protocol v3.6)

**Probe result: `ephemeral`** — classification per the protocol's observed-class
table, on first-hand evidence gathered during this run (2026-10-28…31):

| Observed behavior | Dates observed |
|---|---|
| `/tmp` wiped between tool calls / turns: build trees and restored toolchains lost repeatedly (twice mid-build) | 2026-10-28, 2026-10-30 |
| Exec bits stripped on copies arriving from the durable mount: relinked test binaries failed ctest with `BAD_COMMAND` until `chmod +x` | 2026-10-28 |
| Background `nohup` jobs die with the tool call; only `setsid`-detached processes survive | 2026-10-28 |
| The durable mount serves **stale bytes** to compilers and to git (one link step mixed old and new object code); `touch` does not update mtimes, so ninja never sees edits | 2026-10-28 |
| Cross-filesystem `cp`/`mv` intermittently loses data; git `index.lock` writes fail transiently (bounded retry loop succeeded on attempt 3) | 2026-10-28…30 |
| Outbound network to github.com unreachable (`curl` exit 124) → Rule 6 push cadence runs through the bundle fallback (see Hosts table and HANDOFF.md) | 2026-10-28…31 |

Consequences: §1.5 protections are **active**; the run carries the `[ephemeral]`
environment tag (Rule 0.3, recorded in `plan/PROFILE.md`).

## 1.5 Ephemeral-Environment Protections (protocol v3.6)

1. **Durable surface.** Everything irreplaceable lives on `/mnt/agents`
   (repo clones, `sysroot/sysroot.tar` + `sysroot/cmake-root.tar` toolchain
   images, diagnostic harnesses, handoff bundles). `/tmp` is scratch only and
   is treated as wiped at every turn boundary.
2. **Turn-start restaging.** Restore the toolchain before any build work:
   `bash tools/restage-env.sh` (idempotent; restores `/tmp/sysroot` +
   `/tmp/cmake-root` from the durable tarballs, restores exec bits, prints the
   required environment exports). Then copy the repo to a fresh `/tmp` tree
   and build there — never build inside the durable mount (stale-byte hazard).
3. **Exec-bit restoration.** The mount strips exec bits on copy. After every
   relink or copy of a binary: `chmod +x` before invoking it.
4. **Checkpointed resumable builds.** Build steps are restartable end-to-end:
   restage (item 2) → fresh `/tmp` source copy → configure → build. LTO links
   take 13–20 min, which exceeds a tool-call boundary: run them `setsid`-detached
   and poll the log for completion markers; never `nohup`.
5. **Ordering invariants (learned first-hand; do not relearn):**
   - CMake **silently ignores environment-variable** dependency overrides
     (`CPLUS_INCLUDE_PATH`, `LIBRARY_PATH`, `PKG_CONFIG_PATH` set at configure
     time do not enter the cache). Pass dependency paths as `-D` cache
     arguments at configure time, or export the vars **every** build command.
   - Re-running the configure step re-touches generated headers and invalidates
     dependent objects — expect a long rebuild after any reconfigure.
   - Because mtimes on the durable mount do not update, ninja may not rebuild
     edited sources: **delete the affected `.o`/`.a` to force rebuild**, and
     verify source state by content (`grep`), never by timestamp.
6. **Right-sizing steps to container lifetimes.** Keep single tool calls short;
   anything long runs detached (item 4). Anything longer than a container
   lifetime belongs in CI.
7. **CI for long work.** Long verification dimensions (pluginval on Win/mac,
   ASan+UBSan, mutation testing, per-runner benchmarks) run CI-side. A
   dimension blocked locally is recorded as blocked with a named CI-side owner
   (`BLOCKED.md`) — **never reported as a pass** (D-001a, D-001b, D-001d).
8. **Evidence honesty (Rule 14).** A check that cannot fail is not a check:
   no `|| true` / `continue-on-error` on load-bearing steps, artifact uploads
   assert their inputs exist, every substitution is a logged deviation.

## Pins

| Tool | Version | Source | Verified during planning / existence check |
|---|---|---|---|
| JUCE | **8.0.16-security** (commit `ea77444750de45e036e0279bb0291144bf8e3688`) — **supersedes the original pin 8.0.15** @ `91ad83ae34a81e0833b1a2b0866f54846370ae53` per **D-001** | `github.com/juce-framework/JUCE` (git submodule, tag `8.0.16-security`) | Original pin live-verified 2026-10-27 (newest 9.0.4; 8.x line ends at 8.0.15 + `8.0.16-security` patch tag; 9.x restructures the VST3 client API — forbidden for v1, K-20261004T151500Z). D-001 evaluated and adopted the security patch tag at S-001; `git -C JUCE rev-parse HEAD` == `ea774447…` verified in CI (`Verify JUCE submodule pin` step). Ships VST3 SDK 3.8.0 (MIT). |
| VST3 SDK | 3.8.0 (bundled with JUCE) | via JUCE | — |
| CMake | ≥ 3.25 | kitware.com / OS package manager | — |
| C++ standard | C++20 | compiler flags | pluginval 2.x betas also build C++20 (same toolchain compatibility) |
| pluginval | **v2.0.0-beta3** — **supersedes the original pin 2.0.0**, which does not exist as a stable release (only 2.x betas are published; verified via `gh api repos/Tracktion/pluginval/releases`, 2026-10-29) | `github.com/Tracktion/pluginval` releases; action ref `Tracktion/pluginval/validate@v2.0.0-beta3` | D-001i. The action's default binary (beta1) aborts on macOS with a missing `librtcheck.dylib`, so the binary is pinned explicitly via `pluginval-version: v2.0.0-beta3`. Re-pin to 2.0.0 when a stable release exists. |
| Compilers (CI) | MSVC 2022+ (`windows-latest`), Xcode 15+ clang (`macos-latest`) | GitHub Actions images | JUCE 8.0.12 defaults Projucer to VS2026; CMake path unaffected |
| Python (test tooling) | 3.11+ | python.org | for IR/preset lint scripts |
| Mull (mutation testing, S-MUTATE) | **0.34.0 line**, packages `mull-19` + `clang-19` + `lld-19` (suffixes must agree); config file **`mull.yml` is part of this pin** (the runner has no `--include-path` flag) | Mull apt repo / `brew install mull-project/mull/mull` | ubuntu-latest CI job only; flags `-O0 -g -grecord-command-line -fpass-plugin=/usr/lib/mull-ir-frontend-19`; K-20261027T123200Z, K-20261029T030100Z |
| Ableton Live (manual tests) | Live 12 (any edition, 64-bit) | user machine | VST3 supported since Live 10.1; Live is 64-bit only since Live 10 |

## Per-runner-class performance targets (Rule 14; T-050 / A-010)

Budget: mean CPU ≤ 15 % of one core (Frost voicing, High oversampling, IR
active). Per class, with pre-decided miss rules:

| Runner class | Target | Status / miss rule |
|---|---|---|
| Local sandbox (shared 2-vCPU Xeon, Linux) | informational only | Not the CI runner class (D-001c): Eco ≈ 3.6 %, High ≈ 22–24 % measured locally; numbers never gate. |
| `windows-latest` (AMD EPYC, 4 cpus) | ≤ 15 % | Measured 10.8–11.7 % High per voicing — PASS (run 40771992301). |
| `macos-latest` (virtual Apple M1, 3 cpus) | ≤ 15 % | Measured 18.0–35.6 % High — recorded miss; maintainer decision D-001n (2026-10-30): DSP and budget unchanged, bench step stays informational. Re-gating or budget relaxation must go through the same human-gate mechanism. |

Regression baseline: not yet stored (D-001n keeps the step informational);
bootstrap recipe recorded in `reviews/PROTOCOL_CONFORMANCE_v36.md` §5.

## Setup commands (literal)

```bash
# 1. Clone (implementer)
git clone https://github.com/qualitycoding/guitar-amp-vst.git
cd guitar-amp-vst
git checkout gen-20261027T120000Z-guitar-amp-vst   # generation branch (planning artifacts)
git checkout -b impl-run-20261027T120000Z-guitar-amp-vst   # implementation branch (S-000)

# 2. JUCE submodule (added in S-001; verify with)
git submodule update --init --recursive
git -C JUCE describe --tags        # must print: 8.0.16-security   (D-001 superseded the 8.0.15 pin)
git -C JUCE rev-parse HEAD         # must print: ea77444750de45e036e0279bb0291144bf8e3688

# 2a. Required compile definition (K-20261009T092101Z / K-20261009T092102Z):
# add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0) must appear at directory
# scope BEFORE add_subdirectory(JUCE) — the Standalone target also compiles the
# VST3 client, and the default (=1) can hard-#error a plugin that never shipped VST2.

# 2b. Build-time guidance (K-20261009T164500Z): a full JUCE build exceeds a 300 s
# tool call on a single core. Use CI runners as the build source of truth; locally
# build incrementally per target: cmake --build build --target GrimAmp_VST3

# 3. Configure + build (Windows: run in "x64 Native Tools" or Developer PowerShell)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# 4. Frozen suite
sha256sum -c tests/FROZEN_MANIFEST.sha256        # Windows: use Git Bash
ctest --test-dir build --output-on-failure -C Release

# 5. pluginval (download the v2.0.0-beta3 release binary per D-001i; macOS: remove quarantine attr)
pluginval validate --strictness-level 5 build/GrimAmp_artefacts/Release/VST3/GrimAmp.vst3
# macOS AU additionally:
pluginval validate --strictness-level 5 build/GrimAmp_artefacts/Release/AU/GrimAmp.component

# 6. Freeze manifest check is also wired into CI (S-014).
```

## Hosts the work depends on (Phase 0.1 reachability)

| Host | Needed for | If unreachable (DR-7) |
|---|---|---|
| github.com | repo, CI, releases | build locally; attach artifacts manually |
| githubusercontent.com / objects.githubusercontent.com | submodule fetch, release downloads | same as above |
| api.github.com | CI status checks | report via web UI |
| kitware.com / OS package mirrors | CMake install | any mirror; version pin is what matters |
| help.ableton.com | Live format support docs (already verified) | n/a — captured in `research/SOURCES.md` |
| **outbound network from the build sandbox (ephemeral class)** | Rule 6 push cadence, `git fetch/push`, pluginval download inside the sandbox | **Named fallback (used this run, D-001a + D-001o):** commits accumulate locally and ship as a `git bundle` handed to the human, who pushes and performs the `ls-remote` verification. Raw file reads via the web work; git protocol does not. |

## Implementer credentials the planner does not need but documents

| Credential | Scope | Needed at | Dry-run/staging target |
|---|---|---|---|
| GitHub push token (user's normal auth) | repo push, PRs, Actions | S-000, S-014, S-017 | pushing to the `impl-*` branch. **Never stored in the repo** — redact (e.g. `github_pat_***`); a token pasted into chat is treated as compromised and rotated. |
| Apple Developer ID (optional for v1) | macOS notarization of installer | S-016 | unsigned build first; notarization is `proceed-with-rescope`-able at G-002 |
| Windows code-signing cert (optional for v1) | installer signing | S-016 | unsigned installer first; same rescope path |

## Data / sensitivity

No secrets in the repo. CI needs no tokens beyond the default `GITHUB_TOKEN`. User-facing files (presets, IRs) never leave the user's machine (D-010).
