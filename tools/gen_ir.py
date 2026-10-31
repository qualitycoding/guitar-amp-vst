#!/usr/bin/env python3
"""
gen_ir.py — generate the 8 factory cabinet IR pairs (S-008 placeholders).

Method (K-20261004T162311Z): a dB-vs-log-frequency cabinet table is converted
to a minimum-phase FIR via the real-cepstrum folding trick:
  1. interpolate the table onto the rFFT frequency grid, dB -> natural log
  2. irfft -> real cepstrum
  3. lifter: double the positive-time part, zero the negative-time part
  4. rfft -> complex log spectrum; exp; irfft -> minimum-phase IR
These are synthetic placeholders; S-016 replaces them with original/CC0
captures (see Resources/ir/PROVENANCE.md).
"""
import cmath
import math
import os
import struct
import wave

N = 2048                  # taps
SR = 44100.0              # authored rate; Convolution resamples to host rate
OUT = os.path.join(os.path.dirname(__file__), "..", "Resources", "ir")


def irfft(X):
    """Inverse of numpy.fft.rfft for even N."""
    nf = N // 2 + 1
    assert len(X) == nf
    full = [X[k] for k in range(nf)] + [X[m].conjugate() for m in range(nf - 2, 0, -1)]
    return [z.real for z in inverse_dft(full)]


def inverse_dft(X):
    n = len(X)
    return [sum(X[k] * cmath.exp(2j * math.pi * j * k / n) for k in range(n)) / n
            for j in range(n)]


def rfft(x):
    n = len(x)
    X = inverse_dft([complex(v, 0.0) for v in x])
    return [X[k] for k in range(n // 2 + 1)]


def table_to_minphase_ir(table, n=N):
    """table: list of (freq_hz, db) control points, ascending."""
    freqs = [0.0] + [SR * k / n for k in range(1, n // 2)] + [SR / 2.0]

    def interp_db(f):
        if f <= table[0][0]:
            return table[0][1]
        if f >= table[-1][0]:
            return table[-1][1]
        for i in range(1, len(table)):
            if f <= table[i][0]:
                f0, db0 = table[i - 1]
                f1, db1 = table[i]
                t = (math.log(max(f, 1.0)) - math.log(f0)) / (math.log(f1) - math.log(f0)) \
                    if f1 > f0 else 0.0
                return db0 + t * (db1 - db0)
        return table[-1][1]

    logmag = [interp_db(f) * math.log(10.0) / 20.0 for f in freqs]
    cep = irfft(logmag)
    # minimum-phase liftering
    half = n // 2
    cmin = [0.0] * n
    cmin[0] = cep[0]
    for i in range(1, half):
        cmin[i] = 2.0 * cep[i]
    cmin[half] = cep[half]

    logspec = rfft(cmin)
    h = irfft([cmath.exp(z) for z in logspec])
    # unity-peak normalisation
    peak = max(abs(v) for v in h) or 1.0
    return [v / peak for v in h]


def write_wav(path, samples, rate=SR):
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(int(rate))
        frames = b"".join(
            struct.pack("<h", max(-32768, min(32767, int(round(v * 32767)))))
            for v in samples)
        w.writeframes(frames)


# 8 cabinets x (close-mic table, off-axis table).
# Tables are (freq_hz, dB) points describing a plausible cabinet magnitude curve.
CABS = [
    # 1 British 4x12: scooped-ish mids, aggressive top
    ("cab1", [ (80, 3), (120, 4), (400, 1), (900, -3), (2500, 1), (4200, 2), (6500, -1), (9000, -6), (12000, -12) ]),
    # 2 American 4x12: big low end, smooth top
    ("cab2", [ (70, 4), (150, 3), (500, 0), (1200, 2), (3000, 0), (5000, -2), (8000, -7), (12000, -11) ]),
    # 3 Vintage 2x12: mid-forward, early roll-off
    ("cab3", [ (90, 1), (200, 2), (700, 3), (1800, 1), (3500, -2), (6000, -8), (9000, -14) ]),
    # 4 Tweed 1x12: lo-fi, rolled top, woody bump
    ("cab4", [ (80, 5), (180, 2), (700, 4), (2000, 0), (4000, -6), (7000, -14), (10000, -20) ]),
    # 5 British 2x12: balanced, chime
    ("cab5", [ (85, 2), (200, 3), (800, 0), (2000, 3), (4000, 1), (7000, -4), (11000, -10) ]),
    # 6 Modern 4x12: extended lows and highs, tight
    ("cab6", [ (60, 5), (150, 2), (500, -1), (1500, -2), (3000, 1), (6000, 2), (10000, -2), (14000, -6) ]),
    # 7 Green 4x12: honk mid, soft top
    ("cab7", [ (90, 3), (250, 4), (900, 5), (2200, 2), (4500, -1), (7500, -8), (11000, -14) ]),
    # 8 Doom 4x15: massive low, dark top
    ("cab8", [ (50, 7), (120, 5), (350, 2), (900, -1), (2200, -3), (4500, -8), (8000, -16) ]),
]

# Off-axis variant: high-frequency rolloff + upper-mid bump relative to close.
def offaxis(table):
    out = []
    for f, db in table:
        if f >= 2500:
            db -= 6 * math.log(f / 2500.0, 2)      # extra HF rolloff
        elif 700 <= f <= 2500:
            db += 1.5
        out.append((f, db))
    return out


def main():
    os.makedirs(OUT, exist_ok=True)
    for name, table in CABS:
        close = table_to_minphase_ir(table)
        off = table_to_minphase_ir(offaxis(table))
        write_wav(os.path.join(OUT, f"{name}_close.wav"), close)
        write_wav(os.path.join(OUT, f"{name}_off.wav"), off)
        print(f"wrote {name}_close.wav / {name}_off.wav")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
