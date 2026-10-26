---
id: K-20261027T123100Z-juce8-licensing
statement: "JUCE 8 is dual-licensed under the JUCE licence and the AGPLv3 (JUCE 7's GPLv3 option was replaced by AGPLv3). The JUCE licence has tiers: Starter (free, perpetual, annual revenue/funding limit $20,000, no splash screen), Indie ($40/month or $800 perpetual per user, limit $300,000), Pro ($175/month or $3,500 perpetual, no limit), and Educational (free, non-commercial). Presets and static assets that use only end-user features do not require licence seats."
status: current
supersedes: []
tags: [subject:juce-licensing, tech:juce, phase:intake]
applies_to_version: JUCE 8.x (EULA read 2026-10-27)
source: { url: "https://juce.com/legal/juce-8-licence/", doi: "", title: "The JUCE 8 End User Licence Agreement", locator: "section 1.2 Licence Types table; tier definitions", accessed: "2026-10-27", tier: 1 }
confidence: verified
derived_from: [C-009]
discovered_by: planner
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-10-27T12:31:00Z
---
Corroborated by the JUCE team's EULA announcement thread (t0m, 2024-04): the move from GPLv3 to AGPLv3 affects only those providing remote services under the copyleft licence without publishing source. For a desktop plugin: AGPL-3.0 for open source, or Starter for free closed-source below the revenue limit. Licence seats are required for everyone who modifies source (including contractors), not for preset/asset contributors.
