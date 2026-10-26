# PRIOR_KNOWLEDGE — R0 Load Record (re-run 2026-10-27 with credentials)

## Store status
- Location (A-008, amended): `github.com/qualitycoding/knowledge` — **exists, private, readable** with the human-supplied token.
- `commit_read`: `09f34582dfb50f876834a42b85ba19efac5a3129` (main HEAD at load time).
- Contents at load: 278 files; `TAXONOMY.md` present; `knowledge/` populated; **`lessons/` tree absent — the store currently holds zero lessons**, so zero lessons were loadable. (S-KNOW will add this run's lesson under the Appendix C layout.)
- Matching rule (Appendix C) run against task tags `domain:audio, domain:guitar-amp-sim, tech:juce, tech:cmake, tech:vst3, tech:au, tech:ableton-live, tech:github-actions, tech:pluginval` (+`applies:all`): 78 items matched on tags; the list below records every item whose content is actually applicable to this plan, and the disposition of the rest.

## Knowledge items loaded and how each is applied

### Applied — changed the plan

| Item | Fact | Application |
|---|---|---|
| K-20261004T151500Z-juce-8-vs-9-vst3-breaking-change | JUCE 8.x runs through **8.0.15**; 9.x (9.0.0+) restructures the VST3 client extension API among 35+ breaking changes. | **D-001 amended: pin 8.0.15, forbid silent 9.x upgrade.** Live-verified against the tag list during this re-run (9.0.4 newest; 8.0.15 = `91ad83ae`; an `8.0.16-security` tag also exists — evaluated at S-001). This supersedes the planning-time pin 8.0.12 (C-001 updated). New risk R-014. |
| K-20261009T092101Z-juce-vst3-can-replace-vst2-default-and-error | `JUCE_VST3_CAN_REPLACE_VST2` defaults to 1 and can trigger a hard `#error` about VST2/VST3 parameter-automation conflict; for a plugin that never shipped VST2, define it `=0` at directory scope before `add_subdirectory(JUCE)`. | **New action in S-002** (`add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)` before the JUCE subdirectory) and a checkpoint in S-003 (parameter-ID layout must not rely on VST2 legacy mapping). Prevents a probable build-blocking error. |
| K-20261009T092102Z-juce-standalone-target-compiles-vst3-client | The Standalone target also compiles `juce_audio_plugin_client_VST3.cpp`, so macros must reach both targets (directory-scope definition does). | Same S-002 action; the define is placed at directory scope precisely because Standalone needs it too. |
| K-20261008T173541Z-sandbox-build-tools-and-juce-tags-2026-10-08 | JUCE tag 8.0.15 = `91ad83ae34a81e0833b1a2b0866f54846370ae53`, observed via anonymous ls-remote. | ENVIRONMENT.md records the exact tag **and commit hash**; S-001 verifies the submodule hash, not just the tag name (supply-chain lens). |
| K-20261009T164500Z-juce-module-build-on-one-cpu-core-exceeds-a-300-second-tool-call | Full JUCE module builds exceed a 300 s tool call on a single core; background `nohup` builds did not survive across calls. | ENVIRONMENT.md and S-001 note: builds run in CI (GitHub Actions runners) as the source of truth; local/sandbox builds use incremental per-target builds (`--target GrimAmp_VST3`) — DR-7 already covers unreachable-CI fallback. |
| K-20261009T092103Z-vst3-system-folder-needs-write-access-windows | Installing to `C:\Program Files\Common Files\VST3\` needs elevated write access on Windows. | T-060 installer smoke test and S-016 packaging notes updated: installer must request elevation; manual-install README path notes admin rights. |
| K-20261009T160121Z-dispatch-default-branch | A `workflow_dispatch` workflow offers no "Run workflow" button while it exists only on a non-default branch. | S-014 note: manual-dispatch CI testing requires the workflow on the default branch (or use `gh workflow run`); CI validation happens via PR/push triggers on the impl branch. |
| K-20261009T160122Z-token-scopes | Pushing files under `.github/workflows/` needs the token's **Workflows** permission, separate from Contents. | S-014 records this as a prerequisite check: before pushing workflow files, the implementer verifies the credential's Workflows permission (403 otherwise). This planning push contains no workflow files, so Contents suffices here. |

### Applied — informative, no plan change

| Item | Fact | Disposition |
|---|---|---|
| K-20261004T162310Z-cabinet-structure-and-invariant-excitation | In a reference recording, harmonic levels tie to absolute frequency (fixed cabinet), not played pitch. | Consistent with D-004 (static IR convolution is the right model); no change. |
| K-20261004T162311Z-minimum-phase-cabinet-ir-from-table | A dB-vs-log-frequency cabinet table can be turned into a minimum-phase FIR. | Useful fallback for generating the S-008 synthetic placeholder IRs; noted in S-008 actions. |
| K-20261009T120102Z-oildrum-legacy-pitch-smoothing-bug | A smoothing update written with the wrong difference direction caused audible artifacts. | Cautionary pattern for D-006/S-012: smoothing must be `current += coef*(target-current)` form and is covered by T-010's click bound. |
| K-20261009T164200Z-juce-combobox-attachment-spaces-items-linearly-over-the-parameter-range | ComboBoxAttachment spaces items linearly across the parameter range. | S-010 note: choice parameters use `AudioParameterChoice` so index↔value mapping is exact; typed non-item text has no representation — GUI uses combo boxes, not editable fields, for choice params. |
| K-20261009T164300Z-juce-8-0-15-combobox-has-no-gettext-editor-accessor | JUCE 8.0.15 `ComboBox` has no `getCurrentTextEditor` accessor. | S-010 note: don't design around editable combo boxes. |
| K-20261004T102023Z / K-20261004T151600Z / K-20261004T162306Z / K-20261009T120115Z / K-20261009T193502Z / K-20261009T193504Z (github-access-from-sandbox set) | Token-header hygiene (`http.extraheader` Basic `x-access-token`, nothing in `.git/config`), REST Git Data API fallback for broken pack transport, per-session repo authorization quirks, proxy-injected git config, tokens never written to files. | Applied by the **planner** during this re-run's Phase 5 push; also written into S-000's environment checks (DR-7 covers fallbacks). |
| K-20261009T160120Z-actions-blob-unreachable | CI job logs/artifacts may be unreachable from a sandbox (Azure blob host). | DR-7 already covers; ENVIRONMENT.md host table updated with the ci-diag branch workaround. |

### Matched on tags but not applicable (loaded, reviewed, justified)

- `domain:audio`-only matches with no bearing on an amp-sim plugin: drum-synthesis engine measurements (oildrumkit peak levels/spectra, inharmonic cymbal density, uniform-noise sqrt3), modal-plate physics set, impact-contact-mechanics set, dsp-port-parity (C↔JS port), htdemucs-onnx, bs-roformer-sw, lead-rhythm-guitar-separation, basic-pitch set, audacity-* set, general-midi-drum-map, billboard-chord-corpus, consonance models, fretboard geometry, bass-guitar-tuning, harmonic-level-measurement. Justification: this project is a native C++ amp plugin with no synthesis engines, no stem separation, no MIDI analysis, and no JS port.
- `tech:ableton-live` match K-20261009T092104Z-ableton-computer-midi-keyboard-layout: about Live's computer-keyboard MIDI input; this plugin takes no MIDI input. Not applied.
- `tech:juce` matches K-20261009T164400Z-juce-fontoptions (FontOptions introduction version undocumented — pin 8.0.15 makes it available; no action), K-20261009T092105Z-juce-mpesynthesiser (no synth/MIDI in this plugin), juce-linux-dependencies × 2 (Linux out of scope per A-002; noted for the future).

## Lessons loaded
**Zero** — the store has no `lessons/` tree at `commit_read`. Recorded explicitly for the Phase 5 consistency check.
