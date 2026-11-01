# Changelog

## v1.0.0 (draft — pending gate G-002)

- Initial release: rock & metal guitar amp simulator (VST3/AU/Standalone).
- 7 amp voicings: Tweed, Plexi, JCM800, Recto, 5150, Chainsaw, Frost.
- TS-style boost, noise gate with hysteresis, tone stack (bass/mid/treble/
  presence/depth), power-amp saturation with sag, cabinet IR convolution with
  8 factory IRs (close/off-axis mic crossfade) + user IR slot.
- 24 factory presets across 70s blues rock through black metal, doom, djent.
- Eco (zero-latency) / High (4x oversampled) processing modes.
- Every parameter host-automatable with stable kebab-case IDs; preset changes
  produce no automation gestures.
- Frozen test suite: 13 unit/dsp ctest targets, fuzz (1000 malformed presets),
  pluginval strictness 5, security greps, mutation testing (Mull), benchmark
  harness.
