#!/usr/bin/env python3
"""
gen_presets.py — author the 24 factory presets (S-011, A-006).

Each preset is an APVTS state snapshot (D-008): <Parameters> root with PARAM
children, plus a `name` attribute. Only frozen parameter IDs are referenced.
"""
import os

OUT = os.path.join(os.path.dirname(__file__), "..", "Resources", "presets")

PARAMS = [
    "input-gain", "gate-on", "gate-threshold", "boost-on", "boost-amount",
    "drive", "bass", "mid", "treble", "presence", "depth", "sag",
    "amp-voicing", "cab-ir", "oversampling", "mic-position", "ir-mix",
    "post-eq-on", "post-eq-low", "post-eq-high", "master",
]

VOICING = {"Tweed": 0, "Plexi": 1, "JCM800": 2, "Recto": 3, "5150": 4,
           "Chainsaw": 5, "Frost": 6}

# name, genre, overrides over a sane base
BASE = {
    "input-gain": 0.0, "gate-on": 1, "gate-threshold": -60.0,
    "boost-on": 0, "boost-amount": 0.5, "drive": 0.5,
    "bass": 0.5, "mid": 0.5, "treble": 0.5, "presence": 0.5, "depth": 0.5,
    "sag": 0.5, "amp-voicing": 1, "cab-ir": 0, "oversampling": 1,
    "mic-position": 0.25, "ir-mix": 1.0, "post-eq-on": 0,
    "post-eq-low": 0.0, "post-eq-high": 0.0, "master": 0.7,
}

PRESETS = [
    ("70s Blues Rock",      "blues-rock",  {"amp-voicing": VOICING["Tweed"],  "cab-ir": 3, "drive": 0.45, "bass": 0.55, "mid": 0.6,  "treble": 0.5,  "sag": 0.6}),
    ("Southern Rock",       "southern-rock",{"amp-voicing": VOICING["Plexi"],  "cab-ir": 4, "drive": 0.5,  "bass": 0.55, "mid": 0.6,  "treble": 0.5,  "sag": 0.5}),
    ("Classic Rock Crunch", "classic-rock", {"amp-voicing": VOICING["Plexi"],  "cab-ir": 0, "drive": 0.55, "bass": 0.5,  "mid": 0.55, "treble": 0.55, "sag": 0.4}),
    ("80s Hair Metal",      "hair-metal",  {"amp-voicing": VOICING["JCM800"], "cab-ir": 0, "drive": 0.6,  "bass": 0.5,  "mid": 0.55, "treble": 0.65, "presence": 0.6, "boost-on": 1, "boost-amount": 0.4}),
    ("Thrash Attack",       "thrash",      {"amp-voicing": VOICING["JCM800"], "cab-ir": 0, "drive": 0.7,  "bass": 0.45, "mid": 0.6,  "treble": 0.7,  "gate-threshold": -55.0, "boost-on": 1}),
    ("Bay Area Lead",       "thrash-lead", {"amp-voicing": VOICING["JCM800"], "cab-ir": 6, "drive": 0.65, "bass": 0.45, "mid": 0.65, "treble": 0.7,  "presence": 0.65, "boost-on": 1, "boost-amount": 0.6}),
    ("90s Groove",          "groove",      {"amp-voicing": VOICING["Recto"],  "cab-ir": 1, "drive": 0.6,  "bass": 0.6,  "mid": 0.5,  "treble": 0.5,  "depth": 0.6}),
    ("Nu-Metal Drop",       "nu-metal",    {"amp-voicing": VOICING["Recto"],  "cab-ir": 1, "drive": 0.65, "bass": 0.65, "mid": 0.45, "treble": 0.45, "depth": 0.65, "boost-on": 1, "boost-amount": 0.5, "post-eq-on": 1, "post-eq-low": 3.0}),
    ("Death Metal Chains",  "death-metal", {"amp-voicing": VOICING["5150"],   "cab-ir": 1, "drive": 0.75, "bass": 0.55, "mid": 0.45, "treble": 0.6,  "gate-threshold": -50.0, "boost-on": 1}),
    ("Florida Trem",        "death-metal", {"amp-voicing": VOICING["5150"],   "cab-ir": 6, "drive": 0.7,  "bass": 0.5,  "mid": 0.55, "treble": 0.65, "boost-on": 1, "boost-amount": 0.65}),
    ("Swedish Chainsaw",    "swedish-death",{"amp-voicing": VOICING["Chainsaw"], "cab-ir": 6, "drive": 0.7, "bass": 0.5, "mid": 0.35, "treble": 0.6, "gate-threshold": -52.0, "boost-on": 1, "boost-amount": 0.55}),
    ("Stockholm Buzz",      "swedish-death",{"amp-voicing": VOICING["Chainsaw"], "cab-ir": 2, "drive": 0.8, "bass": 0.45, "mid": 0.3, "treble": 0.65, "presence": 0.55, "gate-threshold": -48.0}),
    ("Black Metal Frost",   "black-metal", {"amp-voicing": VOICING["Frost"],  "cab-ir": 7, "drive": 0.8,  "bass": 0.4,  "mid": 0.35, "treble": 0.65, "gate-threshold": -45.0, "boost-on": 1, "boost-amount": 0.7}),
    ("Cavernous Hiss",      "black-metal", {"amp-voicing": VOICING["Frost"],  "cab-ir": 2, "drive": 0.75, "bass": 0.45, "mid": 0.4,  "treble": 0.55, "mic-position": 0.7, "gate-threshold": -50.0}),
    ("Doom Sludge",         "doom-sludge", {"amp-voicing": VOICING["Recto"],  "cab-ir": 7, "drive": 0.7,  "bass": 0.75, "mid": 0.5,  "treble": 0.4,  "depth": 0.8, "sag": 0.8}),
    ("Funeral March",       "doom",        {"amp-voicing": VOICING["5150"],   "cab-ir": 7, "drive": 0.65, "bass": 0.8,  "mid": 0.45, "treble": 0.35, "depth": 0.85, "sag": 0.85, "ir-mix": 0.9}),
    ("Djent Machine",       "djent",       {"amp-voicing": VOICING["5150"],   "cab-ir": 5, "drive": 0.7,  "bass": 0.5,  "mid": 0.5,  "treble": 0.6,  "gate-threshold": -55.0, "post-eq-on": 1, "post-eq-high": -3.0}),
    ("Modern Prog Lead",    "prog",        {"amp-voicing": VOICING["5150"],   "cab-ir": 5, "drive": 0.6,  "bass": 0.45, "mid": 0.65, "treble": 0.65, "presence": 0.7, "boost-on": 1, "boost-amount": 0.5}),
    ("Djent Clean",         "djent-clean", {"amp-voicing": VOICING["Tweed"],  "cab-ir": 5, "drive": 0.2,  "bass": 0.55, "mid": 0.6,  "treble": 0.6,  "gate-on": 0}),
    ("Ambient Washtub",     "ambient",     {"amp-voicing": VOICING["Plexi"],  "cab-ir": 2, "drive": 0.3,  "bass": 0.5,  "mid": 0.55, "treble": 0.5,  "sag": 0.7, "ir-mix": 0.8}),
    ("Crunch Rhythm",       "utility",     {"amp-voicing": VOICING["JCM800"], "cab-ir": 0, "drive": 0.45, "bass": 0.5,  "mid": 0.55, "treble": 0.55, "gate-on": 0}),
    ("Edge of Breakup",     "utility",     {"amp-voicing": VOICING["Tweed"],  "cab-ir": 4, "drive": 0.35, "bass": 0.5,  "mid": 0.6,  "treble": 0.55, "gate-on": 0, "sag": 0.6}),
    ("Solo Scream",         "lead",        {"amp-voicing": VOICING["JCM800"], "cab-ir": 6, "drive": 0.75, "bass": 0.4,  "mid": 0.7,  "treble": 0.7,  "presence": 0.75, "boost-on": 1, "boost-amount": 0.7, "gate-threshold": -58.0}),
    ("Midnight Frost Lead", "lead",        {"amp-voicing": VOICING["Frost"],  "cab-ir": 5, "drive": 0.85, "bass": 0.35, "mid": 0.5,  "treble": 0.7,  "presence": 0.7, "boost-on": 1, "boost-amount": 0.8}),
]


def render_preset(name: str, genre: str, overrides: dict) -> str:
    values = dict(BASE)
    values.update(overrides)
    lines = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<Parameters name="{name}" genre="{genre}" grimampPresetVersion="1">',
    ]
    for p in PARAMS:
        lines.append(f'  <PARAM id="{p}" value="{values[p]}"/>')
    lines.append("</Parameters>")
    return "\n".join(lines) + "\n"


def main():
    assert len(PRESETS) == 24, f"A-006 requires 24 presets, have {len(PRESETS)}"
    os.makedirs(OUT, exist_ok=True)
    for i, (name, genre, overrides) in enumerate(PRESETS, start=1):
        path = os.path.join(OUT, f"preset{i:02d}.xml")
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(render_preset(name, genre, overrides))
        print(f"wrote {path} ({name})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
