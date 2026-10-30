#!/usr/bin/env python3
"""lint_ci.py — Rule 14 lint: fail on failure-swallowing idioms in CI workflows.

Checks every .github/workflows/*.yml for `|| true`, `continue-on-error`, `set +e`, and for
`| tee` pipelines in a run block that neither sets `pipefail` nor uses `shell: bash`.
Allow-listed lines live in tools/ci/swallow-allowlist.txt (one `file:substring` per line).

Usage: lint_ci.py [--root DIR] [--selftest]
"""
import pathlib
import re
import sys

PATTERNS = [
    (re.compile(r"\|\|\s*true\b"), "'|| true' swallows a failure"),
    (re.compile(r"continue-on-error\s*:\s*true"), "'continue-on-error: true' swallows a failure"),
    (re.compile(r"\bset\s+\+e\b"), "'set +e' disables fail-fast"),
]


def load_allowlist(root):
    p = root / "tools" / "ci" / "swallow-allowlist.txt"
    if not p.exists():
        return []
    return [l.strip() for l in p.read_text().splitlines() if l.strip() and not l.startswith("#")]


def lint_text(name, text, allow):
    problems = []
    lines = text.splitlines()
    for n, line in enumerate(lines, 1):
        code = line.split("#", 1)[0] if not line.lstrip().startswith("run:") else line
        for rx, msg in PATTERNS:
            if rx.search(code) and not any(a == f"{name}:{frag}" for a in allow for frag in [a.split(":", 1)[1]] if frag in line):
                problems.append(f"{name}:{n}: {msg}: {line.strip()}")
    # pipelines into tee need pipefail (default `bash -e {0}` has none)
    i = 0
    while i < len(lines):
        m = re.match(r"(\s*)(-\s+)?run:\s*(\|?)\s*(.*)$", lines[i])
        if m:
            indent = len(m.group(1)) + (len(m.group(2)) if m.group(2) else 0)
            block, j = [lines[i]], i + 1
            while j < len(lines) and (not lines[j].strip() or len(lines[j]) - len(lines[j].lstrip()) > indent):
                block.append(lines[j]); j += 1
            # step-level `shell: bash` appears just above/below as a sibling key
            ctx = "\n".join(lines[max(0, i - 6): j + 1])
            text_block = "\n".join(block)
            if re.search(r"\|\s*tee\b", text_block) and "pipefail" not in text_block \
                    and not re.search(r"shell:\s*bash", ctx):
                problems.append(f"{name}:{i + 1}: pipeline into tee without pipefail / shell: bash")
            i = j
        else:
            i += 1
    return problems


def lint_root(root):
    allow = load_allowlist(root)
    problems = []
    for f in sorted((root / ".github" / "workflows").glob("*.yml")):
        problems += lint_text(f.name, f.read_text(), allow)
    return problems


def selftest():
    bad = "steps:\n  - run: make || true\n  - run: |\n      ./t | tee log\n  - continue-on-error: true\n  - run: set +e\n"
    good = "steps:\n  - run: |\n      set -o pipefail\n      ./t | tee log\n  - shell: bash\n    run: |\n      ./t | tee log\n"
    nb, ng = len(lint_text("x.yml", bad, [])), len(lint_text("x.yml", good, []))
    ok = nb == 4 and ng == 0
    print(f"selftest: bad={nb} (want 4) good={ng} (want 0) -> {'ok' if ok else 'FAIL'}")
    return 0 if ok else 1


def main(argv):
    if "--selftest" in argv:
        return selftest()
    root = pathlib.Path(argv[argv.index("--root") + 1]) if "--root" in argv else pathlib.Path(".")
    problems = lint_root(root)
    print("\n".join(problems) or "lint_ci: no failure-swallowing idioms")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
