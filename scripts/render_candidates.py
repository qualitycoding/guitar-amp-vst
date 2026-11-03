#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Render GrimAmp candidates for the G-010 blind pairs.

For every di/<stem>.wav in --work, run grimamp_bench --render-file with the factory preset and the render_overrides
that references/manifest.json records for that stem, and write cand/<stem>.wav next to ref/ and di/.

  python render_candidates.py --bench build/tests/grimamp_bench --work listening-work/G-010/round-001
"""
import argparse
import json
import pathlib
import subprocess
import sys


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--bench", required=True)
    ap.add_argument("--work", required=True)
    ap.add_argument("--manifest", default=str(pathlib.Path(__file__).parent.parent / "references" / "manifest.json"))
    a = ap.parse_args(argv)
    work = pathlib.Path(a.work)
    (work / "cand").mkdir(exist_ok=True)
    entries = {p["stem"]: p for p in json.load(open(a.manifest))["presets"]}
    for di in sorted((work / "di").glob("*.wav")):
        e = entries[di.stem]
        index = int(e["stem"][1:3]) - 1  # pNN is the 1-based factory preset number
        sets = [f"{k}={v}" for k, v in (e.get("render_overrides") or {}).items()]
        subprocess.run([a.bench, "--render-file", str(di), str(work / "cand" / di.name), str(index), *sets], check=True)
        print(f"{di.stem}: preset {index} {e['name']!r} {sets}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
