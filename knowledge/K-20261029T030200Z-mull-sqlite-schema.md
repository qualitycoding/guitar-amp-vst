---
id: K-20261029T030200Z-mull-sqlite-schema
statement: "The Mull SQLite report has a table mutant(mutant_id, mutator, filename, directory, line_number, column_number, end_line_number, end_column_number, execution_status, exit_status, duration, stdout, stderr, mutation_replacement); execution_status 2 means the mutant survived, 1 and 3 mean it was killed (failed or timed out)."
status: current
supersedes: []
tags: [subject:mull, tech:mull, phase:verification]
applies_to_version: "mull-19 (mull-stable, 2026-10)"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/actions/runs/40847181602", "doi": "", "title": "mull-report artifact", "locator": "mull.sqlite; Surviving mutants: 96 in mull.log equals count of status 2", "accessed": "2026-10-30", "tier": 1}
confidence: verified
derived_from: []
discovered_by: implementer
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-29T03:02:00Z
---
Score = (rows with execution_status != 2) / total rows. In run 40847181602 the log said `Surviving mutants: 96` of 154 and exactly 96 rows had status 2. The text report prints no percentage, so CI must compute the score itself from the database.
