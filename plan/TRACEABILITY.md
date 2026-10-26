# TRACEABILITY — Requirement → Evidence → Step

## Success criteria (from intake) → tests → producing steps

| # | Success criterion (measurable) | Evidence | Producing step(s) |
|---|---|---|---|
| SC-1 | Plugin loads in Ableton Live 12 (Win + mac) as VST3 without scan failure | T-020, T-030 | S-002, S-014, S-015 |
| SC-2 | Tones span 70s blues rock → high-gain black metal: 7 voicings, monotonic gain ordering, loudness-matched | T-006, loudness spread ≤ ±1.5 dB (S-009), G-010 sign-off | S-005, S-009, S-015 |
| SC-3 | 24 genre-specific factory presets, all loadable and round-trippable | T-008 | S-011 |
| SC-4 | Every user-facing control automatable in Live (stable IDs, stepped + continuous) | T-002, T-031 | S-003, S-012, S-015 |
| SC-5 | Automation is click-free and preset loads spawn no automation lanes | T-010, T-011, T-032 | S-004–S-008 (smoothing), S-012, S-015 |
| SC-6 | Session state recalls exactly in Live | T-009, T-033 | S-003, S-015 |
| SC-7 | Robust: no NaN/Inf, denormal-safe, fuzz-safe preset/IR handling | T-004, T-005, T-007, T-012, T-013 | S-004–S-008, S-011 |
| SC-8 | Performance: ≤ 15% core per instance @ 48k/256 High mode; ≤ 3 ms latency High, 0 Eco | T-050, T-051 | S-013 |
| SC-9 | Validated by pluginval strictness 5 + Steinberg validator in CI on both OSes | T-020, T-021 | S-002, S-014 |
| SC-10 | No network/telemetry; local file handling only | T-040, T-041 | S-014 |
| SC-11 | Installers install/scan/run/uninstall cleanly on both OSes | T-060 | S-016 |
| SC-12 | Release happens only with human sign-off | G-002 record | S-017 |

## Test → requirement reverse map

T-001→SC-1 · T-002→SC-4 · T-003→SC-4 · T-004→SC-7 · T-005→SC-7 · T-006→SC-2 · T-007→SC-7 · T-008→SC-3 · T-009→SC-6 · T-010→SC-5 · T-011→SC-5 · T-012→SC-7 · T-013→SC-7 · T-020→SC-1/SC-9 · T-021→SC-9 · T-030→SC-1 · T-031→SC-4 · T-032→SC-5 · T-033→SC-6 · T-034→SC-1 · T-040→SC-10 · T-041→SC-10 · T-050→SC-8 · T-051→SC-8 · T-060→SC-11

Every success criterion has ≥ 1 test; every test maps to ≥ 1 criterion. No N/A-section artifacts are referenced.
