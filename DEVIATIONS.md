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
