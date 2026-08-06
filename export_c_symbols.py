#!/usr/bin/env python3
"""Convert globally defined ELF symbols into Armips constants."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess


SYMBOL_RE = re.compile(r"^([0-9A-Fa-f]+)\s+([A-Za-z])\s+([A-Za-z_][A-Za-z0-9_]*)$")
EXPORTED_KINDS = frozenset("TRDB")


def symbols(nm: str, elf: Path) -> list[tuple[int, str]]:
    result = subprocess.run(
        [nm, "-g", "-n", "--defined-only", str(elf)],
        check=True,
        capture_output=True,
        text=True,
    )
    found: list[tuple[int, str]] = []
    for line in result.stdout.splitlines():
        match = SYMBOL_RE.fullmatch(line.strip())
        if (
            match is None
            or match.group(2) not in EXPORTED_KINDS
            or not match.group(3)[0].isupper()
        ):
            continue
        found.append((int(match.group(1), 16), match.group(3)))
    return found


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--nm", default="arm-none-eabi-nm")
    args = parser.parse_args()

    lines = ["// Generated from the linked freestanding C image. Do not edit."]
    lines.extend(f".definelabel {name}, 0x{address:08X}" for address, name in symbols(args.nm, args.elf))
    content = "\n".join(lines) + "\n"
    if not args.output.exists() or args.output.read_text() != content:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
