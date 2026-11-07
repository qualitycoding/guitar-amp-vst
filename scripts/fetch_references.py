#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Reference recordings of real amplifiers for the factory presets (G-010 realism gate).

Every reference is a real-hardware recording with a simultaneously captured DI, so a candidate can be rendered from
exactly the same performance. Sources are read straight out of their Zenodo archives with HTTP range requests, so only
the members that are needed are downloaded.

Subcommands
  author    Re-derive the preset-to-source mapping and excerpt windows and rewrite references/manifest.json.
            Only needed when the mapping rules change; the committed manifest is the source of truth.
  build     Cut the vendored (CC BY 4.0) excerpts named in the manifest into references/audio/ as FLAC, or with
            --verify only check that the committed files match the manifest's SHA-256 values.
  fetch     Cut the local-only (CC BY-NC 4.0) excerpts into a directory outside the repository. Never commit them.
  export    Write <out>/ref/<stem>.wav and <out>/di/<stem>.wav at 44.1 kHz PCM-24 for every available reference,
            ready for scripts/make_blind_pairs.py --ref <out>/ref (render candidates from <out>/di).

  python3 scripts/fetch_references.py build --verify
  python3 scripts/fetch_references.py fetch --local-dir listening-work/references-nc
  python3 scripts/fetch_references.py export --local-dir listening-work/references-nc --out listening-work/G-010/round-001
"""
import argparse
import hashlib
import io
import json
import pathlib
import sys
import urllib.request
import xml.etree.ElementTree as ET
import zipfile

import numpy as np
import soundfile as sf

ROOT = pathlib.Path(__file__).resolve().parent.parent
REF_DIR = ROOT / "references"
AUDIO_DIR = REF_DIR / "audio"
MANIFEST = REF_DIR / "manifest.json"
PRESET_DIR = ROOT / "Resources" / "presets"
FS = 44100
EXCERPT_S = 12.0
FADE_S = 0.010

SOURCES = {
    "jvm410h": {
        "title": "Marshall JVM 410H Dataset",
        "creators": "Štěpán Miklánek, Alec Wright, Vesa Välimäki, Jiří Schimmel",
        "doi": "10.5281/zenodo.7970723",
        "url": "https://zenodo.org/records/7970723/files/MarshallJVM410H.zip?download=1",
        "licence": "CC-BY-4.0",
        "hardware": "Marshall JVM 410H, OD1 channel; DI input and power-amp speaker output (no cabinet or microphone)",
        "paper": "Miklánek et al., 'Neural Grey-Box Guitar Amplifier Modelling with Limited Data', DAFx23",
        "vendored": True,
    },
    "egipt": {
        "title": "EG-IPT Dataset (Electric Guitar Instrumental Playing Techniques)",
        "creators": "Marco Fiorini, Nicolas Brochec, Joakim Borg, Riccardo Pasini",
        "doi": "10.5281/zenodo.15205644",
        "url": "https://zenodo.org/records/15205644/files/EG-IPT.zip?download=1",
        "licence": "CC-BY-4.0",
        "hardware": "Gibson SG Standard into EVH 5150 III 50W 6L6 (controls flat, clean by measurement), Mesa 4x12 with "
                    "Celestion V30; Shure SM57 at 2.5 cm (dyn), Peluso R14 at 24 cm (rib); BSS AR-133 DI",
        "vendored": True,
    },
    "tt_bluesjr": {
        "title": "ToneTwist AFx Dataset - External: Fender Blues Jr",
        "creators": "Keith Bloemer (GuitarML); compiled by Marco Comunità",
        "doi": "10.5281/zenodo.10797977",
        "url": "https://zenodo.org/records/10797977/files/Fender-BluesJr.zip?download=1",
        "licence": "CC-BY-NC-4.0",
        "hardware": "Fender Blues Junior, volume 5 and 10",
        "vendored": False,
    },
    "tt_markv": {
        "title": "ToneTwist AFx Dataset - External: MesaBoogie Mark V - Channel: Extreme",
        "creators": "Thomas Schmitz, Jean-Jacques Embrechts; compiled by Marco Comunità",
        "doi": "10.5281/zenodo.10796864",
        "url": "https://zenodo.org/records/10796864/files/MesaBoogie-MarkV-ChExtreme.zip?download=1",
        "licence": "CC-BY-NC-4.0",
        "hardware": "Mesa/Boogie Mark V, Extreme channel, gain 5",
        "vendored": False,
    },
}


# --------------------------------------------------------------------------------------------- remote zip access
class _HTTPRange(io.RawIOBase):
    def __init__(self, url):
        self.url, self.pos = url, 0
        head = urllib.request.urlopen(urllib.request.Request(url, method="HEAD"), timeout=120)
        self.size = int(head.headers["Content-Length"])

    def seekable(self):
        return True

    def readable(self):
        return True

    def tell(self):
        return self.pos

    def seek(self, off, whence=0):
        self.pos = off if whence == 0 else self.pos + off if whence == 1 else self.size + off
        return self.pos

    def readinto(self, buf):
        if self.pos >= self.size or not len(buf):
            return 0
        end = min(self.pos + len(buf), self.size) - 1
        req = urllib.request.Request(self.url, headers={"Range": f"bytes={self.pos}-{end}"})
        data = urllib.request.urlopen(req, timeout=600).read()
        buf[:len(data)] = data
        self.pos += len(data)
        return len(data)


_ZIPS = {}


def _zip(source):
    if source not in _ZIPS:
        _ZIPS[source] = zipfile.ZipFile(io.BufferedReader(_HTTPRange(SOURCES[source]["url"]), buffer_size=1 << 20))
    return _ZIPS[source]


_MEMBERS = {}


def read_member(source, member, cache=None):
    """Return (mono float64 audio, fs) for one archive member, caching the raw WAV bytes on disk if asked."""
    key = (source, member)
    if key not in _MEMBERS:
        path = pathlib.Path(cache) / source / member if cache else None
        if path is not None and path.exists():
            raw = path.read_bytes()
        else:
            raw = _zip(source).open(member).read()
            if path is not None:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(raw)
        x, fs = sf.read(io.BytesIO(raw), always_2d=True)
        _MEMBERS[key] = (x.mean(axis=1), fs)
    return _MEMBERS[key]


# --------------------------------------------------------------------------------------------- signal helpers
def resample(x, fs_from, fs_to=FS):
    if fs_from == fs_to:
        return x
    from math import gcd
    from scipy.signal import resample_poly
    g = gcd(int(fs_from), int(fs_to))
    return resample_poly(x, fs_to // g, fs_from // g)


def fade(x, n=int(FADE_S * FS)):
    x = x.copy()
    ramp = 0.5 - 0.5 * np.cos(np.pi * np.arange(n) / n)
    x[:n] *= ramp
    x[-n:] *= ramp[::-1]
    return x


def cut_segment(spec, cache):
    """Build the (ref, di) pair for one manifest entry at 44.1 kHz, applying the same edits to both sides."""
    refs, dis = [], []
    for part in spec["parts"]:
        r, fs_r = read_member(spec["source"], part["ref_member"], cache)
        d, fs_d = read_member(spec["source"], part["di_member"], cache)
        r, d = resample(r, fs_r), resample(d, fs_d)
        i0, n = int(part["start_s"] * FS), int(part["length_s"] * FS)
        refs.append(fade(np.pad(r[i0:i0 + n], (0, max(0, n - len(r[i0:i0 + n]))))))
        dis.append(fade(np.pad(d[i0:i0 + n], (0, max(0, n - len(d[i0:i0 + n]))))))
    return np.concatenate(refs), np.concatenate(dis)


def sha256(path):
    return hashlib.sha256(pathlib.Path(path).read_bytes()).hexdigest()


def pcm_sha256(x):
    """Checksum of the 24-bit integer samples, independent of the FLAC encoder version."""
    q = np.clip(np.round(x * (1 << 23)), -(1 << 23), (1 << 23) - 1).astype("<i4")
    return hashlib.sha256(q.tobytes()).hexdigest()


# --------------------------------------------------------------------------------------------- authoring
def load_presets():
    out = []
    for p in sorted(PRESET_DIR.glob("preset*.xml")):
        root = ET.parse(p).getroot()
        vals = {e.get("id"): float(e.get("value")) for e in root.findall("PARAM")}
        out.append({"file": p.name, "index": int(p.stem[6:]), "name": root.get("name"), "genre": root.get("genre"),
                    "values": vals})
    return out


VOICINGS = ["Tweed", "Plexi", "JCM800", "Recto", "5150", "Chainsaw", "Frost"]
JVM_COMBOS = ["B0_M0_T0_G5", "B0_M5_T5_G5", "B10_M10_T10_G5", "B10_M5_T5_G5", "B2_M5_T5_G5", "B4_M5_T5_G5",
              "B5_M0_T5_G5", "B5_M10_T5_G5", "B5_M2_T5_G5", "B5_M4_T5_G5", "B5_M5_T0_G5", "B5_M5_T10_G5",
              "B5_M5_T2_G5", "B5_M5_T4_G5", "B5_M5_T5_G0", "B5_M5_T5_G1", "B5_M5_T5_G10", "B5_M5_T5_G2",
              "B5_M5_T5_G4", "B5_M5_T5_G5", "B5_M5_T5_G6", "B5_M5_T5_G8", "B5_M5_T6_G5", "B5_M5_T8_G5",
              "B5_M6_T5_G5", "B5_M8_T5_G5", "B6_M5_T5_G5", "B8_M5_T5_G5"]

# Per-voicing rule: which source stands in, how good the match is, and why. Grades: close (same amp family and
# gain structure), partial (right gain range, different circuit or cab situation), weak (only the gain range matches).
RULES = {
    "Tweed": None,  # resolved per preset below (clean vs breakup)
    "Plexi": ("jvm410h", "partial", "Marshall family, but the JVM OD1 channel is a hotter, cascaded preamp than a Plexi"),
    "JCM800": ("jvm410h", "close", "Marshall high-gain channel descended from the JCM800 2203 voicing"),
    "Recto": ("tt_markv", "partial", "Mesa/Boogie high-gain channel; Mark V Extreme rather than a Dual Rectifier"),
    "5150": ("jvm410h", "weak", "Real high-gain tube amp at matched gain, but Marshall voicing rather than 5150"),
    "Chainsaw": None,
    "Frost": None,
}
# Upstream naming slip: the files inside final/B5_M5_T5_G6/ are named B5_M5_T5_G4-*.wav, but their CRCs differ from
# the real G4 files and their output level sits between G4 and G8 (checked when this table was authored).
JVM_MEMBER_STEM = {"B5_M5_T5_G6": "B5_M5_T5_G4"}


def jvm_member(combo, kind):
    return f"final/{combo}/{JVM_MEMBER_STEM.get(combo, combo)}-{kind}.wav"


LEAD = {"thrash-lead", "lead", "prog"}
RHYTHM = {"thrash", "death-metal", "groove", "nu-metal", "djent", "utility", "hair-metal", "classic-rock",
          "southern-rock"}


def jvm_combo(v):
    """Nearest recorded JVM setting: preset 0..1 knobs map to 0..10; drive (+1 with the boost on) maps to gain."""
    tgt = {"B": v["bass"] * 10, "M": v["mid"] * 10, "T": v["treble"] * 10,
           "G": min(10.0, v["drive"] * 10 + (1.0 if v["boost-on"] else 0.0))}

    def cost(c):
        k = dict((s[0], float(s[1:])) for s in c.split("_"))
        return 2 * abs(k["G"] - tgt["G"]) + sum(abs(k[x] - tgt[x]) for x in "BMT")
    return min(JVM_COMBOS, key=cost), tgt


def jvm_windows(cache):
    """Score 12 s windows of the JVM DI (guitar-only region 80-300 s; earlier parts include bass guitar)."""
    d, fs = read_member("jvm410h", "final/B5_M5_T5_G5/B5_M5_T5_G5-input.wav", cache)
    d = resample(d, fs)
    n, hop = int(EXCERPT_S * FS), FS
    frame = 1024
    mag = np.abs(np.fft.rfft(d[: len(d) // frame * frame].reshape(-1, frame) * np.hanning(frame), axis=1))
    flux = np.maximum(0, np.diff(mag, axis=0)).sum(axis=1)
    thr = np.median(flux) * 3
    onsets = np.flatnonzero((flux[1:-1] > thr) & (flux[1:-1] > flux[:-2]) & (flux[1:-1] >= flux[2:])) + 1
    onset_t = (onsets + 1) * frame / FS
    out = []
    for i0 in range(80 * FS, 300 * FS - n, hop):
        s = d[i0:i0 + n]
        spec = np.abs(np.fft.rfft(s)) ** 2
        f = np.fft.rfftfreq(n, 1 / FS)
        low = spec[(f > 30) & (f < 75)].sum() / spec[f > 30].sum()
        sec = s[: len(s) // FS * FS].reshape(-1, FS)
        quiet = (20 * np.log10(np.sqrt((sec ** 2).mean(axis=1)) + 1e-12)).min()
        if low > 0.03 or quiet < -40:
            continue
        t0 = i0 / FS
        rate = ((onset_t >= t0) & (onset_t < t0 + EXCERPT_S)).sum() / EXCERPT_S
        out.append({"start_s": t0, "onsets_per_s": round(float(rate), 2)})
    return out


EGIPT_NOTES = {
    # string 6 to 1, first take of each, ordinario articulation
    "neck": ["01-02_6s", "02-02_5s", "03-02_4s", "04-02_3s", "05-02_2s", "06-02_1s"],
    "couple": ["07-02_6s", "08-02_5s", "09-02_4s", "10-02_3s", "11-02_2s", "12-02_1s"],
}


def egipt_parts(pickup, mic):
    parts = []
    for note in EGIPT_NOTES[pickup]:
        base = f"EG-IPT/HB-{pickup}/{{m}}/ordinario/HB-{pickup}_ordinario_{note}_{{m}}.wav"
        parts.append({"ref_member": base.format(m=mic), "di_member": base.format(m="DI"), "start_s": 0.0,
                      "length_s": 2.0})
    return parts


def active_windows(source, di_member, cache, spacing=EXCERPT_S):
    """Starts of 12 s DI windows with no quiet second, spaced apart, skipping sync markers in the first 2 s."""
    d, fs = read_member(source, di_member, cache)
    d = resample(d, fs)
    n, starts = int(EXCERPT_S * FS), []
    for i0 in range(2 * FS, len(d) - n, FS):
        sec = d[i0:i0 + n].reshape(-1, FS)
        if (20 * np.log10(np.sqrt((sec ** 2).mean(axis=1)) + 1e-12)).min() < -40:
            continue
        if not starts or i0 / FS - starts[-1] >= spacing:
            starts.append(i0 / FS)
    return starts


def cmd_author(a):
    presets = load_presets()
    windows = jvm_windows(a.cache)
    used = []
    entries = []
    per_source = {}
    for p in presets:
        v = p["values"]
        voicing = VOICINGS[int(v["amp-voicing"])]
        stem = f"p{p['index']:02d}-{p['name'].lower().replace(' ', '-').replace('/', '-')}"
        e = {"preset": p["file"], "name": p["name"], "genre": p["genre"], "voicing": voicing, "stem": stem}
        rule = RULES[voicing]
        if voicing == "Tweed":
            if v["drive"] <= 0.25:
                rule = ("egipt", "partial", "Clean real tube amp and 4x12 V30 cab, but EVH 5150 III clean channel, "
                                            "not a Tweed circuit")
            else:
                rule = ("tt_bluesjr", "partial", "Fender tube combo pushed into breakup; Blues Junior rather than a "
                                                 "5E3 Tweed Deluxe")
        if voicing == "Plexi" and v["drive"] <= 0.35:
            rule = ("egipt", "partial", "Low-gain preset: clean real tube amp with the ribbon mic at 24 cm for the "
                                        "ambient character; 5150 III clean, not a Plexi")
        if rule is None:
            e.update({"tier": "none", "match": "none",
                      "why": {"Chainsaw": "No redistributable or research-licensed real Boss HM-2 recording with a "
                                          "paired DI was found.",
                              "Frost": "No black-metal-voiced real-amp recording with a paired DI was found under an "
                                       "open or research licence."}[voicing]})
            entries.append(e)
            continue
        src, grade, why = rule
        k = per_source[src] = per_source.get(src, -1) + 1  # nth preset using this source, for distinct excerpts
        e.update({"source": src, "tier": "vendored" if SOURCES[src]["vendored"] else "local-only", "match": grade,
                  "why": why})
        if src == "jvm410h":
            combo, tgt = jvm_combo(v)
            want_lead = p["genre"] in LEAD
            # starts at least half an excerpt apart, so every preset gets a different onset for the 1 s blind window
            free = [w for w in windows if all(abs(w["start_s"] - u) >= EXCERPT_S / 2 for u in used)] or windows
            pick = (min if want_lead else max)(free, key=lambda w: (w["onsets_per_s"], -w["start_s"]))
            used.append(pick["start_s"])
            e["setting"] = {"jvm_knobs": combo, "target_knobs": {k: round(x, 2) for k, x in tgt.items()}}
            e["content"] = {"selection": "sparsest onsets (lead)" if want_lead else "densest onsets (rhythm)",
                            "onsets_per_s": pick["onsets_per_s"]}
            e["parts"] = [{"ref_member": jvm_member(combo, "speakerout"),
                           "di_member": jvm_member(combo, "input"),
                           "start_s": pick["start_s"], "length_s": EXCERPT_S}]
            e["render_overrides"] = {"ir-mix": 0.0}
            e["render_note"] = ("Reference is the power-amp speaker output with no cabinet or microphone: render the "
                                "candidate with the cabinet bypassed (ir-mix 0).")
        elif src == "egipt":
            pickup, mic = ("neck", "dyn") if voicing == "Tweed" else ("couple", "rib")
            e["setting"] = {"pickup": f"HB-{pickup}", "mic": mic, "amp": "controls flat (clean)"}
            e["content"] = {"selection": "six single notes, strings 6 to 1, 2 s each"}
            e["parts"] = egipt_parts(pickup, mic)
            e["render_overrides"] = {}
            e["render_note"] = "Reference includes a real 4x12 V30 cab and microphone: render with the preset as is."
        elif src == "tt_bluesjr":
            vol = "V100" if v["drive"] >= 0.4 else "V050"
            e["setting"] = {"volume": int(vol[1:]) / 10}
            di = "DRY/trainvaltest/trainvaltest.input.wav"
            starts = active_windows(src, di, a.cache)
            e["parts"] = [{"ref_member": f"Fender-BluesJr/trainvaltest/{vol}/{vol}.trainvaltest.target.wav",
                           "di_member": di, "start_s": starts[(2 * k + 1) % len(starts)], "length_s": EXCERPT_S}]
            e["render_overrides"] = {}
            e["render_note"] = "Check how the source recorded the amp output (mic or load box) before choosing ir-mix."
        elif src == "tt_markv":
            e["setting"] = {"gain": 5}
            di = "DRY/trainval/train.input.wav"
            starts = active_windows(src, di, a.cache)
            e["parts"] = [{"ref_member": "MesaBoogie-MarkV-ChExtreme/trainval/G050/G050.train.target.wav",
                           "di_member": di, "start_s": starts[(2 * k + 1) % len(starts)], "length_s": EXCERPT_S}]
            e["render_overrides"] = {}
            e["render_note"] = "Check how the source recorded the amp output (mic or load box) before choosing ir-mix."
        entries.append(e)
    manifest = {"schema_version": 1, "gate": "G-010", "sample_rate": FS, "format": "FLAC PCM-24 mono",
                "excerpt_seconds_default": EXCERPT_S, "sources": SOURCES, "presets": entries}
    if MANIFEST.exists():  # keep checksums of excerpts that did not change
        old = {e["stem"]: e for e in json.loads(MANIFEST.read_text())["presets"]}
        for e in entries:
            o = old.get(e["stem"])
            if o and o.get("parts") == e.get("parts") and "files" in o:
                e["files"] = o["files"]
    MANIFEST.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")
    print(f"wrote {MANIFEST.relative_to(ROOT)}")


# --------------------------------------------------------------------------------------------- build / fetch
def write_pair(e, out_dir, cache):
    ref, di = cut_segment(e, cache)
    peak = max(np.abs(ref).max(), np.abs(di).max())
    if peak >= 1.0:
        raise SystemExit(f"{e['stem']}: excerpt clips ({peak:.3f})")
    d = pathlib.Path(out_dir) / e["stem"]
    d.mkdir(parents=True, exist_ok=True)
    files = {}
    for kind, x in (("ref", ref), ("di", di)):
        path = d / f"{kind}.flac"
        sf.write(path, x, FS, subtype="PCM_24", format="FLAC")
        y, _ = sf.read(path)
        files[kind] = {"path": str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else path.name,
                       "pcm_sha256": pcm_sha256(y), "seconds": round(len(y) / FS, 3)}
    return files


def cmd_build(a):
    m = json.loads(MANIFEST.read_text())
    bad = 0
    for e in m["presets"]:
        if e.get("tier") != "vendored":
            continue
        if a.verify:
            for kind, f in e["files"].items():
                y, fs = sf.read(ROOT / f["path"])
                ok = fs == FS and pcm_sha256(y) == f["pcm_sha256"]
                bad += not ok
                print(("ok  " if ok else "BAD ") + f["path"])
            continue
        e["files"] = write_pair(e, AUDIO_DIR, a.cache)
        print("built", e["stem"])
    if not a.verify:
        MANIFEST.write_text(json.dumps(m, indent=2, ensure_ascii=False) + "\n")
    return 1 if bad else 0


def cmd_fetch(a):
    m = json.loads(MANIFEST.read_text())
    out = pathlib.Path(a.local_dir).resolve()
    if out.is_relative_to(ROOT) and not str(out.relative_to(ROOT)).startswith("listening-work"):
        raise SystemExit("local-only references must live outside the repository or under listening-work/")
    for e in m["presets"]:
        if e.get("tier") == "local-only":
            files = write_pair(e, out, a.cache)
            print("fetched", e["stem"], files["ref"]["pcm_sha256"][:12])
    return 0


def cmd_export(a):
    m = json.loads(MANIFEST.read_text())
    out = pathlib.Path(a.out)
    (out / "ref").mkdir(parents=True, exist_ok=True)
    (out / "di").mkdir(parents=True, exist_ok=True)
    n = 0
    for e in m["presets"]:
        if e.get("tier") == "vendored":
            d = AUDIO_DIR / e["stem"]
        elif e.get("tier") == "local-only" and a.local_dir and (pathlib.Path(a.local_dir) / e["stem"]).exists():
            d = pathlib.Path(a.local_dir) / e["stem"]
        else:
            continue
        for kind in ("ref", "di"):
            x, fs = sf.read(d / f"{kind}.flac")
            sf.write(out / kind / f"{e['stem']}.wav", x, fs, subtype="PCM_24")
        n += 1
    print(f"exported {n} reference/DI pairs to {out}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--cache", help="directory for downloaded archive members (speeds up repeat runs)")
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("author")
    b = sub.add_parser("build")
    b.add_argument("--verify", action="store_true")
    f = sub.add_parser("fetch")
    f.add_argument("--local-dir", required=True)
    x = sub.add_parser("export")
    x.add_argument("--out", required=True)
    x.add_argument("--local-dir")
    a = ap.parse_args(argv)
    return {"author": cmd_author, "build": cmd_build, "fetch": cmd_fetch, "export": cmd_export}[a.cmd](a) or 0


if __name__ == "__main__":
    sys.exit(main())
