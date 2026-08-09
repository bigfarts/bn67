#!/usr/bin/env python3
"""Build both complete BN6 chip-name and chip-description archives."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import json
import struct
from pathlib import Path


# The leading BN6 English charset entries cover the chip text used here.
# Bracketed tokens are single glyphs in the ROM (for example EX and SP).
EXE6_EN_CHARSET = (
    [" "]
    + list("0123456789")
    + list("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
    + ["*"]
    + list("abcdefghijklmnopqrstuvwxyz")
    + ["[RV]", "[BX]", "[EX]", "[SP]", "[FZ]"]
)
CHAR_TO_BYTE = {character: index for index, character in enumerate(EXE6_EN_CHARSET)}
# BN6's English ampersand glyph is at charset index 0xA3. The compact
# leading-character table above intentionally omits the intervening Japanese
# glyphs, so map the punctuation used by imported descriptions explicitly.
CHAR_TO_BYTE["!"] = 0xA2
CHAR_TO_BYTE["&"] = 0xA3

NAME_END = 0xE6
LINE_BREAK = 0xE9
DESCRIPTION_HEADER = bytes((0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00))
DESCRIPTION_FOOTER = bytes((0xE7, 0x01, NAME_END))


@dataclass(frozen=True)
class TextArchiveSpec:
    name: str
    source: str
    source_index: int
    encoding: str


@dataclass(frozen=True)
class PackageText:
    archives: dict[str, TextArchiveSpec]
    changes: dict[str, dict[int, bytes]]

    def changes_for(self, source: str, source_index: int) -> dict[int, bytes]:
        matches = [
            archive.name
            for archive in self.archives.values()
            if archive.source == source and archive.source_index == source_index
        ]
        if len(matches) != 1:
            raise ValueError(
                f"expected one configured text archive for {source}[{source_index}], "
                f"found {len(matches)}"
            )
        return self.changes[matches[0]]


def encode_text(text: str) -> bytes:
    encoded = bytearray()
    index = 0
    tokens = sorted(CHAR_TO_BYTE, key=len, reverse=True)
    while index < len(text):
        token = next((candidate for candidate in tokens if text.startswith(candidate, index)), None)
        if token is None:
            raise ValueError(f"character at {text[index:]!r} is not in the BN6 English charset")
        encoded.append(CHAR_TO_BYTE[token])
        index += len(token)
    return bytes(encoded)


def encode_name(text: str) -> bytes:
    return encode_text(text) + bytes((NAME_END,))


def encode_description(*lines: str) -> bytes:
    body = bytes((LINE_BREAK,)).join(encode_text(line) for line in lines)
    return DESCRIPTION_HEADER + body + DESCRIPTION_FOOTER


def load_package_text(path: Path) -> PackageText:
    try:
        raw = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(f"cannot read generated package text {path}: {exc}") from exc
    if (
        not isinstance(raw, dict)
        or set(raw) != {"archives", "entries"}
        or not isinstance(raw["archives"], list)
        or not isinstance(raw["entries"], list)
    ):
        raise ValueError(f"{path}: expected an object containing archives and entries arrays")

    archives: dict[str, TextArchiveSpec] = {}
    sources: set[tuple[str, int]] = set()
    for index, archive in enumerate(raw["archives"]):
        context = f"{path}: archives[{index}]"
        if not isinstance(archive, dict) or set(archive) != {
            "name", "source", "source_index", "encoding"
        }:
            raise ValueError(f"{context}: invalid generated archive definition")
        name = archive["name"]
        source = archive["source"]
        source_index = archive["source_index"]
        encoding = archive["encoding"]
        if not isinstance(name, str) or not name:
            raise ValueError(f"{context}: name must be a non-empty string")
        if source not in {"names", "descriptions"}:
            raise ValueError(f"{context}: unknown source {source!r}")
        if not isinstance(source_index, int) or source_index < 0:
            raise ValueError(f"{context}: source_index must be a non-negative integer")
        if encoding not in {"name", "description"}:
            raise ValueError(f"{context}: unknown encoding {encoding!r}")
        if name in archives:
            raise ValueError(f"{context}: duplicate text archive name {name!r}")
        source_key = (source, source_index)
        if source_key in sources:
            raise ValueError(f"{context}: duplicate text archive source {source}[{source_index}]")
        sources.add(source_key)
        archives[name] = TextArchiveSpec(name, source, source_index, encoding)

    changes: dict[str, dict[int, bytes]] = {name: {} for name in archives}
    for index, entry in enumerate(raw["entries"]):
        context = f"{path}: entries[{index}]"
        if not isinstance(entry, dict) or set(entry) != {
            "package", "archive", "index", "value"
        }:
            raise ValueError(f"{context}: invalid generated text entry")
        package = entry["package"]
        archive_name = entry["archive"]
        entry_index = entry["index"]
        value = entry["value"]
        if not isinstance(package, str) or not package:
            raise ValueError(f"{context}: package must be a non-empty string")
        if not isinstance(archive_name, str) or archive_name not in archives:
            raise ValueError(f"{context}: unknown text archive {archive_name!r}")
        if not isinstance(entry_index, int) or entry_index < 0:
            raise ValueError(f"{context}: index must be a non-negative integer")
        archive = archives[archive_name]
        try:
            if archive.encoding == "name":
                if not isinstance(value, str):
                    raise ValueError("value must be a string")
                encoded = encode_name(value)
            else:
                if not isinstance(value, list) or not all(isinstance(line, str) for line in value):
                    raise ValueError("value must be a string array")
                encoded = encode_description(*value)
        except ValueError as exc:
            raise ValueError(f"{context} ({package}): {exc}") from exc
        if entry_index in changes[archive_name]:
            raise ValueError(
                f"{context}: duplicate text target {archive_name}[0x{entry_index:X}]"
            )
        changes[archive_name][entry_index] = encoded
    return PackageText(archives, changes)


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def read_archive(rom: bytes, offset: int) -> list[bytes]:
    table_size = u16(rom, offset)
    if table_size == 0 or table_size & 1:
        raise ValueError(f"invalid archive table size at 0x{offset:X}")
    entry_count = table_size // 2
    offsets = struct.unpack_from(f"<{entry_count}H", rom, offset)
    entries: list[bytes] = []
    for index, relative in enumerate(offsets):
        start = offset + relative
        if index + 1 < entry_count:
            finish = offset + offsets[index + 1]
        else:
            terminator = rom.find(b"\xE6", start)
            if terminator < 0:
                raise ValueError(f"unterminated final archive entry at 0x{start:X}")
            finish = terminator + 1
        if finish < start:
            raise ValueError(f"descending archive offsets at entry {index}")
        entries.append(rom[start:finish])
    return entries


def build_archive(entries: list[bytes]) -> bytes:
    cursor = len(entries) * 2
    offsets: list[int] = []
    for entry in entries:
        offsets.append(cursor)
        cursor += len(entry)
        if cursor > 0xFFFF:
            raise ValueError("archive exceeds 16-bit offsets")
    return struct.pack(f"<{len(offsets)}H", *offsets) + b"".join(entries)


def apply_changes(
    archives: dict[str, list[bytes]],
    changes: dict[str, dict[int, bytes]],
) -> None:
    for archive_name, entries in changes.items():
        if archive_name not in archives:
            raise ValueError(f"unknown text archive {archive_name!r}")
        archive = archives[archive_name]
        for entry_index, entry in entries.items():
            if not 0 <= entry_index < len(archive):
                raise ValueError(
                    f"index 0x{entry_index:X} is outside text archive {archive_name!r}"
                )
            archive[entry_index] = entry


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("name_pointer_table", type=lambda value: int(value, 0))
    parser.add_argument("description_pointer_table", type=lambda value: int(value, 0))
    parser.add_argument("name_output_0", type=Path)
    parser.add_argument("name_output_1", type=Path)
    parser.add_argument("description_output_0", type=Path)
    parser.add_argument("description_output_1", type=Path)
    parser.add_argument("--package-text", type=Path)
    args = parser.parse_args()

    rom = args.rom.read_bytes()
    name_offsets = [
        u32(rom, args.name_pointer_table + index * 4) - 0x08000000
        for index in range(2)
    ]
    description_offsets = [
        u32(rom, args.description_pointer_table + index * 4) - 0x08000000
        for index in range(2)
    ]
    for archive_offset in name_offsets + description_offsets:
        if not 0 <= archive_offset < len(rom):
            raise ValueError(f"text archive pointer is outside the ROM: 0x{archive_offset:X}")

    name_archives = [read_archive(rom, offset) for offset in name_offsets]
    description_archives = [read_archive(rom, offset) for offset in description_offsets]

    if args.package_text is not None:
        package_text = load_package_text(args.package_text)
        source_archives = {
            "names": name_archives,
            "descriptions": description_archives,
        }
        archives: dict[str, list[bytes]] = {}
        for archive in package_text.archives.values():
            source = source_archives[archive.source]
            if archive.source_index >= len(source):
                raise ValueError(
                    f"text archive {archive.name!r} selects missing "
                    f"{archive.source}[{archive.source_index}]"
                )
            archives[archive.name] = source[archive.source_index]
        apply_changes(archives, package_text.changes)

    outputs = [
        (args.name_output_0, name_archives[0]),
        (args.name_output_1, name_archives[1]),
        (args.description_output_0, description_archives[0]),
        (args.description_output_1, description_archives[1]),
    ]
    for output, entries in outputs:
        output.write_bytes(build_archive(entries))


if __name__ == "__main__":
    main()
