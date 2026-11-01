---
id: L-20261028T210000Z-crlf-breaks-hash-manifests-on-windows
title: A byte-hash manifest failed on Windows because git checked the files out with CRLF
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:git, tech:github-actions, phase:verification, kind:environment]
recorded_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-28T21:00:00Z
---
## Trigger
`sha256sum -c MANIFEST` passes on Linux and fails on the `windows-latest` runner with "No such file or directory" for a name ending in `\r`, or a hash mismatch.

## What went wrong
`actions/checkout` on Windows applies `core.autocrlf`, so the manifest itself had CRLF line endings (file names read as `name\r`) and the hashed files could differ in bytes from the Linux-generated hashes.

## Correction
Add a `.gitattributes` entry that forces LF for the hashed files (`tests/** text eol=lf`).

## Prevention rule
Whenever a repository stores hashes of file bytes, ship a `.gitattributes` that pins those paths to LF, and run the verification on Windows in CI.

## Detection check
`sha256sum -c` is part of the Windows job and passes.

## Evidence
guitar-amp-vst run 40761557039 (failure), run 40763261991 (pass).
