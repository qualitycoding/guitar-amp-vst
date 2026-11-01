#!/usr/bin/env python3
"""mull_score.py - mutation score and per-file table from a Mull SQLite report, with a floor.

Mull's own threshold option did not fail a run that also passed the option that tolerates
surviving mutants (observed with mull-runner-19, 2026-11), so the score is computed here from the
report and the exit status is set by this script.

  mull_score.py mull.sqlite --floor 75 [--min-mutants 100] [--survived-status 2] [--strip PREFIX]

Exit status: 0 = score >= floor, 1 = below floor or a post-condition failed (missing or tiny
report), 2 = usage error. Status code 2 means "survived" in the `mutant.execution_status` column
of the Mull SQLite report observed in 2026-11; check with a report that contains a known survivor
before relying on it (the log line "Surviving mutants: N" must equal the count printed here).
"""
import argparse
import collections
import pathlib
import sqlite3
import sys


def score(db_path, survived_status, strip):
    p = pathlib.Path(db_path)
    if not p.is_file() or p.stat().st_size == 0:
        raise SystemExit(f"FAIL: report missing or empty: {db_path}")
    try:
        rows = sqlite3.connect(str(p)).execute(
            "select filename, execution_status from mutant").fetchall()
    except sqlite3.Error as e:
        raise SystemExit(f"FAIL: report unreadable ({e})")
    per = collections.defaultdict(lambda: [0, 0])      # file -> [killed, total]
    for filename, status in rows:
        name = filename.rsplit(strip, 1)[-1] if strip and strip in filename else filename
        per[name][1] += 1
        per[name][0] += 0 if status == survived_status else 1
    killed = sum(k for k, _ in per.values())
    total = sum(t for _, t in per.values())
    return killed, total, dict(per)


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("db")
    ap.add_argument("--floor", type=float, required=True, help="minimum mutation score in percent")
    ap.add_argument("--min-mutants", type=int, default=1, help="post-condition: fewer mutants than this fails")
    ap.add_argument("--survived-status", type=int, default=2)
    ap.add_argument("--strip", default="", help="drop everything up to and including this path fragment")
    a = ap.parse_args(argv)

    killed, total, per = score(a.db, a.survived_status, a.strip)
    if total < a.min_mutants:
        print(f"FAIL: only {total} mutants in the report (minimum {a.min_mutants}); the run did not do its job")
        return 1
    width = max((len(f) for f in per), default=10)
    for f, (k, t) in sorted(per.items(), key=lambda kv: kv[1][0] / kv[1][1]):
        print(f"{f:<{width}}  {k:>4}/{t:<4} {100.0 * k / t:5.1f}%")
    pct = 100.0 * killed / total
    print(f"Mutation score: {pct:.1f}% ({killed}/{total} killed); floor {a.floor:g}%")
    return 0 if pct >= a.floor else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
