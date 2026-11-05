# Reference recordings for the factory presets

Real-amplifier recordings to serve as the realism references for the blind listening gate (G-010,
`LISTENING-AMENDMENT.md`). Each reference has a **DI recorded at the same time**, so the GrimAmp candidate
can be rendered from exactly the same performance. That is what makes a matched A/B pair.

The machine-readable record, with every source member, excerpt window, setting and checksum, is
[`manifest.json`](manifest.json). The script is [`../scripts/fetch_references.py`](../scripts/fetch_references.py).

## Coverage

| # | Preset | Voicing | Real amp in the reference | Setting | Match | Where |
|---|---|---|---|---|---|---|
| 01 | 70s Blues Rock | Tweed | Fender Blues Junior | volume 10 | partial | fetch locally (CC BY-NC 4.0) |
| 02 | Southern Rock | Plexi | Marshall JVM 410H (OD1) | B5 M6 T5 G5 | partial | in repo (CC BY 4.0) |
| 03 | Classic Rock Crunch | Plexi | Marshall JVM 410H (OD1) | B5 M5 T5 G5 | partial | in repo (CC BY 4.0) |
| 04 | 80s Hair Metal | JCM800 | Marshall JVM 410H (OD1) | B5 M5 T5 G6 | close | in repo (CC BY 4.0) |
| 05 | Thrash Attack | JCM800 | Marshall JVM 410H (OD1) | B5 M5 T5 G8 | close | in repo (CC BY 4.0) |
| 06 | Bay Area Lead | JCM800 | Marshall JVM 410H (OD1) | B5 M5 T5 G8 | close | in repo (CC BY 4.0) |
| 07 | 90s Groove | Recto | Mesa/Boogie Mark V (Extreme) | gain 5 | partial | fetch locally (CC BY-NC 4.0) |
| 08 | Nu-Metal Drop | Recto | Mesa/Boogie Mark V (Extreme) | gain 5 | partial | fetch locally (CC BY-NC 4.0) |
| 09 | Death Metal Chains | 5150 | Marshall JVM 410H (OD1) | B5 M5 T5 G8 | weak | in repo (CC BY 4.0) |
| 10 | Florida Trem | 5150 | Marshall JVM 410H (OD1) | B5 M5 T5 G8 | weak | in repo (CC BY 4.0) |
| 11 | Swedish Chainsaw | Chainsaw | — | — | none | **none found** |
| 12 | Stockholm Buzz | Chainsaw | — | — | none | **none found** |
| 13 | Black Metal Frost | Frost | — | — | none | **none found** |
| 14 | Cavernous Hiss | Frost | — | — | none | **none found** |
| 15 | Doom Sludge | Recto | Mesa/Boogie Mark V (Extreme) | gain 5 | partial | fetch locally (CC BY-NC 4.0) |
| 16 | Funeral March | 5150 | Marshall JVM 410H (OD1) | B8 M5 T5 G5 | weak | in repo (CC BY 4.0) |
| 17 | Djent Machine | 5150 | Marshall JVM 410H (OD1) | B5 M5 T5 G6 | weak | in repo (CC BY 4.0) |
| 18 | Modern Prog Lead | 5150 | Marshall JVM 410H (OD1) | B5 M5 T5 G6 | weak | in repo (CC BY 4.0) |
| 19 | Djent Clean | Tweed | EVH 5150 III clean, Mesa 4×12 V30 | neck pickup, SM57 close | partial | in repo (CC BY 4.0) |
| 20 | Ambient Washtub | Plexi | EVH 5150 III clean, Mesa 4×12 V30 | both pickups, R14 ribbon at 24 cm | partial | in repo (CC BY 4.0) |
| 21 | Crunch Rhythm | JCM800 | Marshall JVM 410H (OD1) | B5 M5 T5 G4 | close | in repo (CC BY 4.0) |
| 22 | Edge of Breakup | Tweed | Fender Blues Junior | volume 5 | partial | fetch locally (CC BY-NC 4.0) |
| 23 | Solo Scream | JCM800 | Marshall JVM 410H (OD1) | B5 M5 T5 G8 | close | in repo (CC BY 4.0) |
| 24 | Midnight Frost Lead | Frost | — | — | none | **none found** |

That gives 14 references in the repository, 5 that can be fetched for local use only, and 5 presets with no
usable real recording. The 14 in-repo pairs are enough for the frozen 12-pair primary set.

**Match grades.**
- **close** means the same amp family and gain structure.
- **partial** means the right gain range, but a different circuit or a different cabinet situation.
- **weak** means only the gain range matches. These are real high-gain valve recordings, but with a Marshall voicing
  standing in for a 5150.

Each entry's `why` field in the manifest gives the reason. A weak match will tell you whether GrimAmp
sounds like *a real amp*, not whether it sounds like *the voicing it is named after*. Report it that way.

## Sources

| Key | Dataset | Hardware | Licence | Use |
|---|---|---|---|---|
| `jvm410h` | Marshall JVM 410H Dataset, Miklánek, Wright, Välimäki & Schimmel (DAFx23), [doi:10.5281/zenodo.7970723](https://doi.org/10.5281/zenodo.7970723) | Marshall JVM 410H, OD1 channel. One parameter is varied at a time from B5 M5 T5 G5. DI and **power-amp speaker output**, with no cabinet or microphone | CC BY 4.0 | in repo |
| `egipt` | EG-IPT Dataset, [doi:10.5281/zenodo.15205644](https://doi.org/10.5281/zenodo.15205644) | Gibson SG into an EVH 5150 III 50W 6L6 (controls flat) and a Mesa 4×12 with Celestion V30s. Shure SM57 and Peluso R14 microphones, plus a BSS AR-133 DI | CC BY 4.0 | in repo |
| `tt_bluesjr` | ToneTwist AFx: Fender Blues Jr, [doi:10.5281/zenodo.10797977](https://doi.org/10.5281/zenodo.10797977) | Fender Blues Junior at volume 5 and 10, with a DI | CC BY-NC 4.0 | local only |
| `tt_markv` | ToneTwist AFx: Mesa/Boogie Mark V, Extreme channel, [doi:10.5281/zenodo.10796864](https://doi.org/10.5281/zenodo.10796864) | Mark V, Extreme channel, gain 5, with a DI (Schmitz & Embrechts data) | CC BY-NC 4.0 | local only |

Attribution and the list of changes, which CC BY requires, are in [`ATTRIBUTION.md`](ATTRIBUTION.md).

**Why the CC BY-NC sources are not committed.** This repository's source is Apache-2.0, and binaries linking JUCE
are AGPL-3.0 or commercially licensed. Committing non-commercial material would put a field-of-use restriction
into a tree that otherwise has none. The script cuts those excerpts into `listening-work/`, which is ignored by Git.
This matches the private-local evidence route that `listening/G-010/round-001/result.json` already records.

## Rendering the candidates

Render each candidate from `di/<stem>.wav`, using the preset named in the manifest and applying that entry's
`render_overrides`:

- **JVM 410H references** are the amp's speaker output into a load. There is no cabinet in them, so render the
  candidate with the cabinet bypassed (`ir-mix` 0). Otherwise the comparison is amp-plus-cab against amp alone.
- **EG-IPT references** include a real 4×12 and a microphone, so render with the preset as it stands.
- **ToneTwist references**: the dataset does not say whether the amp was miked or recorded through a load box.
  Find out before choosing `ir-mix`. Until then, treat these pairs as diagnostic.

Settings are as close as the recordings allow, not exact. The JVM data covers one knob at a time around noon. The
nearest setting is chosen by mapping preset knobs 0–1 to 0–10, with drive mapped to gain (+1 when the boost is on);
gain differences weigh double. The manifest keeps the target values next to the setting that was used.

## Commands

```bash
# Check the committed excerpts against the manifest (no network access)
python3 scripts/fetch_references.py build --verify

# Cut the CC BY-NC excerpts locally (never commit these)
python3 scripts/fetch_references.py fetch --local-dir listening-work/references-nc

# Write ref/ and di/ WAVs (44.1 kHz PCM-24) for the pair builder
python3 scripts/fetch_references.py export --local-dir listening-work/references-nc \
    --out listening-work/G-010/round-001
#   ... render candidates from listening-work/G-010/round-001/di into .../cand, same file names ...
python3 scripts/make_blind_pairs.py --ref listening-work/G-010/round-001/ref \
    --cand listening-work/G-010/round-001/cand --out listening-work/G-010/round-001/blind \
    --seed 20261103 --pairs 12 --window start --seconds 1.0 --rms-db -23.0 --fade 0.010 --fs 44100

# Rebuild the committed excerpts from Zenodo (about 600 MB of range reads; --cache keeps them)
python3 scripts/fetch_references.py --cache /tmp/refcache build
```

The pair builder takes the first `--pairs` stems in sorted order. To pick which presets go into the scored set,
export into a fresh directory and remove the stems you do not want before building pairs. Record that choice
before listening.

## How the excerpts were chosen

- **Length and format.** Each excerpt is 12 s, mono, 44.1 kHz, 24-bit FLAC, with 10 ms raised-cosine fades.
  The same cut is applied to the reference and the DI. Checksums are taken over the 24-bit PCM samples, not the
  FLAC bytes, so they do not depend on the encoder version.
- **JVM 410H.** The same DI performance runs through every setting. Windows come only from 80–300 s of the file,
  because earlier sections include bass guitar and the end is test signals. Every second in a window must be
  above −40 dBFS. Lead presets take the windows with the sparsest onsets (sustained notes); rhythm presets take the
  densest. Window starts are at least 6 s apart, so every preset has a different onset in the 1 s blind window.
- **EG-IPT.** Six single notes (strings 6 to 1, first take of the *ordinario* set), 2 s each, are joined into one
  file. The source is 96 kHz and is resampled to 44.1 kHz.
- **ToneTwist.** These are 12 s windows of the DI with no second below −40 dBFS. The first 2 s are skipped
  because they hold sync markers.

## Checks made

- **Alignment.** Onset functions of the reference and DI peak at 0 ms lag (within 6 ms resolution) for all 14
  in-repo pairs.
- **JVM excerpts are guitar, not bass.** DI energy at 30–75 Hz is at most 2 % in every excerpt.
- **EG-IPT is clean.** The SM57 track has about −43 dB of its energy above 2 kHz, against about −14 dB in the JVM
  OD1 output. Its crest factor drops only slightly from the DI. The dataset only says "controls flat", so this
  conclusion comes from that measurement.
- **Upstream naming slip.** The JVM archive's `final/B5_M5_T5_G6/` folder holds files named `B5_M5_T5_G4-*`.
  Their CRCs differ from the real G4 files. Measured gain on the same DI passage rises steadily: G4 +7.13 dB,
  G5 +7.32 dB, "G6" +7.48 dB, G8 +8.23 dB. They are treated as G6; see `JVM_MEMBER_STEM` in the script.
- **Mark V peaks.** The upstream Mark V target is normalised to 0 dBFS (4 samples per excerpt at full scale).
  The pair builder rescales to −23 dBFS RMS, so this does not matter for the test.

## Gaps and how to close them

A search for more material (2026-11-05) is written up in
[`../research/G-010-reference-gap-research.md`](../research/G-010-reference-gap-research.md). It found no new CC BY
or CC0 real recording with a paired DI. It did find these: a weak in-repo Frost proxy (JVM `B5_M0_T5_G5` and `G10`);
TONE3000 captures of a real HM-2, Dual Rectifier, 6505 and 5E3 under the T3K licence, which allows rendered output to
be published; and the unlicensed upstreams of the Mark V data and of a real 6505 LEAD recording, which their authors
could relicense.

- **Chainsaw (HM-2), presets 11 and 12.** No real Boss HM-2 recording with a paired DI was found under an open or
  research licence. Neural captures of HM-2s exist, but they are models rather than recordings, and their usual
  licences forbid redistribution.
- **Frost, presets 13, 14 and 24.** No black-metal-voiced real-amp recording with a paired DI was found.
- **Tweed and Recto.** Closer matches exist only under non-commercial terms (and are Blues Junior and Mark V
  rather than a 5E3 or a Dual Rectifier).
- **5150 high gain.** EG-IPT has a real 5150 III, but only clean.

The robust fix for all of these is to re-amp: send DI recordings through the real amp or pedal and record the
output. This gives exact settings matching as well. If you use your own DIs, you can release the recordings under
your own terms. If you use the CC BY DIs from this folder, the attribution must carry over. If
you do this, add the result as a new `SOURCES` entry, with `vendored` set to true and its licence, and add a
rule for it.
