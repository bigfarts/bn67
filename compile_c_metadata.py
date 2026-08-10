#!/usr/bin/env python3
"""Compile package C sources and export their ordered metadata ELF symbols."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess
import tempfile

SYMBOL_RE = re.compile(r"^[0-9A-Fa-f]+\s+[A-Za-z]\s+(__exe6_meta__[A-Za-z0-9_]+)$")
RUNTIME_SOURCE_NAMES = {"abi.c", "runtime.c"}


def package_symbols(
    cc: str,
    nm: str,
    defines: tuple[str, ...],
    include_dir: Path,
    source: Path,
    object_path: Path,
) -> list[str]:
    subprocess.run(
        [
            cc,
            "-std=c11",
            "-mthumb",
            "-march=armv4t",
            "-Os",
            "-ffreestanding",
            "-fno-builtin",
            "-ffixed-r5",
            "-DEXE6_METADATA_ONLY",
            *(f"-D{define}" for define in defines),
            f"-I{include_dir}",
            "-c",
            str(source),
            "-o",
            str(object_path),
        ],
        check=True,
    )
    result = subprocess.run(
        [nm, "-g", "-n", "--defined-only", str(object_path)],
        check=True,
        capture_output=True,
        text=True,
    )
    symbols: list[str] = []
    for line in result.stdout.splitlines():
        match = SYMBOL_RE.fullmatch(line.strip())
        if match is not None:
            symbols.append(match.group(1))
    return symbols


def write_if_changed(path: Path, content: str) -> None:
    if path.exists() and path.read_text() == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument("--cc", default="arm-none-eabi-gcc")
    parser.add_argument("--nm", default="arm-none-eabi-nm")
    parser.add_argument("--define", action="append", default=[])
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    root = args.root.resolve()
    output = (
        args.output.resolve()
        if args.output
        else root / "build/registry-metadata.generated.json"
    )
    include_dir = root / "src"
    packages: list[dict[str, object]] = []
    with tempfile.TemporaryDirectory(prefix="bn6-registry-metadata-") as directory:
        temporary = Path(directory)
        for source in sorted(root.glob("src/**/*.c")):
            if source.name in RUNTIME_SOURCE_NAMES:
                continue
            package = source.stem
            symbols = package_symbols(
                args.cc,
                args.nm,
                tuple(args.define),
                include_dir,
                source,
                temporary / f"{package}.o",
            )
            packages.append({"name": package, "symbols": symbols})

    content = json.dumps({"packages": packages}, indent=2) + "\n"
    write_if_changed(output, content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
