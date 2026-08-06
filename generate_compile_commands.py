#!/usr/bin/env python3
"""Generate a dual-edition compilation database for the BN6 C sources."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shlex


def write_if_changed(path: Path, content: str) -> None:
    if path.exists() and path.read_text() == content:
        return
    path.write_text(content)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", required=True)
    parser.add_argument("--cflags", required=True)
    parser.add_argument("--directory", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("sources", type=Path, nargs="+")
    args = parser.parse_args()

    directory = args.directory.resolve()
    common = [args.cc, *shlex.split(args.cflags)]
    entries: list[dict[str, object]] = []
    for edition, falzar in (("gregar", 0), ("falzar", 1)):
        for source_path in args.sources:
            source = source_path.resolve()
            object_path = (
                directory
                / "build"
                / "compile-commands"
                / edition
                / f"{source.stem}.o"
            )
            entries.append(
                {
                    "directory": str(directory),
                    "file": str(source),
                    "output": str(object_path),
                    "arguments": [
                        *common,
                        f"-DFALZAR={falzar}",
                        "-c",
                        str(source),
                        "-o",
                        str(object_path),
                    ],
                }
            )

    write_if_changed(args.output.resolve(), json.dumps(entries, indent=2) + "\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
