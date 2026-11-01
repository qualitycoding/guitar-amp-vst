#!/usr/bin/env python3
"""netimports.py - list the functions a PE binary imports from network DLLs (T-040 evidence).

Reads `dumpbin /imports` output and prints `DLL!function` for WININET, WS2_32, WSOCK32, WINHTTP,
and any libcurl DLL. Exits 0 always; the verdict belongs to the T-040 gate, this only produces
evidence. Usage: netimports.py imports.txt [out.txt]
"""
import re
import sys

NET = re.compile(r"^(wininet|ws2_32|wsock32|winhttp|libcurl.*)\.dll$", re.I)


def parse(text):
    out, current = [], None
    for line in text.splitlines():
        m = re.match(r"^\s{4}(\S+\.dll)\s*$", line, re.I)
        if m:
            current = m.group(1) if NET.match(m.group(1)) else None
            continue
        if current:
            f = re.match(r"^\s+(?:[0-9A-Fa-f]+\s+)?(?:[0-9A-Fa-f]+\s+)?([A-Za-z_][A-Za-z0-9_@?$]*)\s*$", line)
            if f and not f.group(1).lower().startswith(("import", "ordinal", "time", "index", "address", "name")):
                out.append(f"{current}!{f.group(1)}")
    return sorted(set(out))


if __name__ == "__main__":
    funcs = parse(open(sys.argv[1], errors="replace").read())
    text = "\n".join(funcs) + ("\n" if funcs else "")
    if len(sys.argv) > 2:
        open(sys.argv[2], "w").write(text)
    print(text or "no network-DLL imports")
