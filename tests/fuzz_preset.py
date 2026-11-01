#!/usr/bin/env python3
"""
fuzz_preset.py — T-013 corpus generator (FROZEN behavior, seed 42).

Generates `count` mutated preset XML files from a seed preset using bit flips,
truncations, huge numbers and structural junk. Deterministic for a given seed.
"""
import argparse
import os
import random
import string


def mutate(text: str, rng: random.Random) -> str:
    mode = rng.randrange(6)
    data = list(text)

    if mode == 0 and len(data) > 8:                      # bit flips
        for _ in range(rng.randrange(1, 12)):
            i = rng.randrange(len(data))
            data[i] = chr(ord(data[i]) ^ (1 << rng.randrange(7)))
    elif mode == 1 and len(data) > 4:                    # truncation
        data = data[: rng.randrange(1, len(data))]
    elif mode == 2:                                      # huge numbers
        for _ in range(rng.randrange(1, 6)):
            i = rng.randrange(len(data))
            data.insert(i, rng.choice(["9" * 40, "1e309", "-1e309", "2147483648"]))
    elif mode == 3:                                      # junk insertion
        for _ in range(rng.randrange(1, 8)):
            i = rng.randrange(len(data) + 1)
            junk = "".join(rng.choice(string.printable) for _ in range(rng.randrange(1, 20)))
            data.insert(i, junk)
    elif mode == 4:                                      # tag vandalism
        for _ in range(rng.randrange(1, 5)):
            i = rng.randrange(len(data))
            data.insert(i, rng.choice(["</", "<", ">", "&", '"', "'", "]]>", "<!--"]))
    else:                                                # deletion burst
        for _ in range(rng.randrange(1, 10)):
            if data:
                del data[rng.randrange(len(data))]

    return "".join(data)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--count", type=int, default=1000)
    ap.add_argument("--out-dir", required=True)
    ap.add_argument("--seed-preset", required=True)
    args = ap.parse_args()

    rng = random.Random(args.seed)
    with open(args.seed_preset, "r", encoding="utf-8", errors="replace") as fh:
        seed_text = fh.read()

    os.makedirs(args.out_dir, exist_ok=True)
    for i in range(args.count):
        out = os.path.join(args.out_dir, f"fuzz_{i:04d}.xml")
        with open(out, "w", encoding="utf-8", errors="replace") as fh:
            fh.write(mutate(seed_text, rng))

    print(f"generated {args.count} mutated presets in {args.out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
