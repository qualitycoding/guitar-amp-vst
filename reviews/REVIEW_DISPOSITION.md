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
| CR-004 | High | One `NoiseGate`, `Preamp` and `PowerAmp` per channel (`std::array<...,2>`); the tone stack now processes the whole block through its `ProcessorDuplicator` (before, each channel went through a one-channel block and shared filter 0); each channel's thump shelf uses its own envelope (an intermediate version that shared one shelf failed the test and was corrected). | `channel_independence` (Eco and High: a channel's output is unchanged when the other channel's signal is removed) |
| CR-006 | High | `UserIRLoader` rewritten: persistent worker thread, lock held only to store the latest request, supersede checked between steps, cooperative shutdown, finished buffer handed to the audio thread through one atomic slot (no mutex in `take()`). | `userir_loader` (supersede, non-blocking call, prompt shutdown) |
| CR-005 (part) | High | Tone stack and post-EQ rebuild coefficients only when a knob moved; BinaryData lookup uses `strcmp` instead of building `juce::String`s; thump shelf rebuilt only on >0.01 dB change. | existing reference tests (`tonestack_reference`, `poweramp_reference`) |
| AR-001 | High | Non-finite `value` attributes are dropped on preset load and session restore; `choiceIndex` is NaN-safe. | `nan_preset` |

## Open - needs a decision or a larger change (not hidden)

| ID | Sev | Why not done now | Proposed next step |
|---|---|---|---|
| **T-040 (new finding)** | **High / human gate** | With the gate now able to fail, the Windows import table shows `WININET.dll` and `WS2_32.dll` in `GrimAmp.vst3` (JUCE links its network code into every binary; `JUCE_USE_CURL=0` does not remove it). Frozen spec T-040 asks for an allowlist check. Allowlisting these DLLs relaxes a security test, so it is the maintainer's call. | Decide: (a) allowlist the two DLLs with this explanation and keep the static source gate, (b) prove no network function is reachable (link-time garbage collection / map file review), or (c) patch JUCE. CI stays red on this step until decided. **Maintainer chose (b) on 2026-10-31**; function-level evidence is in `reviews/T040_EVIDENCE.md` (no socket/connect/send/recv imports; WinInet HTTP/FTP client functions and getaddrinfo are present); the reachability proof is still open. |
| CR-005 (rest) | High | Tracked as issues #2 (cabinet IR loads) and #3 (setLatencySamples). Fixed: allocation-free tone stack, post-EQ, thump shelf, voicing change and Eco/High switch (in-place biquad design, oversampler built once; `no_audio_alloc` probe, `biquad_design`, `retune_equivalence`). **Still open:** cabinet IR changes (`Convolution::loadImpulseResponse` decodes/copies on the audio thread) and `setLatencySamples` from the audio thread (JUCE's own wrapper behaviour, unverified on hosts). A worker thread for IR loads was implemented and reverted: `automation_stress` failed with `std::bad_function_call` (and pluginval failed intermittently on macOS and Windows) while it was in, and passed without it. | Needs a design that keeps every `Convolution` call on one thread (e.g. the message thread via a lock-free request, with its own stress test), or a different convolution engine. |
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

Protocol 3.8 says Critical/High breaks block release. Of the High findings, CR-005 (rest)
and the new T-040 finding remain open, so **G-002 (release) should not be requested** until each is
fixed or explicitly accepted as a risk by the maintainer. (Updated after the CR-004/005/006 round: the remaining High items are CR-005 (rest) and the T-040 decision.)
