#!/usr/bin/env python3
"""
gen_di.py — generate the checked-in synthetic DI guitar clip used by the
benchmark (T-050) and the voicing/preset renders (S-009/S-015).

60 s, 48 kHz, mono, 16-bit. Synthetic additive "riff": palm-muted chugs,
power-chord stabs and a short lead phrase. It is a test signal, not a musical
performance; real DI captures can replace it later (then re-run S-009).
"""
import math
import os
import random
import struct
import wave

SR = 48000
SECONDS = 60
OUT = os.path.join(os.path.dirname(__file__), "..", "tests", "bench", "di.wav")


def note_freq(semitone_from_a440):
    return 440.0 * 2 ** (semitone_from_a440 / 12.0)


def guitar_string(t, freq, dur, mute=False, bright=1.0):
    """Very rough plucked-string additive model with fast brightness decay."""
    out = 0.0
    n_harm = 12
    for h in range(1, n_harm + 1):
        hf = freq * h * (1 + 0.0004 * h * h)          # inharmonicity
        if hf > 12000:
            break
        env = math.exp(-t * (2.0 + 0.35 * h) * (6.0 if mute else 1.6))
        amp = (1.0 / h) * env
        out += amp * math.sin(2 * math.pi * hf * t + h * 0.3)
    # pick attack
    out *= min(1.0, t * 4000.0)
    return out * 0.12 * bright


def main():
    rng = random.Random(7)
    n = SR * SECONDS
    samples = [0.0] * n

    # riff: low chugs on the beat with occasional power chord stabs (E drop-D-ish)
    riff_notes = [-29, -29, -22, -29, -26, -29, -29, -34]   # around drop D
    beat = 0.5
    t = 0.25
    while t < SECONDS - 2.0:
        pos = int(t * SR)
        note = riff_notes[int(t / beat) % len(riff_notes)]
        dur = 0.22 if int(t / beat) % 4 != 3 else 0.45
        mute = int(t / beat) % 2 == 1
        f = note_freq(note)
        length = int(dur * SR)
        for i in range(length):
            if pos + i < n:
                samples[pos + i] += guitar_string(i / SR, f, dur, mute=mute)
        t += beat

    # lead phrase every 8 s
    lead_scale = [0, 3, 5, 7, 10, 12]
    for start_bar in range(8, SECONDS - 8, 8):
        base = start_bar + rng.choice([0.0, 0.5])
        for k in range(6):
            st = base + k * 0.45
            pos = int(st * SR)
            note = 5 + rng.choice(lead_scale)
            f = note_freq(note)
            length = int(0.4 * SR)
            for i in range(length):
                if pos + i < n:
                    samples[pos + i] += guitar_string(i / SR, f, 0.4, mute=False, bright=0.8)

    # gentle fade in/out and soft clip to keep the bench signal hot but sane
    fade = SR // 4
    for i in range(fade):
        g = i / fade
        samples[i] *= g
        samples[-1 - i] *= g
    peak = max(abs(s) for s in samples) or 1.0
    scale = 0.7 / peak
    samples = [math.tanh(s * scale * 1.4) * 0.8 for s in samples]

    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with wave.open(OUT, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(b"".join(
            struct.pack("<h", max(-32768, min(32767, int(round(s * 32767)))))
            for s in samples))
    print(f"wrote {OUT} ({SECONDS}s @ {SR} Hz)")


if __name__ == "__main__":
    main()
