#!/usr/bin/env python3
"""check_bench.py — gates for T-050 (CPU budget, regression) and T-051 (latency).

Reads bench-results.json, tests/bench/bench-targets.json and tests/bench/baseline-T050.json.
Post-conditions (Rule 14): the results file exists, is non-empty, parses, and holds every
voicing x mode plus both latency entries; otherwise the check fails.

  check_bench.py --runner windows-latest --results build/tests/bench-results.json \
                 [--candidate-out build/tests/bench-baseline-candidate.json]
  check_bench.py --selftest
"""
import json
import os
import pathlib
import sys

VOICINGS = ["Tweed", "Plexi", "JCM800", "Recto", "5150", "Chainsaw", "Frost"]
MODES = ["Eco", "High"]


def load_json(path, what):
    p = pathlib.Path(path)
    if not p.is_file() or p.stat().st_size == 0:
        raise SystemExit(f"FAIL: {what} missing or empty: {path}")
    try:
        return json.loads(p.read_text())
    except ValueError as e:
        raise SystemExit(f"FAIL: {what} does not parse: {e}")


def evaluate(results, targets, baseline, runner):
    """Return (failures, lines, candidate)."""
    fails, lines = [], []
    cpu = {}
    for v in VOICINGS:
        for m in MODES:
            key = f"{v}/{m}"
            entry = results.get("voicings", {}).get(key)
            if not entry or "cpuPercent" not in entry:
                fails.append(f"results lack {key}")
            else:
                cpu[key] = float(entry["cpuPercent"])
    for m in MODES:
        if m not in results.get("latency", {}):
            fails.append(f"results lack latency entry {m}")
    if fails:
        return fails, lines, None

    # T-051: latency verdicts must pass
    for m in MODES:
        lat = results["latency"][m]
        ok = bool(lat.get("passes"))
        lines.append(f"T-051 latency {m}: reported={lat.get('reported')} measured={lat.get('measured')} {'PASS' if ok else 'FAIL'}")
        if not ok:
            fails.append(f"T-051 latency {m} out of bounds")

    cfg = targets["runners"].get(runner)
    if cfg is None:
        fails.append(f"no target defined for runner class {runner}")
        return fails, lines, None

    # T-050 budget on the worst case
    wc = targets["worst_case"]
    budget = cfg["budget_pct"]
    wc_val = cpu[wc]
    over = wc_val > budget
    if cfg["enforce_budget"]:
        lines.append(f"T-050 budget {wc}: {wc_val:.2f}% vs {budget}% -> {'FAIL' if over else 'PASS'}")
        if over:
            fails.append(f"T-050 budget missed on {runner}: {wc} {wc_val:.2f}% > {budget}%")
    else:
        lines.append(f"T-050 budget {wc}: {wc_val:.2f}% vs {budget}% -> {'RELAXED, would fail' if over else 'pass'} "
                     f"(not enforced: {cfg.get('relaxed_by', 'relaxed')})")
    for key, val in sorted(cpu.items()):
        if key != wc and val > budget:
            lines.append(f"  info: {key} {val:.2f}% exceeds the {budget}% budget (budget applies to {wc})")

    # T-050 regression vs stored baseline (bootstrap when no baseline exists)
    base = (baseline or {}).get("runners", {}).get(runner)
    candidate = {"source": "this run", "runner": runner, "cpu_percent": {k: round(v, 2) for k, v in cpu.items()},
                 "machine": results.get("machine")}
    if base is None:
        lines.append("BASELINE RECORDED: no baseline for this runner class; candidate written, check passes")
    else:
        tol, floor = targets["regression_tolerance"], targets["regression_floor_pp"]
        worst = []
        for key, val in sorted(cpu.items()):
            b = base["cpu_percent"].get(key)
            if b is None:
                continue
            if val > b * (1 + tol) and val - b > floor:
                worst.append(f"{key}: {val:.2f}% vs baseline {b:.2f}% (+{100 * (val / b - 1):.0f}%)")
        if worst:
            msg = "T-050 regression >" + f"{int(tol * 100)}%: " + "; ".join(worst)
            if cfg["enforce_regression"]:
                lines.append(msg + " -> FAIL")
                fails.append(msg)
            else:
                lines.append(msg + f" -> reported only ({cfg.get('relaxed_by', 'relaxed')})")
        else:
            lines.append("T-050 regression: within tolerance of baseline")
    return fails, lines, candidate


def selftest():
    def mk(frost_high=10.0, lat_ok=True, drop=None):
        r = {"machine": "t", "voicings": {f"{v}/{m}": {"cpuPercent": 2.0 if m == "Eco" else 10.0} for v in VOICINGS for m in MODES},
             "latency": {"Eco": {"reported": 0, "measured": 0, "passes": True}, "High": {"reported": 4, "measured": 5, "passes": lat_ok}}}
        r["voicings"]["Frost/High"]["cpuPercent"] = frost_high
        if drop:
            del r["voicings"][drop]
        return r
    tg = {"worst_case": "Frost/High", "regression_tolerance": 0.2, "regression_floor_pp": 2.0,
          "runners": {"w": {"budget_pct": 15.0, "enforce_budget": True, "enforce_regression": True},
                      "m": {"budget_pct": 15.0, "enforce_budget": False, "enforce_regression": False, "relaxed_by": "x"}}}
    bl = {"runners": {"w": {"cpu_percent": {f"{v}/{m}": (2.0 if m == "Eco" else 10.0) for v in VOICINGS for m in MODES}},
                      "m": {"cpu_percent": {f"{v}/{m}": (2.0 if m == "Eco" else 10.0) for v in VOICINGS for m in MODES}}}}
    cases = [
        ("pass", evaluate(mk(), tg, bl, "w")[0], False),
        ("budget missed on enforced runner", evaluate(mk(16.0), tg, bl, "w")[0], True),
        ("budget missed on relaxed runner", evaluate(mk(16.0), tg, bl, "m")[0], False),
        ("latency fails", evaluate(mk(lat_ok=False), tg, bl, "m")[0], True),
        ("regression on enforced runner", evaluate(mk(13.0), tg, bl, "w")[0], True),
        ("regression within noise floor", evaluate(mk(11.9), tg, bl, "w")[0], False),
        ("missing voicing", evaluate(mk(drop="Recto/High"), tg, bl, "w")[0], True),
        ("bootstrap without baseline", evaluate(mk(), tg, {"runners": {}}, "w")[0], False),
        ("unknown runner class", evaluate(mk(), tg, bl, "zzz")[0], True),
    ]
    bad = [n for n, f, want in cases if bool(f) != want]
    for n, f, want in cases:
        print(f"  {'ok ' if bool(f) == want else 'BAD'} {n}")
    print("selftest:", "ok" if not bad else f"FAIL {bad}")
    return 1 if bad else 0


def main(argv):
    if "--selftest" in argv:
        return selftest()
    arg = lambda k, d=None: argv[argv.index(k) + 1] if k in argv else d
    runner = arg("--runner")
    if not runner:
        raise SystemExit("usage: check_bench.py --runner <label> --results <file> [--candidate-out <file>]")
    results = load_json(arg("--results", "build/tests/bench-results.json"), "bench results")
    targets = load_json(arg("--targets", "tests/bench/bench-targets.json"), "bench targets")
    bpath = arg("--baseline", "tests/bench/baseline-T050.json")
    baseline = load_json(bpath, "bench baseline") if pathlib.Path(bpath).is_file() else None
    fails, lines, candidate = evaluate(results, targets, baseline, runner)
    out = "\n".join(lines)
    print(out)
    cand_path = arg("--candidate-out")
    if cand_path and candidate is not None:
        pathlib.Path(cand_path).write_text(json.dumps(candidate, indent=2))
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a") as f:
            f.write(f"### Bench gates ({runner})\n\n```\n{out}\n```\n")
            if fails:
                f.write("\n**Failures:** " + "; ".join(fails) + "\n")
    if fails:
        print("FAIL: " + "; ".join(fails))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
