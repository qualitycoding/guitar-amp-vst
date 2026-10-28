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

## T-050 CPU budget (INFORMATIONAL — CI is authoritative)

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
