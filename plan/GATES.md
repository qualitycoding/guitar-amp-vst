# GATES — Human Gate Definitions (Rule 10)

## G-002 — Public release sign-off `[All, required: external/irreversible action]`

- **Trigger step:** S-017 (release preparation complete, tag drafted but not published).
- **Evidence bundle the implementer assembles:** `GATE-G-002.md` containing: CI green links for the release commit (builds + tests + pluginval on Windows and macOS), the S-015 Ableton Live test record, the G-010 sign-off file, installer smoke-test results (install → Live rescan → loads → presets present), `CHANGELOG.md` entry, and the exact draft release URL.
- **Questions for the human:** (1) Publish the GitHub Release with these artifacts? (2) Announce anywhere (forums/Reddit) — yes/no, and where?
- **Allowed responses:** `proceed` (publish the drafted release), `proceed-with-rescope: <text>` (e.g. publish without AU, or as beta `v0.9.0`), `stop`.
- **Plan branches:** `proceed` → publish release, continue to S-RETRO. `proceed-with-rescope` → adjust tag/artifacts per text, re-run affected S-016/S-017 items, then publish. `stop` → halt; leave release as draft.

## G-003 — Lessons & knowledge push `[All, required, always last]`

- **Trigger step:** S-KNOW (after S-RETRO).
- **Evidence bundle:** `GATE-G-003.md` listing every `lessons/L-*.md` and `knowledge/K-*.md` accumulated on the branches during planning and implementation (IDs, titles, tags), plus the Appendix C schema-check output (`all entries valid`).
- **Questions for the human:** Push these to the knowledge store recorded at intake (A-008, amended: `qualitycoding/knowledge`)?
- **Allowed responses:** `push-to-proposed`, `push-to: <target>`, `do-not-push`.
- **Plan branches:** per protocol 3.7.2 — on sign-off, copy into the Appendix C layout, regenerate indexes, push (retry 3× on rejection; on persistent failure or no response, write `git bundle create knowledge-<run_id>.bundle` and report its path). `do-not-push` → done, nothing pushed.

## G-010 — Tone sign-off (added per Phase 4.4, mitigates R-001/R-002)

- **Trigger step:** S-015 (Ableton Live manual test protocol executed).
- **Evidence bundle:** `GATE-G-010.md` containing: the T-014 listening checklist results per voicing/preset (pass/fail + notes), rendered audio clips (one 10-second DI→amp render per genre preset, committed under `validation/renders/`), CPU meter screenshots at "High" oversampling, and any known tonal caveats.
- **Questions for the human:** Does the tone meet the goal "70s blues rock through high-gain black metal and beyond"? Any voicing that must be reworked before release?
- **Allowed responses:** `proceed`, `proceed-with-rescope: <text>` (e.g. "release without the Chainsaw voicing", "rework Frost and re-test"), `stop`.
- **Plan branches:** `proceed` → S-016. `proceed-with-rescope` → implement the rescope, re-run S-015 for affected voicings, re-present G-010 (max 2 additional rounds, then escalate per Phase 4.6). `stop` → halt.

## Notes

- `G-001` is **not required** (math/computational profiles inactive — see `plan/PROFILE.md`).
- While halted at a gate, the implementer may continue any step that does not depend on the gate's outcome, but never an external or irreversible action (G-002).
- Gate files are named `GATE-<id>.md` at repo root on the implementation branch.
