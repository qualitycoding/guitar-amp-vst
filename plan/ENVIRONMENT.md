# ENVIRONMENT — Pinned Versions & Verified Setup Commands

## Pins

| Tool | Version | Source | Verified during planning |
|---|---|---|---|
| JUCE | **8.0.15** (commit `91ad83ae34a81e0833b1a2b0866f54846370ae53`) | `github.com/juce-framework/JUCE` (git submodule, tag `8.0.15`) | Live tag listing 2026-10-27: newest 9.0.4; 8.x line ends at 8.0.15 (+ `8.0.16-security` patch tag, evaluated at S-001); 9.x restructures the VST3 client API — forbidden for v1 (K-20261004T151500Z). Ships VST3 SDK 3.8.0 (MIT). |
| VST3 SDK | 3.8.0 (bundled with JUCE) | via JUCE | — |
| CMake | ≥ 3.25 | kitware.com / OS package manager | — |
| C++ standard | C++20 | compiler flags | pluginval 2.0.0 also builds C++20 (same toolchain compatibility) |
| pluginval | **2.0.0** | `github.com/Tracktion/pluginval` releases | cross-platform VST3/AU validator, strictness levels 1–10, CLI + CI actions |
| Compilers (CI) | MSVC 2022+ (`windows-latest`), Xcode 15+ clang (`macos-latest`) | GitHub Actions images | JUCE 8.0.12 defaults Projucer to VS2026; CMake path unaffected |
| Python (test tooling) | 3.11+ | python.org | for IR/preset lint scripts |
| Ableton Live (manual tests) | Live 12 (any edition, 64-bit) | user machine | VST3 supported since Live 10.1; Live is 64-bit only since Live 10 |

## Setup commands (literal)

```bash
# 1. Clone (implementer)
git clone https://github.com/qualitycoding/guitar-amp-vst.git
cd guitar-amp-vst
git checkout gen-20261027T120000Z-guitar-amp-vst   # generation branch (planning artifacts)
git checkout -b impl-run-20261027T120000Z-guitar-amp-vst   # implementation branch (S-000)

# 2. JUCE submodule (added in S-001; verify with)
git submodule update --init --recursive
git -C JUCE describe --tags        # must print: 8.0.15
git -C JUCE rev-parse HEAD         # must print: 91ad83ae34a81e0833b1a2b0866f54846370ae53

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

# 5. pluginval (download v2.0.0 release binary; macOS: remove quarantine attr)
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

## Implementer credentials the planner does not need but documents

| Credential | Scope | Needed at | Dry-run/staging target |
|---|---|---|---|
| GitHub push token (user's normal auth) | repo push, PRs, Actions | S-000, S-014, S-017 | pushing to the `impl-*` branch |
| Apple Developer ID (optional for v1) | macOS notarization of installer | S-016 | unsigned build first; notarization is `proceed-with-rescope`-able at G-002 |
| Windows code-signing cert (optional for v1) | installer signing | S-016 | unsigned installer first; same rescope path |

## Data / sensitivity

No secrets in the repo. CI needs no tokens beyond the default `GITHUB_TOKEN`. User-facing files (presets, IRs) never leave the user's machine (D-010).
