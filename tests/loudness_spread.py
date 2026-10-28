#!/usr/bin/env python3
"""S-009: measure LUFS of 3s rendered riffs per voicing; check spread <= ±1.5 dB."""
import json, subprocess, sys, wave, math, os, contextlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RENDERS = os.path.join(ROOT, "validation", "renders")
LIMIT_DB = 1.5

def measure_lufs(path):
    # ffmpeg ebur128 momentary -> use integrated loudness
    p = subprocess.run(["ffmpeg", "-hide_banner", "-nostats", "-i", path,
                        "-filter_complex", "ebur128=peak=true", "-f", "null", "-"],
                       capture_output=True, text=True)
    integrated = None
    for line in p.stderr.splitlines():
        line = line.strip()
        if line.startswith("I:"):
            parts = line.split()
            try:
                integrated = float(parts[1].replace("LUFS", ""))
            except Exception:
                pass
    return integrated

def main():
    files = sorted(f for f in os.listdir(RENDERS) if f.endswith(".wav") and not f.endswith("_os.wav"))
    if not files:
        print("no renders found in", RENDERS); return 1
    results = {}
    for f in files:
        v = f.replace("riff_", "").replace(".wav", "")
        l = measure_lufs(os.path.join(RENDERS, f))
        if l is not None:
            results[v] = l
    if len(results) < 2:
        print("could not measure", files); return 1
    mean = sum(results.values()) / len(results)
    worst = 0.0
    print(f"{'voicing':<10} {'LUFS':>8} {'delta':>7}")
    for v, l in sorted(results.items()):
        d = l - mean
        worst = max(worst, abs(d))
        print(f"{v:<10} {l:>8.2f} {d:>+7.2f}")
    print(f"mean={mean:.2f} LUFS  worst spread={worst:.2f} dB  limit=±{LIMIT_DB}")
    ok = worst <= LIMIT_DB
    print("PASS" if ok else "FAIL")
    return 0 if ok else 1

if __name__ == "__main__":
    sys.exit(main())
