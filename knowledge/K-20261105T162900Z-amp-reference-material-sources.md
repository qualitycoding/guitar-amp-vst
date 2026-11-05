---
id: K-20261105T162900Z-amp-reference-material-sources
statement: "As of 2026-11-05, no CC BY or CC0 real recording with a paired DI exists online for a Boss HM-2, a black-metal rig, a 5E3 Tweed Deluxe, a Mesa Dual Rectifier or a high-gain 5150/6505. Real recordings of Mark V, Engl, Mesa 550 and 6505 amps exist but carry no licence or a non-commercial one. TONE3000 has T3K-licensed NAM captures of all of these. The T3K licence lets rendered output be published but forbids redistributing the capture. For a non-commercial project, CC BY-NC and CC BY-NC-ND material may be used locally; unlicensed material may not be used at all."
status: current
supersedes: []
tags: [subject:amp-reference-datasets, subject:guitar-amp-simulation, domain:audio, tech:nam, tech:zenodo, phase:research]
applies_to_version: "site contents and licences as read 2026-11-05"
source: {"url": "https://github.com/qualitycoding/guitar-amp-vst/blob/g010/research/G-010-reference-gap-research.md", "doi": "", "title": "G-010 research round: reference material for the uncovered voicings", "locator": "Sites listed in the body; each was read directly (Zenodo REST API, GitHub REST API, tone pages, PDFs)", "accessed": "2026-11-05", "tier": 1}
confidence: verified
derived_from: []
discovered_by: researcher
run: run-20261027T120000Z-guitar-amp-vst
recorded_at: 2026-11-05T16:29:00Z
---
Every site searched for real guitar-amp and pedal reference material, with what each one holds. "Usable" means the
material can be committed to an AGPL/Apache repo (CC BY, CC0, or output-publishable). Licences can change, so re-read
them before vendoring.

## Commit or local use, per source

"Local use" assumes a **non-commercial** project (true for guitar-amp-vst, owner decision 2026-11-05). A commercial
project loses every row marked NC. Origin references (DOI/URL, member path, window, checksum) and your own
measurements may always be committed.

| Source | Licence | Commit audio/renders | Local use (non-commercial project) |
|---|---|---|---|
| Zenodo: JVM 410H, EG-IPT, EGFxSet, Guitar-TECHS, EGSet12, GUITAR-FX-DIST, ToneTwist 2026-05-15 software entries | CC BY 4.0 | yes, with attribution | yes |
| Zenodo: ToneTwist hardware entries, pOD-set | CC BY-NC 4.0 | no (keeps a field-of-use restriction out of an open tree) | yes |
| Zenodo 19112212, ratings corpus | restricted | no | no, until access is granted |
| TONE3000 `t3k` captures | T3K | renders yes; `.nam` file no | yes |
| TONE3000 `cc-by-nc` captures (e.g. 1104) | CC BY-NC | no | yes |
| TONE3000 `cc-by-nc-nd` captures (e.g. 1614) | CC BY-NC-ND | no; renders are adapted material, which ND forbids sharing | yes |
| GitHub `Alec-Wright/Automated-GuitarAmpModelling`, `GuitarML/GuitarLSTM`, `GuitarML/PedalNetRT` data | GPL-3.0 | yes, under GPL terms and if the tree's licence is compatible | yes |
| GitHub `IoSR-Surrey/DNNAmplifierDemos` | none | no | no. GitHub's terms allow only viewing, playing and forking on github.com, and the clips are commercial songs. |
| GitHub `TSchmitzULG/NeuralNetwork_Comparison` | none | no | no. The ToneTwist CC BY-NC copies of Mark V, Mesa 550, Engl Drive, HT-5 Metal and TSA15 are the licensed route. |
| GitHub `stepanmk/grey-box-amp` | none | no | no (the same JVM data is CC BY upstream) |
| eprints.hud.ac.uk (Herbst 2017) | repository terms: non-commercial, unchanged | no; cite facts | yes (read) |
| catalinbread.com, articles.boss.info, mirosol.kapsi.fi | all rights reserved | no; cite facts | yes (read) |
| en.wikipedia.org | CC BY-SA 4.0 (text) | text only under CC BY-SA; cite facts | yes |

## Datasets and audio

**Zenodo** (`zenodo.org/api/records?q=…&size=25`; anonymous page size is capped at 25, larger sizes return HTTP 400)
- *Marshall JVM 410H Dataset*, 7970723: **CC BY 4.0, usable.** OD1 channel, DI plus speaker output into a load (no
  cab). One knob varied at a time from B5 M5 T5 G5, including `B5_M0_T5_G5` and `B5_M5_T5_G10`.
- *EG-IPT*, 15205644: **CC BY 4.0, usable.** EVH 5150 III, clean only, Mesa 4×12 V30, SM57/R14 plus DI.
- *ToneTwist AFx*, about 60 records titled "ToneTwist AFx Dataset…": every **real-hardware** entry is **CC BY-NC 4.0**.
  Amps: Fender Blues Jr, Mesa Mark V (Clean, Crunch, Extreme), Mesa 550 (Clean, Crunch, Burn), Engl Retro Tube 50
  Drive, Blackstar HT1 and HT5 Metal, Ibanez TSA15, Marshall JVM410H OD1. Plus many pedals. The "External" entries
  re-host other groups' data, and the description's "Original Data" link names the upstream. Entries dated 2026-05-15
  are CC BY but software (TH3 Overloud presets, Audacity effects).
- *GUITAR-FX-DIST*, 4296040 / 4298000 / 4298017 / 4298025: CC BY 4.0, but processed with **plugins** (Audified
  Multidrive Pedal Pro, Mercuriall Greed Smasher, Analog Obsession). Not hardware.
- *EGFxSet*, 7044411: **CC BY 4.0, usable.** Real BD-2, Mini Tube Screamer, RAT2 and modulation/delay pedals with clean
  DI. No amp.
- *pOD-set*, 15389653: CC BY-NC 4.0. 27 overdrive pedals × 36 settings.
- *Guitar-TECHS*, 14963133: CC BY 4.0. DI plus miked Orange CR60, Orange Crush 12 and a Yamaha bass amp at flat EQ. Low
  gain.
- *EGSet12*, 11406378: CC BY 4.0. Yamaha B15 miked, no DI.
- *Predicting Subjective Ratings of Guitar Amplifier Emulations – Audio Corpus*, 19112212 (Wright, Dal Santo,
  Schlecht, 2026): **restricted access**, no licence, no files listed.
- Searching "HM-2", "Rectifier", "5150" or "Fender Deluxe" returns no relevant datasets.

**Hugging Face** (`huggingface.co/api/datasets?search=…`): nothing relevant for guitar, amp, amplifier or distortion.

**GitHub**
- `IoSR-Surrey/DNNAmplifierDemos`: **no licence.** `Real Amplifier/RA–RH.wav`: Peavey 6505 LEAD through a Rivera
  RockCrusher load, no cab, 8 excerpts of 5–8 s at 44.1/16. **No DI.** The excerpts are performances of commercial
  songs. Also holds DNN-model and anchor renders.
- `TSchmitzULG/NeuralNetwork_Comparison/Datasets`: **no licence.** `.mat` train/val/test sets for Mesa Mark V (Clean,
  Crunch, Disto), Mesa 550 (Clean, Crunch, Disto), Engl Retro Tube 50 (Disto, plus `outENGL1..10.wav` gain sweep),
  Blackstar HT-5 Metal and Ibanez TSA15. This is the upstream of the ToneTwist Mark V and Engl sets.
- `Alec-Wright/Automated-GuitarAmpModelling/Data`: **GPL-3.0** (repo licence). Blackstar HT-1 and Big Muff
  input/target WAVs.
- `GuitarML/GuitarLSTM` and `GuitarML/PedalNetRT` (GPL-3.0): TS9 in/out WAVs only. `GuitarML/SmartGuitarAmp`
  (Apache-2.0) and `GuitarML/Proteus` (GPL-3.0) hold no amp data. GuitarML's Blues Jr data is not on GitHub; ToneTwist
  record 10797977 says "Original Data N.A."
- `stepanmk/grey-box-amp`: no licence; demo WAVs of the JVM data.
- `sdatkinson/NeuralAmpModelerCore` and `sdatkinson/neural-amp-modeler`: **MIT**, the NAM runtime and trainer.
- `tone-3000/api`: API docs and examples only. Search and download need OAuth.

**TONE3000** (`tone3000.com/tones/<slug>-<id>`)
- Licence is per tone: the `license` field in the tone record embedded in the page (values seen: `t3k`, `cc-by-nc`,
  `cc-by-nc-nd`, `cc-by-sa`). T3K text: "Users may download and use the data file in software and publish the
  resulting outputs without royalties or restrictions. However, they may not upload, republish, or distribute the data
  file without the author's permission." **Rendered output is usable; the .nam file is not.**
- The search page renders client-side, so curl returns no results from it. Tone pages are server-rendered and
  parseable.
- T3K captures of the gap gear:
  - HM-2: 571, 722
  - Dual Rectifier: 1768 (Red, direct, amp only), 69206 (full rig), 52, 62644, 943 (Green)
  - Peavey 6505/6505+: 72177 (Lead, amp head into a load), 6013, 42842, 5275
  - 5E3 Tweed Deluxe: 1091 (StewMac clone), 4976, 75305, 1691 ('57 Custom), 90842 (1957, miked), 27920
  - Peavey Bandit Lead: 67991
  - Marshall Valvestate 8100: 29995
  - Boss DS-1: 215
- Not usable: HM-2 MiJ v2.0 (1104) is CC BY-NC; Tweed Deluxe 1958 DI (1614) is CC BY-NC-ND.
- `/policy` lists what may be shared (analog gear, rooms, openly licensed IRs). It does not list licence options.

**Uliège** (`orbi.uliege.be/handle/2268/228910`): record of Schmitz & Embrechts, AES 145 (2018), with no data licence
stated. Its download link, `montefiore.uliege.be/services/acous/STSI/downloads.php`, returns **404**.

**Freesound**: web search found no HM-2 or black-metal DI-plus-amp pairs. The Freesound API was not queried; it needs
a key.

## Papers and circuit facts (cite only)

- **dafx.de**, DAFx23 paper 40 (Cassidy & De Sena), CC BY 4.0. The 6505 LEAD re-amp chain: Radial X-Amp 10 kΩ,
  RockCrusher load, Apollo Twin X. The DIs are not published. Extract text from the PDF with pypdf; WebFetch returns
  binary.
- **arXiv** 2501.03720 (Guitar-TECHS, amp list) and 2509.22655 (GOAT: DIs re-amped through NAM profiles; dataset
  hosting and licence not stated in the abstract).
- **catalinbread.com** "Boss HM-2" (2020-08-28): Low gyrator 86.79 Hz; High is two gyrators at 958.47 Hz and
  1278.6 Hz; about +10 dB each.
- **articles.boss.info**, HM-2W analysis: Low "around 80 to 90 Hz"; "double-humped" EQ.
- **mirosol.kapsi.fi** (2014): HM-2 gain path. NPN/PNP pregain, asymmetric clipping amp, two clipper pairs, three
  gyrator filters.
- **en.wikipedia.org** "Peavey 5150": lead channel has 4 preamp stages plus phase inverter (5 × 12AX7), 4 × 6L6,
  120 W. The 6505 is the 5150 renamed.
- **eprints.hud.ac.uk** 33248, Herbst 2017, *Metal Music Studies* 3(1). Genre spectral trends from produced guitar
  tracks (not amp outputs). Repository terms are non-commercial; cite only.
- Dead ends: `mennovanderveen.nl` "13_Tweed_Deluxe.pdf" is image-only (no extractable text), and
  `robrobinette.com/How_the_5E3_Works.htm` returns 404. No citable numeric 5E3 circuit source was obtained.
