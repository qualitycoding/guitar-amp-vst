# Attribution for files in `references/audio/`

The audio excerpts in `references/audio/` are **not** covered by this repository's Apache-2.0 licence. They are
excerpts of third-party datasets licensed under the
[Creative Commons Attribution 4.0 International licence (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/).

## Marshall JVM 410H Dataset

Directories: `p02`–`p06`, `p09`, `p10`, `p16`–`p18`, `p21`, `p23` (`ref.flac` and `di.flac` in each)

- **Creators:** Štěpán Miklánek, Alec Wright, Vesa Välimäki, Jiří Schimmel
- **Title:** Marshall JVM 410H Dataset (Zenodo, 2023)
- **Source:** <https://doi.org/10.5281/zenodo.7970723>
- **Licence:** CC BY 4.0
- **Associated paper:** Š. Miklánek, A. Wright, V. Välimäki and J. Schimmel, "Neural Grey-Box Guitar Amplifier
  Modelling with Limited Data", *Proc. DAFx23*, Copenhagen, 2023.

## EG-IPT Dataset (Electric Guitar Instrumental Playing Techniques)

Directories: `p19`, `p20` (`ref.flac` and `di.flac` in each)

- **Creators:** Marco Fiorini, Nicolas Brochec, Joakim Borg, Riccardo Pasini
- **Title:** EG-IPT Dataset (Electric Guitar Instrumental Playing Techniques) (Zenodo, 2025)
- **Source:** <https://doi.org/10.5281/zenodo.15205644>
- **Licence:** CC BY 4.0
- **Associated paper:** "Introducing EG-IPT and ipt~: a novel electric guitar dataset and a new Max/MSP object for
  real-time classification of instrumental playing techniques", NIME 2025, Canberra.

## Changes made

These are changes to the original material, as CC BY 4.0 requires them to be indicated.

- Each excerpt is cut from a longer recording. For EG-IPT, six separate note files are joined into one file.
- Stereo is mixed down to mono where applicable.
- EG-IPT audio is resampled from 96 kHz to 44.1 kHz.
- A 10 ms raised-cosine fade is applied at each cut.
- Excerpts are re-encoded as 24-bit FLAC.

The exact source members and time windows are listed in `manifest.json`. `scripts/fetch_references.py`
reproduces the excerpts.

No endorsement by the dataset creators is implied.

## Not included: local-only material

The Fender Blues Junior and Mesa/Boogie Mark V references come from the ToneTwist AFx Dataset (records
10.5281/zenodo.10797977 and 10.5281/zenodo.10796864). They are licensed CC BY-NC 4.0 and are **not** stored in this
repository. `scripts/fetch_references.py fetch` cuts them into an ignored local directory. Do not commit them or
ship them with the plugin.
