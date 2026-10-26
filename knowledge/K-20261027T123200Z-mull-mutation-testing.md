---
id: K-20261027T123200Z-mull-mutation-testing
statement: "Mull (LLVM-IR mutation testing for C/C++, latest release 0.34.0, May 2026) ships as version-matched packages: mull-19 bundles LLVM 19.1.7 and must be paired with clang-19. Test binaries are built with -O0 -g -grecord-command-line and the IR frontend via -fpass-plugin=/usr/lib/mull-ir-frontend-19; mull-runner-19 then reports killed/survived mutants. Tests and third-party paths must be excluded from mutation; Release builds must not be mutated (optimizer changes IR shape)."
status: current
supersedes: []
tags: [subject:mutation-testing-cpp, tech:cmake, tech:cpp, phase:verification]
applies_to_version: Mull 0.34.0, mull-19/clang-19 pairing (2026)
source: { url: "https://mull.readthedocs.io/en/latest/tutorials/HelloWorld.html", doi: "", title: "Mull documentation and README", locator: "Hello World tutorial; install guide; CLI reference", accessed: "2026-10-27", tier: 1 }
confidence: verified
derived_from: [C-010]
discovered_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:32:00Z
---
Version matching across the mull package suffix, the plugin path suffix, and the clang version is the common failure mode (plugin load / ABI incompatibility). Git-aware scoping (--include-path with changed files) keeps PR runs fast; full-suite runs suit nightly/release jobs. Mutation-score gates should start at the measured baseline, not an aspirational number.
