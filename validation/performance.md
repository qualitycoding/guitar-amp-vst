# Performance & Latency Validation (S-013, T-050/T-051)

Run: `grimamp_bench --bench` on the local sandbox
(2-core shared Intel Xeon Platinum vCPU, GCC 12.2 -O2/-flto Release, 2026-10-28).
Numbers are informational — see DEVIATIONS.md D-001c: the T-050 budget applies to
the CI runner class, which plan/ENVIRONMENT.md does not pin.

## T-051 latency report (PASS)

| Mode | reported (`getLatencySamples`) | measured impulse delay | bound | verdict |
|---|---|---|---|---|
| Eco  | 0 | 0 | reported == 0 and |reported−measured| ≤ 1 | PASS |
| High | 4 | 5 | ≤ 144 samples (≈3 ms @ 48 k) | PASS |

Measurement method (refined 2026-10-28, D-001h): small-signal impulse
(−60 dBFS) streamed in 256-sample blocks (host contract — blocks never exceed
the prepared maximum), peak of |output| over 8192 samples, after
`pumpUntilCabReady` plus 64 settle blocks (the convolution engine crossfades
IR swaps internally; measuring during that tail jittered the peak by 1–2
samples). The gate is bypassed for the measurement so the quiet impulse is
not gated.

Why small-signal: latency is a linear-system property. A full-scale impulse
drives the plan-mandated preamp waveshapers (D-002/D-003) hard, and the peak
of |output| then reports nonlinear energy spread (~4 samples @ 48 k), not
processing latency. In the linear regime the pre/post-emphasis shelf pairs
cancel exactly (verified analytically: same-frequency ±gain shelves are
exact inverses) and the measured delay is the true processing latency —
0 samples in Eco (zero-latency convolution, D-004), 4–5 samples in High
(oversampling), matching the reported values within the ±1-sample bound.

## T-050 CPU budget — CI runner results (authoritative)

`grimamp_bench --bench` on the GitHub-hosted runners, 60 s DI render, 48 kHz / 256-sample
blocks, all 7 voicings x both modes, from the `bench-results-<os>` artifacts of two CI runs on
branch `impl-run-20261027T120000Z-guitar-amp-vst`:

- run 40771992301 (before the noise-gate fix, D-001m)
- run 40898109414 (after it, with the gate now actually attenuating)

Runners: `windows-latest` (AMD EPYC, 4 cpus; the processor model differed between runs: 9V74
then 7763) and `macos-latest` (Apple M1 virtual, 3 cpus). Budget: worst case (Frost, High)
<= 15 % of one core.

High oversampling, mean CPU % of one core:

| Voicing | Windows 40771992301 | Windows 40898109414 | macOS 40771992301 | macOS 40898109414 |
|---|---|---|---|---|
| Tweed | 10.8 | 13.3 | 25.7 | 17.0 |
| Plexi | 11.0 | 13.5 | 26.1 | 17.4 |
| JCM800 | 11.5 | 13.7 | 31.6 | 18.0 |
| Recto | 11.3 | 14.1 | 35.6 | 18.3 |
| 5150 | 11.4 | 14.1 | 18.0 | 17.6 |
| Chainsaw | 11.6 | 14.5 | 18.3 | 17.8 |
| Frost | 11.7 | 14.5 | 18.7 | 17.3 |

Eco mode: Windows 1.7–1.8 % / 2.4–3.3 %, macOS 1.4–3.1 % / 1.3–1.5 % (runs in the
order above) — far inside the budget everywhere.

### Verdicts

- **Eco: PASS** on both runners.
- **High, Windows: PASS in both runs** (worst case Frost/High 11.7 % then 14.5 %), but the
  second run is within 0.5 points of the budget, so the margin is thin and varies with the
  runner hardware lottery (different EPYC model, different gate behaviour).
- **High, macOS: over budget in the first run (18.0–35.6 %, Frost/High 18.7 %), 17.0–18.3 % in
  the second** — still above 15 % on every voicing. The shared virtual M1 is noisy (the first
  run's Recto/High was twice the second's), but even the better run misses the budget.
- **Decision (D-001n, maintainer, 2026-10-30):** leave the DSP and the budget as they are; the
  bench step stays informational and the macOS miss is recorded, not fixed. The 15 % figure is
  therefore a pass on Windows runners and a known, accepted miss on macOS virtual hardware.
- **>20 % regression gate: not evaluated.** It needs a baseline from `main`; no `main` bench
  artifact exists yet. Until one does, the check cannot fail.

The T-051 latency verdicts are identical in both CI runs (Eco 0/0, High 4/5, both PASS).

## T-050 sandbox numbers (historical, informational)

60 s DI render, 48 kHz / 256-sample blocks, all 7 voicings × both modes:

| Voicing | Eco | High |
|---|---|---|
| Tweed   | 3.62 % | 22.09 % |
| Plexi   | 3.52 % | 22.51 % |
| JCM800  | 3.55 % | 22.66 % |
| Recto   | 3.55 % | 22.90 % |
| 5150    | 3.56 % | 22.90 % |
| Chainsaw| 3.63 % | 23.00 % |
| Frost   | 3.58 % | 23.08 % |

Worst case (Frost, High — the spec's worst case): 23.08 % of one core on this
sandbox vs the ≤15 % budget. **This sandbox is not the CI runner class** (shared
2-vCPU VM, ~2015-era Xeon Platinum with noisy neighbours, GCC vs MSVC/clang);
the budget verdict and the >20 % regression gate must be read from the CI
artifact (BLOCKED.md B-5). Eco mode is ≈3.6 % worst-case — comfortably inside
the budget on any runner class.

Raw artifact: `bench-results.json` (emitted per run; CI attaches it).
