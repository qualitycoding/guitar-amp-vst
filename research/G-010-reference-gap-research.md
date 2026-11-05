# G-010 research round: reference material for the uncovered voicings

Date: 2026-11-05. Trigger: `references/README.md` lists five voicings with no usable real-amp reference. Chainsaw
(presets 11, 12) and Frost (13, 14, 24) have none. Tweed and Recto have references, but only under CC BY-NC, so they
are local-only. 5150 high gain has only a clean 5150 III recording. The README's realism claim excludes these presets
for that reason.

Question: what reference material exists for these amps, and does its licence let us commit it? Material whose
licence blocks use was traced back to its upstream source, because a re-host can carry a stricter licence than the
original. (The JVM 410H data shows this: CC BY-NC on ToneTwist, CC BY upstream.)

Licences and page contents below were read on 2026-11-05. IDs are stable; licences can change, so re-check them before
vendoring anything.

## Summary per gap

| Gap | Best real recording found | Licence / status | Usable route found this round |
|---|---|---|---|
| Chainsaw (HM-2) | none | — | Render CC BY DIs through T3K-licensed NAM captures of a real HM-2 (§A2). Circuit facts in §D1. |
| Frost | none | — | Weak in-repo proxy from the JVM 410H M0 and G10 settings (§A1). Same-gear T3K captures exist (§A2), but there is no agreed "Frost amp". |
| Tweed | none closer than the Blues Junior | Blues Jr is CC BY-NC, and GuitarML never published it upstream (§B3) | T3K captures of real and clone 5E3 Tweed Deluxes (§A2). |
| Recto | Mesa Mark V Extreme (Schmitz & Embrechts) | Upstream repo has **no licence** (§B2) | T3K captures of a Dual Rectifier Rev F Red channel (§A2). Ask the authors to relicense (§E). |
| 5150 high gain | **Peavey 6505 LEAD, real amp, load box, no cab** (Cassidy & De Sena, DAFx23) | **No licence, no DI**, and the excerpts are covers of commercial songs (§B1) | T3K captures of the 6505 lead channel (§A2). Ask the authors for the DIs and a licence (§E). |

No new **real recording with a paired DI** under CC BY or CC0 was found for any of the five gaps. The new routes are
(1) a weak proxy using CC BY data we already have, (2) a capture-derived tier whose licence allows publishing rendered
audio, and (3) three upstream owners who could be asked to relicense.

## A. Usable now

### A1. JVM 410H settings we already have (CC BY 4.0, vendored source)

The JVM 410H dataset (doi:10.5281/zenodo.7970723) already contains `B5_M0_T5_G5` (mids fully cut) and
`B5_M5_T5_G10` (gain fully up). They are listed in `JVM_COMBOS` in `scripts/fetch_references.py`. Frost is described
as "boosted, scooped, tight" (`plan/ASSUMPTIONS.md`), and these are the nearest real-valve settings in-repo. Grade:
**weak**, on the same terms as the 5150 rows: a real high-gain valve amp, not a black-metal voicing. No new licence is
involved; `RULES["Frost"]` only needs a rule.

### A2. TONE3000 captures under the T3K licence (rendered audio is publishable)

TONE3000 sets a licence per capture. The T3K text shown on each tone page reads, verbatim:

> Users may download and use the data file in software and publish the resulting outputs without royalties or
> restrictions. However, they may not upload, republish, or distribute the data file without the author's permission.

Rendering CC BY DIs (JVM 410H, EG-IPT) through a T3K capture therefore gives audio we may commit. The `.nam` files
must stay out of the repo, so fetch them locally the way the CC BY-NC sources are fetched. The renderer
(NeuralAmpModelerCore) is MIT. The README rejected captures earlier because "their usual licences forbid
redistribution". That is true of the capture file but not of its output.

**Caveat, which needs to be stated wherever these are used:** a capture is a neural model of the device, not a
recording. Compare a capture-derived pair as "GrimAmp against a high-accuracy model of a real X", in its own tier,
and never pool it with real-recording pairs. Note each capture's model architecture (A1/A2, standard/lite/feather),
because accuracy differs.

| Gap | TONE3000 id | Title (as published) | Gear | Models | Licence | Notes from the description |
|---|---|---|---|---|---|---|
| 5150 | 72177 | Peavey 6505 1992 Original - Lead - Mesa 4x12 Load - Amp Head | amp | 48 | T3K | Lead channel, amp head into a cab load. Closest match to an amp-only 5150 reference. |
| 5150 | 6013 | Peavey 6505 1992 Original (11.5dBu) | amp | 202 | T3K | Interface level calibrated at 11.5 dBu |
| 5150 | 42842 | Peavy 6505 Lead High Gain | amp | 4 | T3K | "without cabinet", St. Rock ReactIR load |
| 5150 | 5275 | Peavey 6505+ Hi Gain Pack | amp | 10 | T3K | 6505+ (not the original 6505), boosted |
| Recto | 1768 | Mesa Boogie Dual Rectifier RevF Red Essentials | amp | 18 | T3K | "6, direct, amp only captures", Red mode |
| Recto | 69206 | Mesa Dual Rectifier MW Red Modern - Mesa 4x12 - Full Rig | amp-cab | 116 | T3K | Includes the cab: render with `ir-mix` 0 on our side, or compare against amp+cab |
| Recto | 52 | Mesa Boogie Dual Rectifier | amp | 4 | T3K | Boosted and unboosted |
| Recto | 62644 | Dual Rectifier 3 Channel | amp | 6 | T3K | Red channel Modern, TS808 boost |
| Recto | 943 | Mesa Boogie Dual Rectifier Rev F Green Essentials | amp | 12 | T3K | Clean channel. Not a Recto preset target. |
| Tweed | 1091 | Fender Tweed Deluxe 5E3 Captures | amp | 12 | T3K | StewMac 5E3 clone. Volume 1 / Volume 2 / Tone given per capture. |
| Tweed | 4976 | DIY Tweed Deluxe 5E3 Head (11.5dBu) | amp | 200 | T3K | DIY 5E3 |
| Tweed | 75305 | Tweed Deluxe 5E3 DI | amp | 2 | T3K | |
| Tweed | 1691 | Fender 57 Custom Tweed Deluxe (DI) | amp | 2 | T3K | Fender reissue, "close to breakup" |
| Tweed | 90842 | Fender Tweed Deluxe 5E3 1957 - Crunch - A2 | amp-cab | 1 | T3K | Original 1957; T8 VI3 VII0, Jensen P12R, miked |
| Tweed | 27920 | 58 5e3 Clean | amp-cab | 8 | T3K | 1958 Deluxe, with cab |
| Chainsaw | 571 | Boss HM-2 Mega Pack | pedal | 17 | T3K | Several settings and model sizes |
| Chainsaw | 722 | Boss HM-2 Pack | pedal | 6 | T3K | Includes "everything turned all the way up" (the Swedish setting) |
| Frost | 67991 | Peavey Bandit 112 Lead Channel | amp | 6 | T3K | 2019 Bandit (solid state), knobs at noon |
| Frost | 29995 | Marshall Valvestate 8100 Full Rig | amp-cab | 2 | T3K | SD-1 + graphic EQ boost, G8.5 B8 M4.5 T2.5 |
| Frost | 215 | Boss DS-1 Pack | pedal | 4 | T3K | |

Pages are `https://www.tone3000.com/tones/<slug>-<id>`. The licence is the `license` field of the tone record
embedded in the page.

HM-2 captures are pedal-only. A Chainsaw reference needs the pedal followed by an amp. Either chain HM-2 capture → amp
capture (for example a 6505 or a solid-state amp capture), or chain HM-2 capture → a real JVM recording at a matching
setting (impossible here, because the JVM files are fixed recordings). Record which chain is used. The Frost rows are
suggestions only: the project has never named a target amp for Frost (§D3).

### A3. Wright et al., Blackstar HT-1 (GPL-3.0), low relevance

`Alec-Wright/Automated-GuitarAmpModelling` (DAFx-19) ships HT-1 and Big Muff input/target WAVs in `Data/`. The
repository licence is GPL-3.0. The HT-1 is a 1 W valve amp on its overdrive channel, which does not fill any of the
five gaps. It is listed because its licence is not CC BY-NC (ToneTwist re-hosts it as CC BY-NC). Vendoring GPL data
needs the licence decision in §F first.

## B. Exists, but the licence or permission blocks it

### B1. Peavey 6505 LEAD, Cassidy & De Sena (DAFx23): the best 5150 match found

- Paper: W. J. Cassidy and E. De Sena, "Perceptual Evaluation and Genre-Specific Training of Deep Neural Network
  Models of a High-Gain Guitar Amplifier", Proc. DAFx23, Copenhagen. The paper is CC BY 4.0.
- Hardware: Peavey 6505 (the same circuit as the 5150; Wikipedia: "identical to the 5150, except for its name and the
  brand of tubes used"), LEAD channel. The input was re-amped through a Radial X-Amp (10 kΩ). The output went through a
  Rivera RockCrusher reactive load into an Apollo Twin X, with **no cabinet**. That is the same situation as our JVM
  references.
- Audio: `github.com/IoSR-Surrey/DNNAmplifierDemos`, folder `Real Amplifier/`, RA.wav to RH.wav: 8 excerpts of 5–8 s,
  44.1 kHz/16-bit. The same folder has DNN-model and anchor renders.
- **Blockers:** (1) the repository has no licence file, so it is all rights reserved; (2) no DI is published, so no
  candidate can be rendered from the same performance; (3) the excerpts are performances of commercial songs
  (B.Y.O.B., My Curse, …), which adds a composition-copyright question even with a licence.
- The training DIs and the 6505 output recorded for them (Section 4.4 of the paper) would be the ideal 5150 reference.
  They are not published. Ask the authors (§E).

### B2. Schmitz & Embrechts (Uliège): upstream of the Mark V data, plus Engl, Mesa 550, HT-5 Metal

- Papers: T. Schmitz and J.-J. Embrechts, "Introducing a Dataset of Guitar Amplifier Sounds for Nonlinear Emulation
  Benchmarking", AES 145th Convention, 2018; "Objective and subjective comparison of several machine learning
  techniques applied for the real-time emulation of the guitar amplifier nonlinear behavior", AES 146th Convention,
  2019.
- Data: `github.com/TSchmitzULG/NeuralNetwork_Comparison/tree/master/Datasets` holds training/val/test `.mat` files
  for Mesa Mark V (Clean, Crunch, **Disto**), Mesa 550 (Clean, Crunch, Disto), Engl Retro Tube 50 (Disto, plus
  `outENGL1..10.wav`, a 10-step gain sweep), Blackstar HT-5 Metal and Ibanez TSA15, with a shared input. The original
  download page, `montefiore.uliege.be/services/acous/STSI/downloads.php`, now returns 404.
- **Blocker:** the GitHub repository has no licence. ToneTwist re-hosts five of these sets as CC BY-NC 4.0
  (10796864 and others), which is the licence the project hit. The upstream terms are not more permissive; they are
  unstated.
- If the authors relicensed this data as CC BY, Recto would get an in-repo **partial** reference (Mark V Disto). The
  Engl 10-step gain sweep would also give a second high-gain family alongside the JVM.

### B3. GuitarML Fender Blues Jr

ToneTwist record 10797977 says "No publication is associated with this data" and "Original Data N.A.". No upstream
copy with other terms was found. The CC BY-NC re-host is the only source. Only Keith Bloemer (GuitarML) can change
that.

### B4. Other blocked items

- TONE3000 **CC BY-NC** "BOSS HM-2 Heavy Metal (MiJ) v2.0" (1104, 24 models, settings "Swede", "At the Gates",
  "Godflesh") is local-only at most. **CC BY-NC-ND** "Tweed Deluxe 1958 DI" (1614) cannot be used.
- Wright, Dal Santo & Schlecht, "Predicting Subjective Ratings of Guitar Amplifier Emulations – Audio Corpus"
  (doi:10.5281/zenodo.19112212, 2026-06-04): access is restricted, with no licence and no file list. Worth checking
  again once the paper is out.

## C. Checked and rejected

| Source | Licence | Why rejected |
|---|---|---|
| GUITAR-FX-DIST, Comunità (zenodo 4296040 / 4298000 / 4298017 / 4298025) | CC BY 4.0 | Processed with **plugins** (Audified Multidrive Pedal Pro, which emulates the MT-2, DS-1 and others; Mercuriall Greed Smasher), not hardware |
| ToneTwist AFx entries dated 2026-05-15 (20213113, 20213568, 20213928, …) | CC BY 4.0 | Software (TH3 Overloud presets, Audacity effects) |
| ToneTwist AFx hardware entries (all others) | CC BY-NC 4.0 | Same block as now. Upstreams traced in §B. |
| EGFxSet, Pedroza et al. (zenodo 7044411) | CC BY 4.0 | Real BD-2, Mini TS and RAT2, but pedals only with no amp. Could serve later as a real boost-pedal reference. |
| pOD-set, Dal Rì et al. (zenodo 15389653) | CC BY-NC 4.0 | Overdrive pedals, non-commercial |
| Guitar-TECHS, Pedroza et al. (zenodo 14963133) | CC BY 4.0 | DI + miked amp, but Orange CR60, Orange Crush 12 and a Yamaha bass amp at flat EQ: low gain, no gap match |
| EGSet12 (zenodo 11406378) | CC BY 4.0 | Yamaha B15 miked, no DI |
| GOAT (arXiv 2509.22655) | paper CC BY 4.0 | Amp audio is DIs re-amped through NAM profiles. The dataset's own hosting and licence were not confirmed. |
| Blues-to-jazz web-audio amp sims, Buffa et al. (zenodo 6769408) | CC BY 4.0 | Simulator description, not real-amp data |

Hugging Face dataset search ("guitar", "amp", "amplifier", "distortion") found nothing relevant.

## D. Circuit and voicing facts (cite-only, for DSP work)

Facts are not copyrightable, so these can inform the voicing table with a citation. The texts themselves stay out of
the repo.

### D1. Boss HM-2 (Chainsaw)

- EQ: three gyrator (simulated-inductor) band filters. The **Low** band is centred at **86.79 Hz**. **High** drives
  two gyrators at once, at **958.47 Hz and 1278.6 Hz**. Fully up, each boosts its band by about **10 dB**
  (Catalinbread, "Boss HM-2", 2020-08-28). BOSS's own HM-2W analysis puts Low at "around 80 to 90 Hz" and calls the
  overall curve "double-humped".
- Gain path: a pregain stage built from an NPN/PNP transistor network, an asymmetric clipping amplifier, two clipper
  pairs, a buffer, then the three gyrator filters (mirosol.kapsi.fi, "Boss HM-2 Heavy Metal", 2014).
- **Compared with `Source/DSP/Voicings.h` (Chainsaw):** bass 110 Hz, mid 1000 Hz ±8 dB, treble 1800 Hz. The pedal's
  low peak is about 23 Hz lower. Its upper peak is two close resonances (958 Hz and 1279 Hz) rather than one at
  1000 Hz plus a treble band at 1800 Hz. In the pedal the EQ comes **after** the clipping, whereas our tone stack runs
  at the preamp's tone-stack position. A capture-derived pair (§A2) would test whether these differences can be heard.

### D2. Peavey 5150 / 6505 (5150)

Lead channel: "cascading five preamps (actually four preamplifiers and one phase inverter)", 5 × 12AX7, 4 × 6L6,
120 W RMS. Pre and post gain per channel; shared 3-band EQ plus resonance and presence (Wikipedia, "Peavey 5150"). Our
5150 row has two shaper stages. That fits the open item in `research/G-010-preamp-research.md`, which points to
stage-by-stage gain structure as the remaining lever.

### D3. Frost / black metal

No amp-level quantitative source for a black-metal voicing was found. The genre has no single reference amp: period
gear ranges from solid-state combos (Peavey Bandit, Marshall Valvestate) to boosted valve heads. Herbst (2017, below)
quotes black metal's preference for low-fidelity aesthetics (Hagen 2011), but measures only mixed production tracks.
Frost remains the least-supported voicing. A decision on its intended real-world target would decide which §A2 capture
to use.

### D4. Genre spectra (all voicings, context only)

J.-P. Herbst, "Historical development, sound aesthetics and production techniques of the distorted electric guitar in
metal music", *Metal Music Studies* 3(1), 2017, pp. 23–46 (eprints.hud.ac.uk/id/eprint/33248). The figures come from
isolated guitar tracks of 50 released recordings, so they describe produced tracks, not amp outputs:

- Lower mids (250–1,000 Hz) and mids (1–2 kHz) carry more weight in 1970s/1980s metal than in later genres.
- The first energy cut-off rises from about 4.66 kHz (1970s) to about 6.45 kHz (2010s); the text prints "6.45 Hz",
  evidently kHz. The second cut-off rises from 8.20 kHz to 13.31 kHz between the 1970s and 1980s.
- Average RMS of the guitar tracks: −16.99 dB (1970s) up to −9.90 dB (2000s death metal).

Repository terms allow personal research and non-profit use only, unchanged and with credit. Cite it; do not copy it.

## E. Requests that would turn blocked material into in-repo references

1. **Thomas Schmitz / Jean-Jacques Embrechts (Uliège):** ask for a CC BY licence on the `NeuralNetwork_Comparison`
   datasets, at least Mark V Disto and the Engl gain sweep. This upgrades Recto to in-repo and adds a second high-gain
   family.
2. **Will Cassidy / Enzo De Sena (University of Surrey IoSR):** ask for the 6505 LEAD training DIs and outputs from
   Section 4.4, under CC BY. This gives an in-repo **close** reference for the five 5150 presets.
3. **Keith Bloemer (GuitarML):** ask for a CC BY licence on the Blues Jr input/target pair.

## F. Open question for the owner

`references/README.md` says "This repository's source is Apache-2.0", but `LICENSE` and the top-level README say
AGPL-3.0. This decides whether GPL-3.0 data (§A3) can be vendored, and should be settled before any licence-dependent
step. It is not changed here.

## Suggested next steps (not done in this round)

1. Add a `Frost` rule using JVM `B5_M0_T5_G5` / `B5_M5_T5_G10`, graded **weak**.
2. Add a `capture` tier to the manifest: fetch T3K `.nam` files into ignored `listening-work/`, render the vendored
   CC BY DIs through them, and commit the renders with the capture id, model architecture and chain recorded. Start
   with 72177 (5150), 1768 (Recto), 1091 (Tweed) and 722 → 72177 (Chainsaw). Score these pairs separately from
   real-recording pairs.
3. Send the three requests in §E.
4. Re-amping our own DIs through a real HM-2 and a chosen Frost rig (README, "Gaps and how to close them") is still the
   only route found to *recordings* for Chainsaw and Frost.
