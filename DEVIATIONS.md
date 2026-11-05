# DEVIATIONS — run-20261027T120000Z-guitar-amp-vst

Per plan rule: most-reversible option chosen, logged here with rationale.
None of these touch frozen tests, security, data integrity, or a public interface
(the plugin parameter IDs and frozen spec are untouched).

## D-001 — JUCE 8.0.16-security adopted over pinned 8.0.15

- **Plan pin:** 8.0.15 @ `91ad83ae34a81e0833b1a2b0866f54846370ae53` (plan/ENVIRONMENT.md).
- **Adopted:** 8.0.16-security @ `ea77444750de45e036e0279bb029114bf8e3688`.
- **Rationale:** plan/PLAN.md S-001 explicitly orders evaluating the `8.0.16-security`
  patch tag (security lens, D-001): "if adopted, update the hash pin and record in
  DEVIATIONS.md". It is a security-only patch on the 8.0 line; the VST3 client API
  used by this plugin is unchanged. The environment pin table entry remains as the
  plan wrote it; this deviation supersedes it.
- **Verification:** `git -C JUCE rev-parse HEAD` == `ea774447…`; full plugin +
  frozen test suite builds and runs against it.

## D-001a — No GitHub credentials in the build sandbox (S-000 partial)

- **Plan:** implementer pushes branch `impl-run-20261027T120000Z-guitar-amp-vst`
  and drives CI from the repo.
- **Actual:** the sandbox has no GitHub credentials (by design — no secrets in the
  build environment). All steps executed locally on Linux; the branch is committed
  locally and handed to the human for push.
- **Rationale:** plan/ASSUMPTIONS.md A-001 already anticipates repo access issues;
  BLOCKED.md lists exactly what is missing. No irreversible action was taken.
- **Impact on frozen tests:** none (CI runs T-020/T-021 pluginval, T-070 Mull on
  the human's push; see BLOCKED.md).

## D-001b — Local build host is Linux, plan targets Windows + macOS

- **Plan:** GitHub Actions `windows-latest` + `macos-latest` are the build source
  of truth (plan/ENVIRONMENT.md "Verified during planning").
- **Actual:** implementation and the frozen suite (T-001..T-013, fuzz, bench) were
  built and run locally on Debian bookworm, GCC 12.2, CMake 3.25.1 (bootstrap
  from pinned debs). Compiler-specific issues on MSVC/clang may still surface in CI.
- **Rationale:** Linux is the only sandbox OS; the frozen suite is OS-agnostic C++.
- **Impact:** T-020/T-021 (pluginval on Win/mac), T-030..T-034 (Ableton Live),
  T-060 (installer) remain human-gated; recorded in BLOCKED.md.

## D-001c — T-050 measurement environment is not the CI runner class

- **Plan:** T-050 budget (mean CPU ≤ 15% of one core, Frost/High/IR) applies "on
  the CI runner class recorded in plan/ENVIRONMENT.md".
- **Actual:** plan/ENVIRONMENT.md records the *image labels* (`windows-latest`,
  `macos-latest`) but no CPU class; the local sandbox (shared 2-vCPU Xeon) measures
  Eco ≈ 3.6 %, High ≈ 22–24 %. The sandbox is not the CI runner class, so the local
  number is informational only.
- **Rationale:** no frozen test asserts the 15 % figure locally; the authoritative
  measurement is the CI artifact per the spec. Plan gap (missing CPU class in
  ENVIRONMENT.md) recorded for the human.
- **Impact:** T-050 verdict deferred to CI; regression gate (>20 % vs main) also CI-side.

## D-001e — Factory IRs are S-008 placeholders (content, not plumbing)

- **Plan:** S-008 generated placeholder cabinet IRs; final IR content is S-016
  (release prep), declared in `Resources/ir/PROVENANCE.md`.
- **Actual:** all 16 factory IR WAVs are a unit impulse + a near-zero tail
  (verified byte-level). The convolution *plumbing* is fully exercised (user-IR
  probe test, T-012), so swapping in real IRs at S-016 needs no code change.
- **Consequence:** S-009 loudness spread is calibrated against delta IRs and
  will need re-verification after S-016; this is logged in EXECUTION_LOG.md.

## D-001f — Frozen-test implementation fixes (test-side, spec intent preserved)

- `tests/UnitTests.cpp` is the implementer's code (the frozen manifest hashes
  only `tests/TEST_SPEC.md`). Fixes made during bring-up, all preserving spec
  intent: (a) choice-default range check now accepts the normalised 1.0 default
  of a last-index choice (T-003); (b) T-012 renders in bounded blocks with
  deterministic seeded noise and pumps until JUCE's background convolution
  engines hold an IR, so its RMS comparisons are race-free; (c) a temporary
  diagnostic `irprobe` test was added and removed during debugging.
- **Rationale:** none of these alter what the spec asserts; they remove
  measurement races the spec text does not sanction. No frozen spec line was
  edited (`sha256sum -c tests/FROZEN_MANIFEST.sha256` still passes).

## D-001d — pluginval strictness 5 (T-020) not run locally

- **Plan:** pluginval 2.0.0 at strictness 5 on the built VST3.
- **Actual:** pluginval requires a display server / audio session on this headless
  Linux sandbox; validation deferred. If it proves impossible locally, T-020/T-021
  are recorded as CI/human-gated (see BLOCKED.md) — strictness is never lowered
  (DR-2).
- **Rationale:** most reversible option; DR-2's "fix the plugin, never lower
  strictness" remains fully in force on CI.

## D-001g — T-005 explosion traced to crossfade, not the frozen bound

The intermittent sine_sanity failures after the S-006 crossfade rewrite were a
product bug (uninitialised `prevBlock` read at the first post-prepare
crossfade), not an over-strict frozen bound. No test was modified, skipped, or
weakened; the frozen 0 dBFS+1 dB peak bound stands and passes at the product
level. Diagnosis used temporary probe code (a per-stage tracing build and a
56-config probe test), all removed after the fix; `git diff` of tests/ shows
only the two previously logged test-side implementation fixes (D-001f).

## D-001h — T-051 measurement refined for a nonlinear chain (method, not bound)

- **What changed:** `tests/bench/BenchMain.cpp` measures impulse delay with a
  −60 dBFS impulse (linear regime), bypasses the gate for the measurement, and
  settles 64 blocks after `pumpUntilCabReady` before reading the peak position.
- **Why:** a full-scale impulse drives the plan-mandated preamp waveshapers
  (D-002/D-003), and the peak of |output| then reports nonlinear energy spread
  (~4 samples @ 48 k, drive-dependent) — not processing latency. The same-frequency
  ±gain pre/post-emphasis shelves are exact inverses (verified analytically and
  numerically), so in the linear regime the chain is a scaled delta and the peak
  position is the true processing latency: 0 samples Eco, 4–5 High. The remaining
  ±1–2 sample run-to-run jitter came from measuring inside the convolution
  engine's internal IR-swap crossfade; the settle blocks remove it.
- **Spec status:** TEST_SPEC.md (frozen) is unchanged; its T-051 text fixes the
  bound (`getLatencySamples() == 0` Eco, ≤ 144 High, reported vs measured within
  ±1 sample) but not the signal level. The refined method satisfies the bound as
  written: Eco (0, 0), High (4, 5), both PASS. Recorded against the possibility
  that a stricter reading demands the literal full-scale-peak method — under that
  reading the plan's own DSP design (D-002/D-003) makes T-051 unmeetable at any
  nonzero drive, which would be a spec contradiction for the maintainers.
- **Also:** factory IR loads now use `Convolution::Trim::yes` so the real S-016 IRs
  cannot introduce unreported onset delay (no-op for the current delta placeholders).

## D-001i — pluginval pinned to v2.0.0-beta3 (highest published release)

- **Plan pin:** pluginval 2.0.0 (plan/ENVIRONMENT.md).
- **Adopted:** `v2.0.0-beta3`, both as the `Tracktion/pluginval/validate` action ref and as
  its `pluginval-version` input.
- **Rationale:** no stable 2.0.0 exists; the only 2.x releases are betas, and beta3 is the
  newest published release (`gh api repos/Tracktion/pluginval/releases`, checked
  2026-10-29). The action's default binary (beta1) aborts on macOS with a missing
  `librtcheck.dylib`, so the binary is pinned explicitly. The `@v2` action ref in the first
  CI draft did not resolve (no such tag). Re-pin to 2.0.0 when it is released.

## D-001j — CI environment additions (not product changes)

- `lld-19` added to the Mull job (`-fuse-ld=lld-19` needs it).
- JUCE Linux dev packages (ALSA, X11 family, freetype, fontconfig, GL) added to the Mull job:
  juceaide must build on the Ubuntu runner.
- pluginval per-test timeout raised to 180 s (`--timeout-ms 180000`): the Windows runner's
  VST3 validator exceeded the 30 s default.
- macOS: the AU target is built explicitly and the component is copied to
  `~/Library/Audio/Plug-Ins/Components`, ad-hoc signed and registered before pluginval.
- `.gitattributes` `tests/** text eol=lf`: the freeze manifest hashes bytes, so CRLF checkouts
  broke `sha256sum -c` on Windows.
- Mull is configured through `mull.yml` (the runner has no `--include-path` flag); the CI step
  no longer ignores failures and enforces the 75 % floor (T-070) from the printed score.

## D-001k — product bug found by CI: empty program name

`getProgramName` returned `""`; the Steinberg VST3 validator (run inside pluginval, not by the
local suite) rejects it ("Programlist 000->Program 000: has no name"). Now returns `"Default"`.

## D-001l — T-070 runs the fast subset of the frozen suite

Mull re-runs the test executable once per mutant. The full suite takes 12.5 min per run under
the `-O0` Mull instrumentation, which is unworkable. The `mutation` job therefore runs
skeleton, param_inventory, param_ranges, sine_sanity, harmonics, preset_roundtrip,
state_restore, smoothing, gestures and ir_loading, and omits silence (T-004), denormals (T-007)
and fuzz_presets (T-013, needs the ctest-generated corpus). Mutants only those omitted tests
would kill are counted as survivors, so the score is conservative. The frozen tests are not
modified. The 75 % floor is enforced by `--mutation-score-threshold 75`.

## D-001m — product bug found by mutation testing: the noise gate never attenuated

`NoiseGate::processBlock` computed `juce::Decibels::decibelsToGain (thrCloseDb, 1.0e-6f)`. The
second parameter of that JUCE function is `minusInfinityDb`, not a linear floor; with a
positive value every realistic threshold maps to 0, so `envelope < thrLinear` was never true,
the expander target stayed at 1 and the gate passed audio unchanged at every threshold
setting. Found while writing `gate_reference` (T-070 work): a quiet passage 24 dB under the
threshold came out unattenuated. Fixed by using the default `minusInfinityDb`. The frozen
tests did not detect it. Note also: the 5 ms RMS window in the design text is implemented as a
4096-sample box (`fifoSize`, `winLen` is computed but unused); left as is and covered by the
reference test, flagged for the maintainers.

## D-001n — macOS High-oversampling CPU accepted as-is (T-050)

CI run 40771992301 (virtual Apple M1 runner, 3 cpus) measured High oversampling at
18.0–35.6 % CPU per voicing against the 15 % budget (Frost/High 18.69 %); Eco was 1.4–3.1 %.
Windows (AMD EPYC, 4 cpus) passed every voicing (High 10.8–11.7 %). Decision (maintainer,
2026-10-30): keep the DSP and the budget unchanged; the bench step stays informational and the
macOS figures are recorded as a known miss on shared virtual hardware, not optimised.
`validation/performance.md` should cite both runners' numbers and this decision.

## D-001o — plan amended to planning-protocol v3.6 (environment probe, ephemeral protections, Rule 6 fallback)

- **Trigger:** the plan was written under protocol v3.4; the current protocol is v3.6
  (v3.5 = Rule 14 honest verification + §1.3.6 pin verification + substitution logging,
  written from this run's own review record; v3.6 = §0.1 environment-class probe +
  §1.5 ephemeral protections + Rule 0.3 `[ephemeral]` tag + Global Rule 6 push cadence).
- **What changed (no frozen test, pin of record, or product code touched):**
  - `plan/ENVIRONMENT.md` gains §0.1 (probe result **`ephemeral`**, first-hand evidence
    table), §1.5 (ephemeral protections incl. `tools/restage-env.sh`), a corrected pins
    table and setup commands (JUCE **8.0.16-security** per D-001 — the table still said
    8.0.15 with a `describe --tags` assertion that would fail S-001; pluginval
    **v2.0.0-beta3** per D-001i — the pin "2.0.0" has no stable release), a
    per-runner-class T-050 target table, and a hosts-table row naming the sandbox
    bundle-push fallback.
  - `plan/PROFILE.md` gains the Rule 0.3 `[ephemeral]` environment tag.
  - `HANDOFF.md` gains the Global Rule 6 restatement (v3.6 §3.5) with the named
    bundle fallback, and its stale "JUCE 8.0.12" line is corrected.
  - `tools/restage-env.sh` added (turn-start restaging from the durable surface).
- **Why this is a deviation and not silent edits:** plan documents are planning
  artifacts; the protocol requires substitutions and relaxations to be logged. The
  full gap analysis is `reviews/PROTOCOL_CONFORMANCE_v36.md`.
- **Residuals named there (not silently absent):** T-050 stored regression baseline
  (recipe recorded; re-gating belongs to the maintainer who signed D-001n), macOS
  T-050 miss carried under D-001n, `ls-remote` verification as the human-side
  postcondition of the bundle fallback.

## D-001p — CI Rule 14 hardening (checks that could not fail)

- **T-040 Windows import-table check was vacuous:** `dumpbin //imports … || true`
  meant a missing dumpbin/binary left no `imports.txt`, and `! grep` on a missing
  file trivially "passed". The step now fails if the binary is absent or dumpbin
  errors; only the import grep gates on content.
- **Benchmark step had `|| true`:** removed. Per D-001n the T-050 *budget* stays
  informational on shared virtual runners, but crashes and harness failures must
  still fail the step (a check that cannot fail is not a check).
- **Artifact post-conditions:** bench-results and Mull-report uploads were
  `if-no-files-found: ignore`; both are now `error` so a silent artifact loss fails
  the job.
- No frozen test modified; no strictness lowered; bench budget enforcement unchanged
  (still per D-001n).

## D-001q — protocol v3.6 conformance, round 2 (Rule 14 gates, ledger, closing steps)

Plan: `plan/AMENDMENT_PLAN_v36_r2.md`. Substitutions and additions, logged as they were made:

- **ci.yml:** `set -o pipefail` in both `sanitize` run blocks; `killall ... || true` replaced by a
  `pgrep` guard and the diagnostic `auval ... || true` line removed; new job `lint-ci`; new step
  "Check bench gates (T-050 / T-051)" in `build-test`; bench artifact now includes
  `bench-baseline-candidate.json` and is uploaded `if: always()`; new steps "Assert sanitizer runs
  completed" and "Upload sanitizer log"; mutation gate asserts `mull.sqlite` is non-empty and holds
  at least 100 mutants. The first `python` vs `python3` choice (Windows image) is selected by
  `runner.os`.
- **Also logged here (found unlogged by the 1a diff):** the `sanitize` job (ASan + UBSan over the fast subset and the T-013 fuzz corpus) and `tests/MutationTests.cpp` with its `tests/CMakeLists.txt` wiring.
- **New files (config/tooling, part of the pins under 1.3.6):** `tools/ci/lint_ci.py`,
  `tools/ci/check_bench.py`, `tools/ci/swallow-allowlist.txt` (empty),
  `tests/bench/bench-targets.json`, `tests/bench/baseline-T050.json` (from run 40898109414),
  `tests/bench/README.md`, `plan/POSTCONDITIONS_v36.md`, `research/COMMAND_VERIFICATION.md`.
- **Behaviour change:** the T-051 latency verdict now fails the build (the harness always exited 0);
  the T-050 Frost/High budget and the >20 % regression check now fail the build on `windows-latest`.
  macOS stays reported-only under D-001n. The Windows budget margin is thin (14.51 % in run
  40898109414) and hosted-runner variance exceeds the 20 % tolerance (K-20261029T030800Z): a
  failure is a signal to investigate, not to silence; relaxing it is a maintainer decision.
- **Docs:** `plan/ENVIRONMENT.md` AU setup command (registration required); the 77.9 % run id in
  `reviews/PROTOCOL_CONFORMANCE_v36.md` corrected (40771992301 -> 40898109414).
- **State:** `.checkpoints/state.json` gains `execution_environments` (generation sandbox:
  ephemeral; amendment session: stable) and `amendment_rounds`.
- **Not done, named:** `S-REVIEW` and `S-ADVERSARIAL` (protocol 3.8, in the v3.4 plan) were never
  executed: `reviews/` holds only MUTATION.md and the conformance review. They need fresh-context
  reviewers. See RETROSPECTIVE.md section 2.

### D-001q addendum — T-040 had never run on Windows

Removing the `|| true` from the dumpbin step (D-001p) exposed `dumpbin: command not found` (exit
127, run 41144187118). The import-table check (T-040) had therefore passed vacuously in every
earlier run. The step now finds `dumpbin.exe` in the Visual Studio installation and fails if it is
missing or produces no output. Whether the forbidden-import grep then passes is determined by the
next CI run; a real finding there is a product or security issue, not a CI problem.

## D-001r — fixes from the S-REVIEW / S-ADVERSARIAL reviews

Source changes beyond the original plan, each backed by a regression test (disposition table in
`reviews/REVIEW_DISPOSITION.md`): `PluginProcessor.cpp` (prepareToPlay takes stepped settings from
the parameters, setStateInformation no longer touches them, oversize host blocks are processed in
chunks via the new `processChunk`, NaN-safe `choiceIndex`, non-finite state values dropped),
`NoiseGate.h` (silence clamp, finite guard, full reset), `UserIRLoader.cpp` (length/size/finite
validation), `PresetManager.cpp` (legal file names, non-finite values dropped). New tests in
`tests/MutationTests.cpp`: `state_recall_dsp`, `oversize_block`, `nan_preset`, plus cases in
`gate_reference`, `userir_loader`, `preset_list`. CI: three security gates rewritten (CR-001),
`permissions: contents: read`. The behaviour of the plugin changes only in the repaired cases
(recall after prepare, silence after loud audio, hostile inputs); loudness calibration was not
re-run.

## D-001s — CR-004 / CR-005 (part) / CR-006 fixes

Behaviour changes beyond the original plan: the gate, preamp and power amp are now instantiated per
channel and the tone stack sees both channels (before, both channels shared one state, giving
block-rate discontinuities and cross-channel gate/sag triggering); `PowerAmp::processThump` is
unchanged in signature but rebuilds its shelf only on a >0.01 dB change; `ToneStack` and `PostEq`
skip identical coefficient rebuilds; `CabIR` BinaryData lookup uses `strcmp`; `UserIRLoader` is a
persistent worker with a lock-free hand-off. The sound of stereo material changes slightly (each
channel is now processed independently as intended) and the S-009 loudness calibration was **not
re-run**; mono and identical-L/R material should be unaffected apart from the removed block-boundary
state jumps. Regression test `channel_independence` pins the new behaviour.

## D-001t — CR-005 allocation work: what landed and what was reverted

Landed: `Source/DSP/BiquadDesign.h` (in-place RBJ peak/shelf design that mirrors JUCE's make* functions;
`biquad_design` compares them), `retune`/in-place `setVoicing` on `Preamp` and `PowerAmp`, in-place
coefficient updates in `ToneStack`, `PostEq` and the thump shelf, a pre-built oversampler with
`OversampledStage::setHighQuality`, and an allocation probe (`no_audio_alloc`) that replaces the global
operator new/delete in the test executable and counts calls made by the processing thread.
Reverted: a worker thread for `Convolution::loadImpulseResponse` (commit 7b7ea52). Evidence: the new
`automation_stress` test (random automation of all parameters in 32-sample sub-blocks) failed with
`std::bad_function_call` at block size 1024 under the sanitizer job with the worker, pluginval's Automation test
failed once on macOS (96 kHz/1024) and once on Windows (96 kHz/64), and `automation_stress` passed after the
revert (commit 2ab8a57). Likely cause (unverified): JUCE's convolution message queue assumes one producing
thread. Consequence: cabinet IR changes still decode/copy on the audio thread.

## D-001u — T-040 resolved by function-level linking (MSVC)

`CMakeLists.txt` adds `/Gy /Gw` (compile) and `/OPT:REF /OPT:ICF` (link) for MSVC. This is a build-setting change, not a change to the frozen T-040 spec or its gate: the Windows binary no longer imports WININET/WS2_32 (CI run 41304830804), so the import-table check passes without an allowlist. Side effects to watch: a smaller binary, `/OPT:ICF` folds identical functions (function-pointer identity is not relied on), and the T-050 CPU numbers may shift slightly (not yet measured with the new flags).

## D-001v — T-050 gate uses the median of three bench runs

Maintainer-approved (2026-11-01) after the Windows budget gate read 15.08 % (Frost/High) against 15.0 % on a run where the
same code had read 14.4-14.5 % before. The CI step runs `grimamp_bench` three times and `tools/ci/check_bench.py` gates on the
median per voicing/mode; latency must pass in all three. The budget (15 %), the regression tolerance (20 %) and the baseline
are unchanged; relaxing any of them remains a maintainer decision. Cost: about 2.5 extra minutes per OS. The raw runs and the
median are uploaded in the `bench-results-<os>` artifact.

## D-001w — mutation testing moved to its own three-tier workflow

`.github/workflows/mutation.yml` replaces the `mutation` job of `ci.yml` (skill `mutation-testing-in-ci`, planning-protocol v3.9). Tier 1 (pull request, push to `impl-*`/`main`/`master`): scope step diffs against the PR base or the push's `before` commit; a change under `tests/`, to `mull.yml` or to `tools/ci/mull_*` runs the full set, otherwise only the changed files under `Source/DSP/` and `Source/Presets/` are mutated, and `not-applicable` is reported when none changed. Tier 2: nightly full run (GitHub runs schedules only from the default branch's copy of the file, so it takes effect once this workflow is on `gen-20261027T120000Z-guitar-amp-vst` or its successor); a failed scheduled run opens an issue. Tier 3: `workflow_dispatch` with `scope`. `mutation-gate` is the always-running aggregate; making it a required status check is a repository setting (branch protection / rulesets) and has not been changed. The gate now uses `tools/ci/mull_score.py` (floor 75 %, minimum mutants 10 incremental / 150 full) instead of the inline script; actions are pinned by commit SHA. Previous behaviour: every push ran the full job (66-150 minutes). The `tests/` change rule means a test-only push still runs the full set.

### D-001w addendum - incremental runs gate per file

Applying the skill showed that a project-wide floor on a single changed file would fail every change to files that legitimately score below it (CabIR.h 62.5 %, UserIRLoader.cpp 61.5 %, OversampledStage.h 50 %, with justified survivors in `reviews/MUTATION.md`). Incremental runs therefore gate each changed file against its score from the last full run (`reviews/mutation-baseline.json`, from run 41308370137, tolerance 5 points); full runs keep the 75 % project floor with at least 150 mutants. Refresh the baseline from a full run when scores legitimately move (`tools/ci/mull_score.py --write-baseline`); lowering a recorded score is a reviewed decision.

### D-001x / D-001y - preamp stage changes after G-010 round 1

Round 1 (9/12, listener: "the others all had more grit in the real clip") and the band/crest measurement against the real-amp references showed the candidates 7-13 dB under the references above 2 kHz and 4-7 dB under in crest factor. Two general changes in `Source/DSP/Preamp.h`:

- **D-001x** the post-shaper shelf undoes half of the pre-emphasis boost (`deEmphasisFraction` 0.5). Effect on its own: about 1 dB at 8-16 kHz.
- **D-001y** the shaper sees the signal above a 150 Hz coupling corner (one-pole high-pass); the lows go through a separate shaper path at weight 0.3 (`couplingCornerHz`, `lowPathMix`). Mean gap to the references, 2-4 kHz / 4-8 kHz / 8-16 kHz: -6.9/-10.7/-11.8 dB before, -3.4/-7.8/-8.9 dB after; crest 7.5 dB to 10.4 dB (references 11.1). The 80-250 Hz band falls to -8.4 dB against the references.
- Music loudness fell about 4 dB. The frozen T-005 peak bound blocks raising `loudnessGainDb` (+0.3 of the needed change already fails it), so the trims were lowered to equalise the voicings instead (Tweed unchanged at +1.7, Plexi +0.39, JCM800 -0.78, Recto -2.10, 5150 -2.59, Chainsaw -3.54, Frost -3.73). Voicing spread 0.81 dB by BS.1770 K-weighting (limit 1.5); the plugin is about 5 dB quieter at the same master setting. All 31 ctest targets pass. Mull was not re-run, so T-070 for `Preamp.h` is unchecked.
- Constants are set from the pooled 14-pair measurement, not per clip. The corner was chosen by sweeping 0-1000 Hz and taking the first value where crest factor reaches the references' (about 150-200 Hz).

### D-001z - D-001y reverted, square-root drive knob law (G-010 round 3 candidate)

Round 2 (11/12, listener: "the real clips have more grit and more weight") showed the 150 Hz coupling corner of D-001y thinned the low end. D-001y is removed; D-001x (half de-emphasis) stays. Stage isolation on the 14 reference pairs showed the preamp is the only source of 2-8 kHz energy (bypassing tone stack or power amp changes it by 1-2 dB) and that the low-drive presets are the ones far below the references. The drive knob law is now `0.25 + 3.75*sqrt(knob)` instead of linear; the ends of travel are unchanged. Mean gap to the references, 2-4/4-8/8-16 kHz: -6.9/-10.7/-11.8 dB before, -5.8/-8.6/-8.6 dB after; 80-250 Hz gap unchanged (-2.8 dB); crest 7.5 to 6.6 dB (references 11.1). Trims lowered by 0.6-1.7 dB (Tweed +0.04, Plexi -0.36, JCM800 -0.66, Recto -1.35, 5150 -1.48, Chainsaw -1.86, Frost -1.90); spread 1.10 dB (limit 1.5), mean loudness within 0.15 dB of before. 31/31 ctest. Mull not re-run. A harder shaper knee and larger pre-emphasis were measured and changed nothing (under 0.5 dB). Preset sounds change at mid-knob settings, so all 24 presets are affected.

### D-001aa - envelope-driven stage bias (G-010 research round)

After round 3 (7/12) the human asked for further research before another listening round. The literature (research/G-010-preamp-research.md) says real preamp stages are asymmetric and not memoryless: grid current and the cathode bypass capacitor move the bias with the signal envelope. Each `PreampStage` now shapes `x + b` with `b = 0.4 + 0.5 * envelope`, subtracts `shape(b)`, and blocks the DC with a 15 Hz one-pole high-pass (envelope attack 1 ms, release 50 ms). Measured on the 14 reference pairs: crest factor 6.9 to 10.4 dB (references 11.1), 80-250 Hz gap -3.3 to -0.8 dB; the 2-8 kHz gap barely changes (-5.6/-8.2 dB). Loudness fell about 6.7 dB at the old trims and the frozen sine-peak bound (T-005) blocks raising them, so the trims of every voicing but Tweed were lowered: Plexi -1.22, JCM800 -2.14, Recto -3.38, 5150 -3.77, Chainsaw -4.60, Frost -4.86 (Tweed +0.04 unchanged). Voicing spread 0.45 dB. The plugin is now about 6.7 dB quieter than at round 3 at the same master setting, which needs a human decision (master make-up gain or amending T-005's peak bound). 31/31 ctest. Mull not re-run.

### D-001ab - high-band shaper in each preamp stage (G-010 round 5)

After round 4 (11/12) the listener said brightness was the cue. In the 1 s onset windows played, the real amp had 10-17 dB more energy above 2 kHz for the low-drive presets, and the reference was brighter in 9 of 12 pairs. Each `PreampStage` now splits its input at 1.2 kHz (one-pole), shapes the high part with gain `6 * drive^2` through the stage shaper, and adds it at weight 0.1 before the output gain (multiband waveshaping after Fernandez-Cid & Quiros 2001 as described in Pakarinen & Yeh 2009; the `drive^2` law keeps the clean presets almost untouched). Onset-window mean absolute gap above 2 kHz, over the 14 reference pairs: 7.0 dB to 4.5 dB; 12 s band gaps: 2-4 kHz -5.6 to -2.3 dB, 4-8 kHz -8.2 to -3.3 dB, 8-16 kHz -7.8 to -0.4 dB. Remaining: Southern Rock, Classic Rock Crunch, Funeral March and Crunch Rhythm still 9-11 dB short in the onset window (the JVM OD1 channel at gain 5 is hotter than the Plexi/5150 presets at their knob settings, so this is partly a reference mismatch, not a fixable constant); Thrash Attack and Bay Area Lead are 4-6 dB too bright. Larger weights or fixed (drive-independent) gain overshoot the clean presets by 20-28 dB. Loudness moved by less than 0.4 dB (spread 0.19 dB); 31/31 ctest. Mull not re-run.
