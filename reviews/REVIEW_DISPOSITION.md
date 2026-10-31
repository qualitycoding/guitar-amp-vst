# REVIEW DISPOSITION — S-REVIEW and S-ADVERSARIAL findings

Reports: `reviews/CODE_REVIEW.md` (28 findings: 2 Critical, 6 High, 11 Medium, 9 Low) and
`reviews/ADVERSARIAL_REVIEW.md` (1 High, 6 Medium, 3 Low, no Critical). Both were produced by
fresh-context reviewers from code reading alone (no JUCE checkout, no build); findings that rest on
JUCE internals are marked unverified in the reports. Protocol 3.8: Critical/High findings must be
fixed or explicitly accepted before any release step. Status as of this commit:

## Fixed in this round (with regression tests)

| ID | Sev | Fix | Test |
|---|---|---|---|
| CR-001 | Critical | Three CI gates (T-041, T-040 static, T-040 Windows imports) used `! grep` under errexit and could never fail. Rewritten as `if grep; then exit 1; fi` with a `test -d` post-condition; lint rule + self-test added. Verified by reproducing the shell behaviour locally. | `lint_ci.py --selftest` |
| CR-002 | Critical | `setStateInformation` no longer writes the stepped-setting mirrors; `prepareToPlay` takes them from the parameters. A restore after prepare is now detected by `applySteppedParams`. | `state_recall_dsp` |
| CR-003 | High | Gate RMS sum clamped/re-zeroed in silence, envelope guarded with `isfinite`, `reset()` clears the FIFO (CR-021). | `gate_reference` (exact-zero silence case) |
| CR-007 | High (part) | IR loader: 64-bit length clamp, resampled length capped, `read()` result checked, non-finite samples rejected. | `userir_loader` (low-rate case) |
| CR-008 | High | `processBlock` splits blocks larger than the prepared maximum into chunks. | `oversize_block` (also under ASan/UBSan) |
| CR-013 / AR-008 | Medium | Preset names are legalised to file names; a result outside the presets directory is refused. | `preset_list` |
| CR-017 (part) | Medium | Workflow token limited to `contents: read`. | n/a |
| AR-001 | High | Non-finite `value` attributes are dropped on preset load and session restore; `choiceIndex` is NaN-safe. | `nan_preset` |

## Open - needs a decision or a larger change (not hidden)

| ID | Sev | Why not done now | Proposed next step |
|---|---|---|---|
| **T-040 (new finding)** | **High / human gate** | With the gate now able to fail, the Windows import table shows `WININET.dll` and `WS2_32.dll` in `GrimAmp.vst3` (JUCE links its network code into every binary; `JUCE_USE_CURL=0` does not remove it). Frozen spec T-040 asks for an allowlist check. Allowlisting these DLLs relaxes a security test, so it is the maintainer's call. | Decide: (a) allowlist the two DLLs with this explanation and keep the static source gate, (b) prove no network function is reachable (link-time garbage collection / map file review), or (c) patch JUCE. CI stays red on this step until decided. |
| CR-004 | High | Per-channel filter/gate/sag state means restructuring the DSP chain; changes the sound and the loudness calibration (S-009) and touches frozen-test expectations. | Plan as a new step with a loudness re-run; decide scope. |
| CR-005 | High | Real-time allocations in `processBlock` (coefficient objects, oversampling rebuild, IR copy). Larger refactor; correctness unaffected. | New step; add a real-time-safety test (allocation hook) first. |
| CR-006 | High | Loader locking / lost request. Needs a redesign of `UserIRLoader`. Partly unverified. | New step with a stress test; consider the CR-016 UI decision first. |
| CR-009 / D-001m | Medium | 4096-sample box vs documented 5 ms window. Changes gate behaviour. | Maintainer decision (already flagged). |
| CR-010, AR-004, CR-011, AR-002, AR-003 | Medium | Per-block smoothing staircase, crossfade jump, mono High-mode artefacts. Audible quality items; need DSP changes and listening checks. | Backlog with tests first (jump-size and mono tests proposed in the adversarial report). |
| CR-012, AR-010 | Medium | No `reset()` override, tail length 0. | Small change; bundle with CR-004/005. |
| AR-005 | Medium | Float32 biquad instability at 8/11.025 kHz and 768 kHz. A-010 only promises 44.1-192 kHz. | Accept or reject unsupported rates in `isBusesLayoutSupported`/`prepareToPlay`. |
| CR-014, CR-015, CR-016, AR-006 | Medium | Parser limits, loader format list, user IR slot has no UI and is not persisted. Scope decisions. | Maintainer decision. |
| CR-017 (rest), CR-020 | Medium/Low | Mutable action tags, `curl | sudo bash` for Mull, unpinned Mull, older actions (checkout v4 vs v7). Pinning by SHA is a policy decision with churn. | Pin by commit SHA in a dedicated change. |
| CR-018 | Medium | The freeze manifest hashes only `TEST_SPEC.md`, not the frozen test code. Changing the manifest is a protocol-level (2E.4) action. | Maintainer decision. |
| CR-019, AR-007, AR-009 | Medium/Low | Coverage gaps; T-011(b) cannot fail; macOS preset directory path. | Backlog. |
| CR-021..CR-028 | Low | Cleanup items. | Backlog. |

## Release impact

Protocol 3.8 says Critical/High breaks block release. Of the High findings, CR-004, CR-005, CR-006
and the new T-040 finding remain open, so **G-002 (release) should not be requested** until each is
fixed or explicitly accepted as a risk by the maintainer.
