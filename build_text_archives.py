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
# fmt: off
EXE6_EN_CHARSET = [" ", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "*", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "[RV]", "[BX]", "[EX]", "[SP]", "[FZ]", "ウ", "ア", "イ", "オ", "エ", "ケ", "コ", "カ", "ク", "キ", "セ", "サ", "ソ", "シ", "ス", "テ", "ト", "ツ", "タ", "チ", "ネ", "ノ", "ヌ", "ナ", "ニ", "ヒ", "ヘ", "ホ", "ハ", "フ", "ミ", "マ", "メ", "ム", "モ", "ヤ", "ヨ", "ユ", "ロ", "ル", "リ", "レ", "ラ", "ン", "熱", "斗", "ワ", "ヲ", "ギ", "ガ", "ゲ", "ゴ", "グ", "ゾ", "ジ", "ゼ", "ズ", "ザ", "デ", "ド", "ヅ", "ダ", "ヂ", "ベ", "ビ", "ボ", "バ", "ブ", "ピ", "パ", "ペ", "プ", "ポ", "ゥ", "ァ", "ィ", "ォ", "ェ", "ュ", "ヴ", "ッ", "ョ", "ャ", "-", "×", "=", ":", "%", "?", "+", "█", "[bat]", "ー", "!", "&", ",", "゜", ".", "・", ";", "'", "\"", "~", "/", "(", ")", "「", "」", "�", "_", "ƶ", "[L]", "[B]", "[R]", "[A]", "あ", "い", "け", "く", "き", "こ", "か", "せ", "そ", "す", "さ", "し", "つ", "と", "て", "た", "ち", "ね", "の", "な", "ぬ", "に", "へ", "ふ", "ほ", "は", "ひ", "め", "む", "み", "も", "ま", "ゆ", "よ", "や", "る", "ら", "り", "ろ", "れ", "[END]", "ん", "を", "わ", "研", "げ", "ぐ", "ご", "が", "ぎ", "ぜ", "ず", "じ", "ぞ", "ざ", "で", "ど", "づ", "だ", "ぢ", "べ", "ば", "び", "ぼ", "ぶ", "ぽ", "ぷ", "ぴ", "ぺ", "ぱ", "ぅ", "ぁ", "ぃ", "ぉ", "ぇ", "ゅ", "ょ", "っ", "ゃ", "容", "量", "全", "木", "[MB]", "無", "現", "実", "[circle]", "×", "緑", "道", "不", "止", "彩", "起", "父", "集", "院", "一", "二", "三", "四", "五", "六", "七", "八", "陽", "十", "百", "千", "万", "脳", "上", "下", "左", "右", "手", "来", "日", "目", "月", "獣", "各", "人", "入", "出", "山", "口", "光", "電", "気", "綾", "科", "次", "名", "前", "学", "校", "省", "祐", "室", "世", "界", "高", "朗", "枚", "野", "悪", "路", "闇", "大", "小", "中", "自", "分", "間", "系", "花", "問", "究", "門", "城", "王", "兄", "化", "葉", "行", "街", "屋", "水", "見", "終", "新", "桜", "先", "生", "長", "今", "了", "点", "井", "子", "言", "太", "属", "風", "会", "性", "持", "時", "勝", "赤", "代", "年", "火", "改", "計", "画", "職", "体", "波", "回", "外", "地", "員", "正", "造", "値", "合", "戦", "川", "秋", "原", "町", "晴", "用", "金", "郎", "作", "数", "方", "社", "攻", "撃", "力", "同", "武", "何", "発", "少", "教", "以", "白", "早", "暮", "面", "組", "後", "文", "字", "本", "階", "明", "才", "者", "向", "犬", "々", "ヶ", "連", "射", "舟", "戸", "切", "土", "炎", "伊", "夫", "鉄", "国", "男", "天", "老", "師", "堀", "杉", "士", "悟", "森", "霧", "麻", "剛", "垣", "★", "[bracket1]", "[bracket2]", "[.]"]
# fmt: on

RECORD_END = 0xE6
LINE_BREAK = 0xE9

CHAR_TO_BYTE = {character: index for index, character in enumerate(EXE6_EN_CHARSET)}
CHAR_TO_BYTE["\n"] = LINE_BREAK

DESCRIPTION_HEADER = bytes((0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00))
DESCRIPTION_FOOTER = bytes((0xE7, 0x01))


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
        token = next(
            (candidate for candidate in tokens if text.startswith(candidate, index)),
            None,
        )
        if token is None:
            raise ValueError(
                f"character at {text[index:]!r} is not in the BN6 English charset"
            )
        encoded.append(CHAR_TO_BYTE[token])
        index += len(token)
    return bytes(encoded)


def encode_name(text: str) -> bytes:
    return encode_text(text)


def encode_description(text: str) -> bytes:
    return DESCRIPTION_HEADER + encode_text(text) + DESCRIPTION_FOOTER


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
        raise ValueError(
            f"{path}: expected an object containing archives and entries arrays"
        )

    archives: dict[str, TextArchiveSpec] = {}
    sources: set[tuple[str, int]] = set()
    for index, archive in enumerate(raw["archives"]):
        context = f"{path}: archives[{index}]"
        if not isinstance(archive, dict) or set(archive) != {
            "name",
            "source",
            "source_index",
            "encoding",
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
            raise ValueError(
                f"{context}: duplicate text archive source {source}[{source_index}]"
            )
        sources.add(source_key)
        archives[name] = TextArchiveSpec(name, source, source_index, encoding)

    changes: dict[str, dict[int, bytes]] = {name: {} for name in archives}
    for index, entry in enumerate(raw["entries"]):
        context = f"{path}: entries[{index}]"
        if not isinstance(entry, dict) or set(entry) != {
            "package",
            "archive",
            "index",
            "value",
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
                if not isinstance(value, str):
                    raise ValueError("value must be a string")
                encoded = encode_description(value)
        except ValueError as exc:
            raise ValueError(f"{context} ({package}): {exc}") from exc
        encoded += bytes((RECORD_END,))
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
            terminator = rom.find(bytes((RECORD_END,)), start)
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
        u32(rom, args.name_pointer_table + index * 4) - 0x08000000 for index in range(2)
    ]
    description_offsets = [
        u32(rom, args.description_pointer_table + index * 4) - 0x08000000
        for index in range(2)
    ]
    for archive_offset in name_offsets + description_offsets:
        if not 0 <= archive_offset < len(rom):
            raise ValueError(
                f"text archive pointer is outside the ROM: 0x{archive_offset:X}"
            )

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
