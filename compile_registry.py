#!/usr/bin/env python3
"""Compile BN6 C metadata and text replacements into registry glue."""

from __future__ import annotations

import argparse
from dataclasses import dataclass, replace
import json
from pathlib import Path
import re
import sys
import tomllib
from typing import Any

NAME_RE = re.compile(r"^[a-z][a-z0-9]*(?:[-_][a-z0-9]+)*$")
SNAKE_CASE_RE = re.compile(r"^[a-z][a-z0-9]*(?:_[a-z0-9]+)*$")
POINTER_METADATA_RE = re.compile(r"^pointer__(0[xX][0-9A-Fa-f]+)__([a-z][a-z0-9_]*)$")
THUMB_POINTER_METADATA_RE = re.compile(
    r"^thumb_pointer__(0[xX][0-9A-Fa-f]+)__([a-z][a-z0-9_]*)$"
)
SECTION_METADATA_RE = re.compile(
    r"^section__(0[xX][0-9A-Fa-f]+)__(0[xX][0-9A-Fa-f]+)__([a-z][a-z0-9_]*)$"
)
LINKED_CALL_METADATA_RE = re.compile(
    r"^linked_call__([a-z][a-z0-9_]*)__(0[xX][0-9A-Fa-f]+)__"
    r"([a-z][a-z0-9_]*)$"
)
RUNTIME_SOURCE_NAMES = {"abi.c", "runtime.c"}
SONG_PLAYER_FIRST = 0x0C
SONG_PLAYER_LAST = 0x1F

class PackageError(Exception):
    pass


@dataclass(frozen=True)
class ObjectClass:
    number: int
    native_entries: int
    native_table: str
    references: tuple[int, ...]


@dataclass(frozen=True)
class SpriteGroup:
    number: int
    references: tuple[int, ...]
    native_table: str
    native_entries: int


@dataclass(frozen=True)
class SongConfig:
    native_table: str
    native_entries: int
    references: tuple[int, ...]


@dataclass(frozen=True)
class DustSpriteReclaim:
    kind: int
    alias: int
    references: tuple[int, ...]


@dataclass(frozen=True)
class DustSpriteConfig:
    native_table: str
    native_entries: int
    references: tuple[int, ...]
    max_kind: int
    reclaimed: tuple[DustSpriteReclaim, ...]


@dataclass(frozen=True)
class FieldObjectConfig:
    native_table: str
    native_entries: int
    references: tuple[int, ...]
    base_id: int
    max_id: int


@dataclass(frozen=True)
class TextArchive:
    name: str
    region: int
    source: str
    source_index: int
    encoding: str
    native_entries: int
    symbol: str
    binary: str
    references: tuple[int, ...]


@dataclass(frozen=True)
class TextArchiveGroup:
    archives: tuple[TextArchive, ...]


@dataclass(frozen=True)
class TextConfig:
    folder_edit_skip: tuple[int, int]
    groups: tuple[TextArchiveGroup, ...]


@dataclass(frozen=True)
class ChipConfig:
    table_address: int
    record_size: int
    record_count: int


@dataclass(frozen=True)
class AttackPool:
    family: int
    native_entries: int
    native_table: str
    references: tuple[int, ...]


@dataclass(frozen=True)
class Config:
    root: Path
    variant: str
    object_classes: dict[int, ObjectClass]
    sprite_groups: dict[int, SpriteGroup]
    dust_sprites: DustSpriteConfig
    field_objects: FieldObjectConfig
    songs: SongConfig
    text: TextConfig
    chips: ChipConfig
    attack_pools: dict[str, AttackPool]


@dataclass(frozen=True)
class ObjectResource:
    object_class: int
    main: str


@dataclass(frozen=True)
class FixedObjectResource:
    object_class: int
    object_id: int
    main: str


@dataclass(frozen=True)
class SpriteResource:
    archive: str


@dataclass(frozen=True)
class FixedSpriteResource:
    group: int
    index: int
    archive: str
    compressed: bool = False


@dataclass(frozen=True)
class DustSpriteResource:
    archive: str


@dataclass(frozen=True)
class FieldObjectResource:
    archive: str
    animation: int
    palette: int
    shadow: int


@dataclass(frozen=True)
class SongResource:
    archive: str


@dataclass(frozen=True)
class TextResource:
    package: str
    archive: str
    index: int
    value: str


@dataclass(frozen=True)
class ChipResource:
    package: str
    chip_id: int
    record: str


@dataclass(frozen=True)
class AttackResource:
    package: str
    chip_id: int
    kind: str
    main: str


@dataclass(frozen=True)
class FixedAttackResource:
    family: int
    subfamily: int
    main: str


@dataclass(frozen=True)
class AttackAllocation:
    family: int
    subfamily: int


@dataclass(frozen=True)
class PointerPatch:
    symbol: str
    address: int
    thumb: bool = False


@dataclass(frozen=True)
class SectionPatch:
    symbol: str
    address: int
    relay_address: int


@dataclass(frozen=True)
class LinkedCallPatch:
    source: str
    offset: int
    target: str


@dataclass(frozen=True)
class Package:
    name: str
    source: str
    definitions: Path
    objects: tuple[ObjectResource, ...]
    fixed_objects: tuple[FixedObjectResource, ...]
    sprites: tuple[SpriteResource, ...]
    fixed_sprites: tuple[FixedSpriteResource, ...]
    dust_sprites: tuple[DustSpriteResource, ...]
    field_objects: tuple[FieldObjectResource, ...]
    songs: tuple[SongResource, ...]
    text: tuple[TextResource, ...]
    chips: tuple[ChipResource, ...]
    attack: AttackResource | None
    fixed_attacks: tuple[FixedAttackResource, ...]
    pointer_patches: tuple[PointerPatch, ...]
    section_patches: tuple[SectionPatch, ...]
    linked_call_patches: tuple[LinkedCallPatch, ...]


@dataclass(frozen=True)
class Allocations:
    objects: dict[int, dict[str, int]]
    sprites: dict[str, tuple[int, int]]
    dust_sprites: dict[str, int]
    field_objects: dict[str, int]
    songs: dict[str, int]
    song_players: dict[str, int]
    attacks: dict[str, AttackAllocation]


def require_int(table: dict[str, Any], key: str, context: str) -> int:
    value = table.get(key)
    if not isinstance(value, int) or isinstance(value, bool):
        raise PackageError(f"{context}: {key!r} must be an integer")
    return value


def require_str(table: dict[str, Any], key: str, context: str) -> str:
    value = table.get(key)
    if not isinstance(value, str) or not value:
        raise PackageError(f"{context}: {key!r} must be a non-empty string")
    return value


def check_keys(table: dict[str, Any], allowed: set[str], context: str) -> None:
    unknown = sorted(set(table) - allowed)
    if unknown:
        raise PackageError(f"{context}: unknown field(s): {', '.join(unknown)}")


def check_name(name: str, context: str) -> None:
    if not NAME_RE.fullmatch(name):
        raise PackageError(f"{context}: {name!r} must match {NAME_RE.pattern}")


def require_int_array(table: dict[str, Any], key: str, context: str) -> tuple[int, ...]:
    value = table.get(key)
    if not isinstance(value, list) or not all(
        isinstance(item, int) and not isinstance(item, bool) for item in value
    ):
        raise PackageError(f"{context}: {key!r} must be an integer array")
    return tuple(value)


def require_table(raw: dict[str, Any], key: str, context: str) -> dict[str, Any]:
    value = raw.get(key)
    if not isinstance(value, dict):
        raise PackageError(f"{context}: {key!r} must be a table")
    return value


def require_table_array(
    raw: dict[str, Any], key: str, context: str
) -> list[dict[str, Any]]:
    value = raw.get(key)
    if not isinstance(value, list) or not all(isinstance(item, dict) for item in value):
        raise PackageError(f"{context}: {key!r} must be an array of tables")
    return value


def fixed_width_entry_count(
    root: Path,
    binary: str,
    entry_size: int,
    maximum: int,
    context: str,
) -> int:
    """Derive a native table's entry count from its extracted binary."""
    path = root / binary
    try:
        size = path.stat().st_size
    except OSError as exc:
        raise PackageError(f"{context}: cannot read {path}: {exc}") from exc
    if size == 0 or size % entry_size:
        raise PackageError(
            f"{context}: {path} must be a non-empty multiple of "
            f"{entry_size} bytes, got {size}"
        )
    entries = size // entry_size
    if entries > maximum:
        raise PackageError(
            f"{context}: {path} contains {entries} entries, maximum is {maximum}"
        )
    return entries


def text_archive_entry_count(root: Path, binary: str, context: str) -> int:
    """Read the entry count encoded by a BN6 text archive's offset table."""
    path = root / binary
    try:
        data = path.read_bytes()
    except OSError as exc:
        raise PackageError(f"{context}: cannot read {path}: {exc}") from exc
    if len(data) < 2:
        raise PackageError(f"{context}: {path} is too short for an offset table")
    table_size = int.from_bytes(data[:2], "little")
    if table_size == 0 or table_size & 1 or table_size > len(data):
        raise PackageError(
            f"{context}: {path} has invalid offset-table size 0x{table_size:X}"
        )
    entries = table_size // 2
    if entries > 0x100:
        raise PackageError(
            f"{context}: {path} contains {entries} entries, maximum is 256"
        )
    return entries


def load_config(path: Path) -> Config:
    context = str(path)
    root = path.resolve().parent
    try:
        raw = tomllib.loads(path.read_text())
    except (OSError, tomllib.TOMLDecodeError) as exc:
        raise PackageError(f"cannot read {path}: {exc}") from exc
    check_keys(
        raw,
        {
            "variant",
            "object_classes",
            "sprite_groups",
            "dust_sprites",
            "field_objects",
            "songs",
            "text",
            "chips",
            "attack_pools",
        },
        context,
    )
    variant = require_str(raw, "variant", context)
    check_name(variant, f"{path}: variant")
    object_classes: dict[int, ObjectClass] = {}
    for index, item in enumerate(require_table_array(raw, "object_classes", context)):
        item_context = f"{path}: object_classes[{index}]"
        check_keys(
            item,
            {
                "number",
                "native_table",
                "references",
            },
            item_context,
        )
        number = require_int(item, "number", item_context)
        if number in object_classes:
            raise PackageError(f"{item_context}: duplicate object class {number}")
        native_table = require_str(item, "native_table", item_context)
        native_entries = fixed_width_entry_count(
            root,
            native_table,
            4,
            0x100,
            f"{item_context}: native_table",
        )
        references = require_int_array(item, "references", item_context)
        if not references:
            raise PackageError(f"{item_context}: references must not be empty")
        if any(address % 4 for address in references):
            raise PackageError(f"{item_context}: references must be word-aligned")
        object_classes[number] = ObjectClass(
            number,
            native_entries,
            native_table,
            references,
        )

    sprite_groups: dict[int, SpriteGroup] = {}
    for index, item in enumerate(require_table_array(raw, "sprite_groups", context)):
        item_context = f"{path}: sprite_groups[{index}]"
        check_keys(
            item,
            {"number", "references", "native_table"},
            item_context,
        )
        number = require_int(item, "number", item_context)
        if number in sprite_groups:
            raise PackageError(f"{item_context}: duplicate sprite group 0x{number:X}")
        references = require_int_array(item, "references", item_context)
        if not references:
            raise PackageError(f"{item_context}: references must not be empty")
        if any(address % 4 for address in references):
            raise PackageError(f"{item_context}: references must be word-aligned")
        native_table = require_str(item, "native_table", item_context)
        sprite_groups[number] = SpriteGroup(
            number,
            references,
            native_table,
            fixed_width_entry_count(
                root,
                native_table,
                4,
                0x100,
                f"{item_context}: native_table",
            ),
        )

    songs_raw = require_table(raw, "songs", context)
    check_keys(songs_raw, {"native_table", "references"}, f"{path}: songs")
    song_table = require_str(songs_raw, "native_table", f"{path}: songs")
    songs = SongConfig(
        song_table,
        fixed_width_entry_count(
            root,
            song_table,
            8,
            0x10000,
            f"{path}: songs.native_table",
        ),
        require_int_array(songs_raw, "references", f"{path}: songs"),
    )

    dust_raw = require_table(raw, "dust_sprites", context)
    check_keys(
        dust_raw,
        {"native_table", "references", "max_kind", "reclaimed"},
        f"{path}: dust_sprites",
    )
    dust_table = require_str(
        dust_raw, "native_table", f"{path}: dust_sprites"
    )
    dust_references = require_int_array(
        dust_raw, "references", f"{path}: dust_sprites"
    )
    dust_max_kind = checked_int(
        require_int(dust_raw, "max_kind", f"{path}: dust_sprites"),
        0,
        0x0F,
        f"{path}: dust_sprites.max_kind",
    )
    if not dust_references or any(address % 4 for address in dust_references):
        raise PackageError(
            f"{path}: dust_sprites.references must be non-empty and word-aligned"
        )
    dust_native_entries = fixed_width_entry_count(
        root,
        dust_table,
        2,
        dust_max_kind + 1,
        f"{path}: dust_sprites.native_table",
    )
    try:
        dust_native_data = (root / dust_table).read_bytes()
    except OSError as exc:
        raise PackageError(
            f"{path}: dust_sprites.native_table: cannot read {root / dust_table}: {exc}"
        ) from exc
    reclaimed: list[DustSpriteReclaim] = []
    reclaimed_kinds: set[int] = set()
    for index, item in enumerate(
        require_table_array(dust_raw, "reclaimed", f"{path}: dust_sprites")
    ):
        item_context = f"{path}: dust_sprites.reclaimed[{index}]"
        check_keys(item, {"kind", "alias", "references"}, item_context)
        kind = checked_int(
            require_int(item, "kind", item_context),
            0,
            dust_native_entries - 1,
            f"{item_context}.kind",
        )
        alias = checked_int(
            require_int(item, "alias", item_context),
            0,
            dust_native_entries - 1,
            f"{item_context}.alias",
        )
        references = require_int_array(item, "references", item_context)
        if not references or any(address % 2 for address in references):
            raise PackageError(
                f"{item_context}.references must be non-empty and halfword-aligned"
            )
        if kind in reclaimed_kinds:
            raise PackageError(f"{item_context}: duplicate reclaimed kind 0x{kind:02X}")
        if (
            dust_native_data[kind * 2 : kind * 2 + 2]
            != dust_native_data[alias * 2 : alias * 2 + 2]
        ):
            raise PackageError(
                f"{item_context}: kind 0x{kind:02X} and alias 0x{alias:02X} "
                "must use the same native sprite selector"
            )
        reclaimed_kinds.add(kind)
        reclaimed.append(DustSpriteReclaim(kind, alias, references))
    dust_sprites = DustSpriteConfig(
        dust_table,
        dust_native_entries,
        dust_references,
        dust_max_kind,
        tuple(reclaimed),
    )

    field_raw = require_table(raw, "field_objects", context)
    check_keys(
        field_raw,
        {"native_table", "references", "base_id", "max_id"},
        f"{path}: field_objects",
    )
    field_table = require_str(
        field_raw, "native_table", f"{path}: field_objects"
    )
    field_references = require_int_array(
        field_raw, "references", f"{path}: field_objects"
    )
    if not field_references or any(address % 4 for address in field_references):
        raise PackageError(
            f"{path}: field_objects.references must be non-empty and word-aligned"
        )
    field_base_id = checked_int(
        require_int(field_raw, "base_id", f"{path}: field_objects"),
        0,
        0xFF,
        f"{path}: field_objects.base_id",
    )
    field_max_id = checked_int(
        require_int(field_raw, "max_id", f"{path}: field_objects"),
        field_base_id,
        0xFF,
        f"{path}: field_objects.max_id",
    )
    field_objects = FieldObjectConfig(
        field_table,
        fixed_width_entry_count(
            root,
            field_table,
            5,
            field_max_id - field_base_id + 1,
            f"{path}: field_objects.native_table",
        ),
        field_references,
        field_base_id,
        field_max_id,
    )

    chips_raw = require_table(raw, "chips", context)
    check_keys(
        chips_raw, {"table_address", "record_size", "record_count"}, f"{path}: chips"
    )
    chips = ChipConfig(
        require_int(chips_raw, "table_address", f"{path}: chips"),
        require_int(chips_raw, "record_size", f"{path}: chips"),
        require_int(chips_raw, "record_count", f"{path}: chips"),
    )

    attack_pools: dict[str, AttackPool] = {}
    attack_pool_values = require_table(raw, "attack_pools", context)
    for kind, item in attack_pool_values.items():
        item_context = f"{path}: attack_pools.{kind}"
        check_name(kind, item_context)
        if not isinstance(item, dict):
            raise PackageError(f"{item_context}: must be a table")
        check_keys(
            item,
            {"family", "native_table", "references"},
            item_context,
        )
        family = checked_int(
            require_int(item, "family", item_context),
            0,
            0xFF,
            f"{item_context}: family",
        )
        native_table = require_str(item, "native_table", item_context)
        native_entries = fixed_width_entry_count(
            root,
            native_table,
            4,
            0x100,
            f"{item_context}: native_table",
        )
        references = require_int_array(item, "references", item_context)
        if not references:
            raise PackageError(f"{item_context}: references must not be empty")
        if any(address % 4 for address in references):
            raise PackageError(f"{item_context}: references must be word-aligned")
        attack_pools[kind] = AttackPool(
            family,
            native_entries,
            native_table,
            references,
        )

    text_raw = require_table(raw, "text", context)
    check_keys(
        text_raw,
        {"folder_edit_skip_address", "folder_edit_skip_target", "archives"},
        f"{path}: text",
    )
    archives: list[TextArchive] = []
    archive_names: set[str] = set()
    for index, item in enumerate(
        require_table_array(text_raw, "archives", f"{path}: text")
    ):
        item_context = f"{path}: text.archives[{index}]"
        check_keys(
            item,
            {
                "name",
                "region",
                "source",
                "source_index",
                "encoding",
                "symbol",
                "binary",
                "references",
            },
            item_context,
        )
        name = require_str(item, "name", item_context)
        check_name(name, item_context)
        if name in archive_names:
            raise PackageError(f"{item_context}: duplicate text archive name {name!r}")
        archive_names.add(name)
        symbol = require_str(item, "symbol", item_context)
        if not SNAKE_CASE_RE.fullmatch(symbol):
            raise PackageError(
                f"{item_context}: assembly symbol {symbol!r} must be snake_case"
            )
        source = require_str(item, "source", item_context)
        encoding = require_str(item, "encoding", item_context)
        if source not in {"names", "descriptions"}:
            raise PackageError(f"{item_context}: unknown source {source!r}")
        if encoding not in {"name", "description"}:
            raise PackageError(f"{item_context}: unknown encoding {encoding!r}")
        binary = require_str(item, "binary", item_context)
        archives.append(
            TextArchive(
                name,
                require_int(item, "region", item_context),
                source,
                require_int(item, "source_index", item_context),
                encoding,
                text_archive_entry_count(root, binary, f"{item_context}: binary"),
                symbol,
                binary,
                require_int_array(item, "references", item_context),
            )
        )
    groups = tuple(
        TextArchiveGroup(
            tuple(archive for archive in archives if archive.region == region)
        )
        for region in sorted({archive.region for archive in archives})
    )
    text = TextConfig(
        (
            require_int(text_raw, "folder_edit_skip_address", f"{path}: text"),
            require_int(text_raw, "folder_edit_skip_target", f"{path}: text"),
        ),
        groups,
    )
    return Config(
        root,
        variant,
        object_classes,
        sprite_groups,
        dust_sprites,
        field_objects,
        songs,
        text,
        chips,
        attack_pools,
    )


def checked_int(value: Any, minimum: int, maximum: int, context: str) -> int:
    if (
        isinstance(value, bool)
        or not isinstance(value, int)
        or not minimum <= value <= maximum
    ):
        raise PackageError(
            f"{context}: must be an integer between {minimum} and {maximum}"
        )
    return value


def load_package(source_path: Path, config: Config) -> Package:
    name = source_path.stem
    check_name(name, str(source_path))
    text_definitions = source_path.with_suffix(".text.toml")
    if text_definitions.is_file():
        try:
            text_values = tomllib.loads(text_definitions.read_text())
        except (OSError, tomllib.TOMLDecodeError) as exc:
            raise PackageError(f"cannot read {text_definitions}: {exc}") from exc
    else:
        text_values = {}

    text: list[TextResource] = []
    archives = {
        archive.name: archive
        for group in config.text.groups
        for archive in group.archives
    }
    for archive_name, entries in text_values.items():
        context = f"{text_definitions}:{archive_name}"
        archive = archives.get(archive_name)
        if archive is None:
            raise PackageError(f"{context}: unknown text archive")
        if not isinstance(entries, dict):
            raise PackageError(f"{context}: entries must be a table")
        for entry_key, value in entries.items():
            entry_context = f"{context}.{entry_key}"
            try:
                entry_index = int(entry_key, 0)
            except ValueError as exc:
                raise PackageError(
                    f"{entry_context}: text index must be an integer string"
                ) from exc
            if not 0 <= entry_index < archive.native_entries:
                raise PackageError(
                    f"{entry_context}: text index must be between 0 and "
                    f"0x{archive.native_entries - 1:X}"
                )
            if not isinstance(value, str) or not value:
                raise PackageError(
                    f"{entry_context}: replacement must be a non-empty string"
                )
            text.append(TextResource(name, archive_name, entry_index, value))

    return Package(
        name,
        source_path.relative_to(config.root).as_posix(),
        text_definitions if text_definitions.is_file() else source_path,
        (),
        (),
        (),
        (),
        (),
        (),
        (),
        tuple(text),
        (),
        None,
        (),
        (),
        (),
        (),
    )


def load_metadata(path: Path) -> dict[str, list[str]]:
    try:
        raw = json.loads(path.read_text())
    except (OSError, json.JSONDecodeError) as exc:
        raise PackageError(f"cannot read C metadata {path}: {exc}") from exc
    entries = raw.get("packages") if isinstance(raw, dict) else None
    if not isinstance(entries, list):
        raise PackageError(f"{path}: packages must be an array")
    result: dict[str, list[str]] = {}
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("name"), str):
            raise PackageError(f"{path}: invalid package metadata entry")
        symbols = entry.get("symbols")
        if not isinstance(symbols, list) or not all(
            isinstance(item, str) for item in symbols
        ):
            raise PackageError(f"{path}: invalid symbols for {entry['name']}")
        result[entry["name"]] = symbols
    return result


def check_snake_resource_label(package: str, label: str, suffix: str) -> None:
    prefix = package.replace("-", "_") + "_"
    if not label.startswith(prefix) or not label.endswith(suffix):
        raise PackageError(
            f"{package}: {label} must use the {prefix}<name>{suffix} convention"
        )
    middle = label[len(prefix) : -len(suffix)]
    if not middle or SNAKE_CASE_RE.fullmatch(label) is None:
        raise PackageError(f"{package}: invalid snake_case resource name {label}")


def apply_metadata(package: Package, symbols: list[str]) -> Package:
    objects: list[ObjectResource] = []
    fixed_objects: list[FixedObjectResource] = []
    sprites: list[SpriteResource] = []
    fixed_sprites: list[FixedSpriteResource] = []
    dust_sprites: list[DustSpriteResource] = []
    field_objects: list[FieldObjectResource] = []
    songs: list[SongResource] = []
    chips: list[ChipResource] = []
    attack: AttackResource | None = None
    fixed_attacks: list[FixedAttackResource] = []
    pointer_patches: list[PointerPatch] = []
    section_patches: list[SectionPatch] = []
    linked_call_patches: list[LinkedCallPatch] = []
    for symbol in symbols:
        prefix = "__bn67_meta__"
        if not symbol.startswith(prefix):
            raise PackageError(f"{package.name}: invalid metadata symbol {symbol}")
        body = symbol[len(prefix) :]
        thumb_pointer = THUMB_POINTER_METADATA_RE.fullmatch(body)
        pointer = POINTER_METADATA_RE.fullmatch(body)
        if thumb_pointer is not None:
            address_text, patch_symbol = thumb_pointer.groups()
            address = checked_int(int(address_text, 0), 0, 0xFFFFFFFF, symbol)
            if SNAKE_CASE_RE.fullmatch(patch_symbol) is None:
                raise PackageError(
                    f"{package.name}: patch symbol {patch_symbol} must be snake_case"
                )
            pointer_patches.append(PointerPatch(patch_symbol, address, True))
            continue
        if pointer is not None:
            address_text, patch_symbol = pointer.groups()
            address = checked_int(int(address_text, 0), 0, 0xFFFFFFFF, symbol)
            if SNAKE_CASE_RE.fullmatch(patch_symbol) is None:
                raise PackageError(
                    f"{package.name}: patch symbol {patch_symbol} must be snake_case"
                )
            pointer_patches.append(PointerPatch(patch_symbol, address))
            continue
        section = SECTION_METADATA_RE.fullmatch(body)
        if section is not None:
            address_text, relay_address_text, patch_symbol = section.groups()
            address = checked_int(int(address_text, 0), 0, 0xFFFFFFFF, symbol)
            relay_address = checked_int(
                int(relay_address_text, 0), 0, 0xFFFFFFFF, symbol
            )
            if SNAKE_CASE_RE.fullmatch(patch_symbol) is None:
                raise PackageError(
                    f"{package.name}: patch symbol {patch_symbol} must be snake_case"
                )
            if address % 2:
                raise PackageError(
                    f"{package.name}: section patch address must be halfword-aligned"
                )
            if relay_address % 4:
                raise PackageError(
                    f"{package.name}: section patch relay must be word-aligned"
                )
            section_patches.append(
                SectionPatch(patch_symbol, address, relay_address)
            )
            continue
        linked_call = LINKED_CALL_METADATA_RE.fullmatch(body)
        if linked_call is not None:
            source, offset_text, target = linked_call.groups()
            offset = checked_int(int(offset_text, 0), 0, 0xFFFFFFFF, symbol)
            if offset % 2:
                raise PackageError(
                    f"{package.name}: linked call offset must be halfword-aligned"
                )
            linked_call_patches.append(LinkedCallPatch(source, offset, target))
            continue
        parts = body.split("__")
        kind = parts[0]
        if kind == "object" and len(parts) == 3:
            object_class = int(parts[1], 0)
            check_snake_resource_label(package.name, parts[2], "_main")
            objects.append(ObjectResource(object_class, parts[2]))
        elif kind == "fixed_object" and len(parts) == 4:
            check_snake_resource_label(package.name, parts[3], "_main")
            fixed_objects.append(
                FixedObjectResource(
                    checked_int(int(parts[1], 0), 0, 0xFF, symbol),
                    checked_int(int(parts[2], 0), 0, 0xFF, symbol),
                    parts[3],
                )
            )
        elif kind == "sprite" and len(parts) == 2:
            check_snake_resource_label(package.name, parts[1], "_sprite")
            sprites.append(SpriteResource(parts[1]))
        elif (
            kind in {"fixed_sprite", "fixed_compressed_sprite"}
            and len(parts) == 4
        ):
            check_snake_resource_label(package.name, parts[3], "_sprite")
            fixed_sprites.append(
                FixedSpriteResource(
                    checked_int(int(parts[1], 0), 0, 0xFF, symbol),
                    checked_int(int(parts[2], 0), 0, 0xFF, symbol),
                    parts[3],
                    kind == "fixed_compressed_sprite",
                )
            )
        elif kind == "dust_sprite" and len(parts) == 2:
            check_snake_resource_label(package.name, parts[1], "_sprite")
            dust_sprites.append(DustSpriteResource(parts[1]))
        elif kind == "field_object" and len(parts) == 5:
            check_snake_resource_label(package.name, parts[1], "_sprite")
            field_objects.append(
                FieldObjectResource(
                    parts[1],
                    checked_int(int(parts[2], 0), 0, 0xFF, symbol),
                    checked_int(int(parts[3], 0), 0, 0xFF, symbol),
                    checked_int(int(parts[4], 0), 0, 1, symbol),
                )
            )
        elif kind == "song" and len(parts) == 2:
            check_snake_resource_label(package.name, parts[1], "_song")
            songs.append(SongResource(parts[1]))
        elif kind == "chip" and len(parts) == 2:
            chip_id = checked_int(int(parts[1], 0), 0, 0xFFFF, symbol)
            chips.append(
                ChipResource(package.name, chip_id, f"bn67_chip_record_{parts[1]}")
            )
        elif (
            kind
            in {
                "persistent_attack",
                "summon_attack",
                "ephemeral_attack",
            }
            and len(parts) == 3
        ):
            if attack is not None:
                raise PackageError(f"{package.name}: duplicate BN6 attack declaration")
            chip_id = checked_int(int(parts[1], 0), 0, 0xFFFF, symbol)
            check_snake_resource_label(package.name, parts[2], "_main")
            attack = AttackResource(package.name, chip_id, kind, parts[2])
        elif kind == "fixed_attack" and len(parts) == 4:
            check_snake_resource_label(package.name, parts[3], "_main")
            fixed_attacks.append(
                FixedAttackResource(
                    checked_int(int(parts[1], 0), 0, 0xFF, symbol),
                    checked_int(int(parts[2], 0), 0, 0xFF, symbol),
                    parts[3],
                )
            )
        else:
            raise PackageError(f"{package.name}: invalid metadata symbol {symbol}")
    return replace(
        package,
        objects=tuple(objects),
        fixed_objects=tuple(fixed_objects),
        sprites=tuple(sprites),
        fixed_sprites=tuple(fixed_sprites),
        dust_sprites=tuple(dust_sprites),
        field_objects=tuple(field_objects),
        songs=tuple(songs),
        chips=tuple(chips),
        attack=attack,
        fixed_attacks=tuple(fixed_attacks),
        pointer_patches=tuple(pointer_patches),
        section_patches=tuple(section_patches),
        linked_call_patches=tuple(linked_call_patches),
    )


def discover_packages(config: Config, metadata: dict[str, list[str]]) -> list[Package]:
    sources = sorted(
        path.resolve()
        for path in config.root.glob("src/**/*.c")
        if path.name not in RUNTIME_SOURCE_NAMES
    )
    if not sources:
        raise PackageError("no gameplay implementations were found under src")
    packages = [load_package(path, config) for path in sources]
    unknown = sorted(set(metadata) - {package.name for package in packages})
    if unknown:
        raise PackageError(f"C metadata has no implementation: {', '.join(unknown)}")
    missing = sorted({package.name for package in packages} - set(metadata))
    if missing:
        raise PackageError(
            "C metadata is missing implementation(s): " + ", ".join(missing)
        )
    return [apply_metadata(package, metadata[package.name]) for package in packages]


def validate_and_allocate(config: Config, packages: list[Package]) -> Allocations:
    objects = [item for package in packages for item in package.objects]
    fixed_objects = [item for package in packages for item in package.fixed_objects]
    sprites = [item for package in packages for item in package.sprites]
    fixed_sprites = [item for package in packages for item in package.fixed_sprites]
    dust_sprites = [item for package in packages for item in package.dust_sprites]
    field_objects = [item for package in packages for item in package.field_objects]
    songs = [item for package in packages for item in package.songs]
    text = [item for package in packages for item in package.text]
    chips = [item for package in packages for item in package.chips]
    fixed_attacks = [item for package in packages for item in package.fixed_attacks]

    sprite_names = {item.archive for item in sprites}
    for item in fixed_sprites:
        group = config.sprite_groups.get(item.group)
        if group is None:
            raise PackageError(
                f"{item.archive}: unknown fixed sprite group 0x{item.group:02X}"
            )
        if item.index >= group.native_entries:
            raise PackageError(
                f"{item.archive}: fixed sprite 0x{item.group:02X}/0x{item.index:02X} "
                "is outside the native table"
            )
        if item.archive in sprite_names:
            raise PackageError(f"{item.archive}: duplicate sprite declaration")
        sprite_names.add(item.archive)
    fixed_slots: dict[tuple[int, int], str] = {}
    for item in fixed_sprites:
        key = (item.group, item.index)
        if key in fixed_slots:
            raise PackageError(
                f"fixed sprite 0x{item.group:02X}/0x{item.index:02X} is declared "
                f"by both {fixed_slots[key]} and {item.archive}"
            )
        fixed_slots[key] = item.archive
    dust_names: set[str] = set()
    for item in dust_sprites:
        if item.archive not in sprite_names:
            raise PackageError(
                f"{item.archive}: DustCross ammo must also be declared with BN67_SPRITE"
            )
        if item.archive in dust_names:
            raise PackageError(f"{item.archive}: duplicate DustCross ammo declaration")
        dust_names.add(item.archive)
    dust_slots = sorted(item.kind for item in config.dust_sprites.reclaimed)
    dust_slots.extend(
        range(config.dust_sprites.native_entries, config.dust_sprites.max_kind + 1)
    )
    if len(dust_sprites) > len(dust_slots):
        raise PackageError(
            f"DustCross sprite table has {len(dust_slots)} custom slots for "
            f"{len(dust_sprites)} registered sprites"
        )
    dust_allocations = {
        item.archive: dust_slots[index] for index, item in enumerate(dust_sprites)
    }

    field_names: set[str] = set()
    for item in field_objects:
        if item.archive not in sprite_names:
            raise PackageError(
                f"{item.archive}: field object must also be declared with BN67_SPRITE"
            )
        if item.archive in field_names:
            raise PackageError(f"{item.archive}: duplicate field-object declaration")
        field_names.add(item.archive)
    first_custom_field_id = (
        config.field_objects.base_id + config.field_objects.native_entries
    )
    field_capacity = config.field_objects.max_id - first_custom_field_id + 1
    if len(field_objects) > field_capacity:
        raise PackageError(
            f"field-object table has {field_capacity} custom IDs for "
            f"{len(field_objects)} registered objects"
        )
    field_allocations = {
        item.archive: first_custom_field_id + index
        for index, item in enumerate(field_objects)
    }

    for item in objects:
        if item.object_class not in config.object_classes:
            raise PackageError(f"{item.main}: unknown object class {item.object_class}")
    object_names = {item.main for item in objects}
    fixed_object_slots: dict[tuple[int, int], str] = {}
    for item in fixed_objects:
        object_class = config.object_classes.get(item.object_class)
        if object_class is None:
            raise PackageError(
                f"{item.main}: unknown fixed object class {item.object_class}"
            )
        if item.object_id >= object_class.native_entries:
            raise PackageError(
                f"{item.main}: fixed object class {item.object_class} "
                f"ID 0x{item.object_id:02X} is outside the native table"
            )
        if item.main in object_names:
            raise PackageError(f"{item.main}: duplicate object declaration")
        object_names.add(item.main)
        key = (item.object_class, item.object_id)
        if key in fixed_object_slots:
            raise PackageError(
                f"fixed object class {item.object_class} ID 0x{item.object_id:02X} "
                f"is declared by both {fixed_object_slots[key]} and {item.main}"
            )
        fixed_object_slots[key] = item.main
    text_owners: dict[tuple[str, int], str] = {}
    for item in text:
        key = (item.archive, item.index)
        if key in text_owners:
            raise PackageError(
                f"text archive {item.archive!r} index 0x{item.index:X} is declared by both "
                f"{text_owners[key]} and {item.package}"
            )
        text_owners[key] = item.package
    chip_owners: dict[int, str] = {}
    for item in chips:
        if not 0 <= item.chip_id < config.chips.record_count:
            raise PackageError(
                f"{item.record}: chip ID must be between 0x000 and "
                f"0x{config.chips.record_count - 1:03X}"
            )
        if item.chip_id in chip_owners:
            raise PackageError(
                f"chip 0x{item.chip_id:03X} is declared by both "
                f"{chip_owners[item.chip_id]} and {item.package}"
            )
        chip_owners[item.chip_id] = item.package

    attack_allocations = allocate_attacks(config, packages)

    attack_pools = {pool.family: pool for pool in config.attack_pools.values()}
    fixed_attack_slots: dict[tuple[int, int], str] = {}
    for item in fixed_attacks:
        pool = attack_pools.get(item.family)
        if pool is None:
            raise PackageError(
                f"{item.main}: no compiler-owned attack family 0x{item.family:02X}"
            )
        if item.subfamily >= pool.native_entries:
            raise PackageError(
                f"{item.main}: fixed attack 0x{item.family:02X}/"
                f"0x{item.subfamily:02X} is outside the native table"
            )
        key = (item.family, item.subfamily)
        if key in fixed_attack_slots:
            raise PackageError(
                f"fixed attack 0x{item.family:02X}/0x{item.subfamily:02X} "
                f"is declared by both {fixed_attack_slots[key]} and {item.main}"
            )
        fixed_attack_slots[key] = item.main

    object_allocations: dict[int, dict[str, int]] = {}
    for number, object_class in config.object_classes.items():
        names = [item.main for item in objects if item.object_class == number]
        first_custom_id = object_class.native_entries
        capacity = 0x100 - first_custom_id
        if len(names) > capacity:
            raise PackageError(
                f"object class {number} has {capacity} entries for "
                f"{len(names)} package objects"
            )
        object_allocations[number] = {
            name: first_custom_id + index for index, name in enumerate(names)
        }

    # Sprite handles contain both a table group and an index. Spread resources
    # over the fixed native tables in a stable round-robin order; package authors
    # only declare the resource and use the two generated constants.
    group_numbers = sorted(config.sprite_groups)
    next_sprite_index = {
        number: config.sprite_groups[number].native_entries for number in group_numbers
    }
    sprite_allocations: dict[str, tuple[int, int]] = {}
    group_cursor = 0
    for item in sprites:
        for offset in range(len(group_numbers)):
            position = (group_cursor + offset) % len(group_numbers)
            number = group_numbers[position]
            resource_id = next_sprite_index[number]
            if resource_id <= 0xFF:
                sprite_allocations[item.archive] = (number, resource_id)
                next_sprite_index[number] += 1
                group_cursor = (position + 1) % len(group_numbers)
                break
        else:
            raise PackageError("all sprite groups are full")

    if config.songs.native_entries + len(songs) > 0x10000:
        raise PackageError("song table exceeds 65536 entries")
    song_allocations = {
        item.archive: config.songs.native_entries + index
        for index, item in enumerate(songs)
    }
    player_count = SONG_PLAYER_LAST - SONG_PLAYER_FIRST + 1
    if len(songs) > player_count:
        raise PackageError(
            f"more than {player_count} imported songs; no deterministic player groups remain"
        )
    song_players = {
        item.archive: SONG_PLAYER_FIRST + index for index, item in enumerate(songs)
    }
    return Allocations(
        object_allocations,
        sprite_allocations,
        dust_allocations,
        field_allocations,
        song_allocations,
        song_players,
        attack_allocations,
    )


def allocate_attacks(
    config: Config,
    packages: list[Package],
) -> dict[str, AttackAllocation]:
    """Give every package attack a direct subfamily entry for its native ABI."""
    allocations: dict[str, AttackAllocation] = {}
    families = [pool.family for pool in config.attack_pools.values()]
    if len(set(families)) != len(families):
        raise PackageError("attack pools must use distinct native families")

    for package in packages:
        if package.attack is None:
            continue
        if package.attack.chip_id not in {chip.chip_id for chip in package.chips}:
            raise PackageError(
                f"{package.source}: BN6 attack declaration refers to chip "
                f"0x{package.attack.chip_id:03X}, but {package.definitions} "
                "does not declare it"
            )
        if package.attack.kind not in config.attack_pools:
            raise PackageError(
                f"{package.source}: no attack pool is configured for "
                f"{package.attack.kind!r}"
            )

    for kind, pool in config.attack_pools.items():
        attacks = sorted(
            (
                package.attack
                for package in packages
                if package.attack is not None and package.attack.kind == kind
            ),
            key=lambda attack: (attack.chip_id, attack.package, attack.main),
        )
        capacity = 0x100 - pool.native_entries
        if len(attacks) > capacity:
            raise PackageError(
                f"attack pool {kind!r} has {capacity} entries for "
                f"{len(attacks)} attacks"
            )
        for index, attack in enumerate(attacks):
            allocations[attack.main] = AttackAllocation(
                pool.family,
                pool.native_entries + index,
            )
    return allocations


def generate_linker_values(packages: list[Package], allocations: Allocations) -> str:
    """Define C-visible absolute symbols after deterministic allocation."""
    lines = [
        "/* Generated by compile_registry.py from C object metadata. */",
        "/* Absolute symbols are selectors, not addresses in the ROM. */",
    ]
    for package in packages:
        for item in package.objects:
            value = allocations.objects[item.object_class][item.main]
            lines.append(f"__bn67_object_id_{item.main} = 0x{value:X};")
        for item in package.sprites:
            group, resource_id = allocations.sprites[item.archive]
            lines.append(f"__bn67_sprite_group_{item.archive} = 0x{group:X};")
            lines.append(f"__bn67_sprite_id_{item.archive} = 0x{resource_id:X};")
        for item in package.dust_sprites:
            lines.append(
                f"__bn67_dust_kind_{item.archive} = "
                f"0x{allocations.dust_sprites[item.archive]:X};"
            )
        for item in package.field_objects:
            lines.append(
                f"__bn67_field_object_id_{item.archive} = "
                f"0x{allocations.field_objects[item.archive]:X};"
            )
        for item in package.songs:
            player = allocations.song_players[item.archive]
            song_id = allocations.songs[item.archive]
            lines.append(f"__bn67_song_group_{item.archive} = 0x{player:X};")
            lines.append(f"__bn67_song_id_{item.archive} = 0x{song_id:X};")
    return "\n".join(lines) + "\n"


def generate_c_values(allocations: Allocations) -> str:
    """Expose allocated attack selectors as integer constant expressions."""
    lines = [
        "/* Generated by compile_registry.py. Do not edit. */",
        "#define BN67_ATTACK_FAMILY(main) \\",
        "    BN67_JOIN(bn67_attack_family_, main)",
        "#define BN67_ATTACK_SUBFAMILY(main) \\",
        "    BN67_JOIN(bn67_attack_subfamily_, main)",
    ]
    for main, allocation in sorted(allocations.attacks.items()):
        lines.append(
            f"#define bn67_attack_family_{main} 0x{allocation.family:02X}u"
        )
        lines.append(
            f"#define bn67_attack_subfamily_{main} 0x{allocation.subfamily:02X}u"
        )
    return "\n".join(lines) + "\n"


def emit_chip_records(
    config: Config, packages: list[Package]
) -> list[str]:
    lines = ["// Complete chip records linked into the C gameplay image."]
    for package in packages:
        for chip in package.chips:
            address = config.chips.table_address + chip.chip_id * config.chips.record_size
            lines.extend(
                [
                    "",
                    f"// {chip.package}: chip 0x{chip.chip_id:03X}",
                    f"copy_c_data 0x{address:08X},{chip.record},"
                    f"0x{config.chips.record_size:X}",
                ]
            )
    return lines


def emit_attack_tables(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    """Relocate native attack tables and append allocated subfamilies."""
    lines = ["// Compiler-owned chip attack tables."]
    for kind, pool in config.attack_pools.items():
        label = f"attack_family_{pool.family:02x}_table"
        lines.extend(["", f"// {kind}: native family 0x{pool.family:02X}"])
        for address in pool.references:
            lines.extend([f".org 0x{address:08X}", f"    .dw {label}"])

    attacks = {
        package.attack.main: package.attack
        for package in packages
        if package.attack is not None
    }
    fixed_attacks = [
        item for package in packages for item in package.fixed_attacks
    ]
    for pool in config.attack_pools.values():
        label = f"attack_family_{pool.family:02x}_table"
        assigned = {
            allocation.subfamily: attacks[main]
            for main, allocation in allocations.attacks.items()
            if allocation.family == pool.family
        }
        fixed = {
            item.subfamily: item for item in fixed_attacks if item.family == pool.family
        }
        lines.extend(
            [
                "",
                ".autoregion",
                ".align 4",
                f"{label}:",
            ]
        )
        native_cursor = 0
        for subfamily, item in sorted(fixed.items()):
            if native_cursor < subfamily:
                lines.append(
                    f'    .incbin "{pool.native_table}",'
                    f"0x{native_cursor * 4:X},0x{(subfamily - native_cursor) * 4:X}"
                )
            lines.append(
                f"    .dw {item.main} + 1 // 0x{subfamily:02X} {item.main}"
            )
            native_cursor = subfamily + 1
        if native_cursor < pool.native_entries:
            lines.append(
                f'    .incbin "{pool.native_table}",'
                f"0x{native_cursor * 4:X},"
                f"0x{(pool.native_entries - native_cursor) * 4:X}"
            )
        for subfamily, attack in sorted(assigned.items()):
            lines.append(
                f"    .dw {attack.main} + 1 // 0x{subfamily:02X} {attack.package}"
            )
        lines.extend([f"{label}_end:", ".endautoregion"])
    return lines


def emit_object_tables(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    """Relocate native object-class tables and append allocated IDs."""
    lines = ["// Compiler-owned object-class tables."]
    class_numbers = sorted(config.object_classes)
    for number in class_numbers:
        object_class = config.object_classes[number]
        label = f"object_class_{number}_table"
        lines.extend(["", f"// Object class {number}"])
        for address in object_class.references:
            lines.extend([f".org 0x{address:08X}", f"    .dw {label}"])

    all_objects = [item for package in packages for item in package.objects]
    all_fixed_objects = [
        item for package in packages for item in package.fixed_objects
    ]
    for number in class_numbers:
        object_class = config.object_classes[number]
        namespace = allocations.objects[number]
        active = {
            item.main: item for item in all_objects if item.object_class == number
        }
        reverse = {resource_id: name for name, resource_id in namespace.items()}
        fixed = {
            item.object_id: item
            for item in all_fixed_objects
            if item.object_class == number
        }
        label = f"object_class_{number}_table"
        lines.extend(
            [
                "",
                ".autoregion",
                ".align 4",
                f"{label}:",
            ]
        )
        native_cursor = 0
        for object_id, item in sorted(fixed.items()):
            if native_cursor < object_id:
                lines.append(
                    f'    .incbin "{object_class.native_table}",'
                    f"0x{native_cursor * 4:X},0x{(object_id - native_cursor) * 4:X}"
                )
            lines.append(
                f"    .dw {item.main} + 1 // 0x{object_id:02X} {item.main}"
            )
            native_cursor = object_id + 1
        if native_cursor < object_class.native_entries:
            lines.append(
                f'    .incbin "{object_class.native_table}",'
                f"0x{native_cursor * 4:X},"
                f"0x{(object_class.native_entries - native_cursor) * 4:X}"
            )
        for resource_id, name in sorted(reverse.items()):
            lines.append(
                f"    .dw {active[name].main} + 1 // 0x{resource_id:02X} {name}"
            )
        lines.extend([f"{label}_end:", ".endautoregion"])
    return lines


def emit_pointer_patches(packages: list[Package]) -> list[str]:
    lines = ["// Package-declared fixed pointer patches."]
    for package in packages:
        for patch in package.pointer_patches:
            value = f"{patch.symbol} + 1" if patch.thumb else patch.symbol
            lines.extend(
                [
                    f"// {package.name}: {patch.symbol}",
                    f".org 0x{patch.address:08X}",
                    f"    .dw {value}",
                ]
            )
    return lines


def emit_section_patches(packages: list[Package]) -> list[str]:
    lines = ["// Package-declared fixed section patches."]
    for package in packages:
        for patch in package.section_patches:
            relay = f"section_patch_{patch.symbol}_relay"
            # Keep the fixed-site patch to six bytes so literals cannot consume
            # adjacent native instructions or internal branch targets. The
            # package supplies eight dead, word-aligned bytes for the relay.
            lines.extend(
                [
                    f"// {package.name}: {patch.symbol}",
                    f".org 0x{patch.address:08X}",
                    "    push {r1}",
                    f"    bl {relay}",
                    f".org 0x{patch.relay_address:08X}",
                    f"{relay}:",
                    f"    ldr r1,={patch.symbol} + 1",
                    "    bx r1",
                    "    .pool",
                ]
            )
    return lines


def emit_linked_call_patches(packages: list[Package]) -> list[str]:
    lines = ["// Package-declared calls inside linked gameplay routines."]
    for package in packages:
        for patch in package.linked_call_patches:
            lines.extend(
                [
                    f"// {package.name}: {patch.source} -> {patch.target}",
                    f".org {patch.source} + 0x{patch.offset:X}",
                    f"    bl {patch.target}",
                ]
            )
    return lines


def emit_sprite_tables(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    lines = ["// Relocated native sprite groups with package-owned archives appended."]
    all_sprites = [item for package in packages for item in package.sprites]
    all_fixed_sprites = [
        item for package in packages for item in package.fixed_sprites
    ]
    for number, group in sorted(config.sprite_groups.items()):
        table = f"imported_sprite_group_{number:02x}_table"
        for address in group.references:
            lines.extend([f".org 0x{address:08X}", f".dw {table}"])
    for number, group in sorted(config.sprite_groups.items()):
        active = {
            item.archive: item
            for item in all_sprites
            if allocations.sprites[item.archive][0] == number
        }
        reverse = {
            resource_id: name
            for name, (allocated_group, resource_id) in allocations.sprites.items()
            if allocated_group == number
        }
        table = f"imported_sprite_group_{number:02x}_table"
        fixed = {
            item.index: item
            for item in all_fixed_sprites
            if item.group == number
        }
        lines.extend(
            [
                "",
                ".autoregion",
                ".align 4",
                f"{table}:",
            ]
        )
        native_cursor = 0
        for resource_id, item in sorted(fixed.items()):
            if native_cursor < resource_id:
                lines.append(
                    f'    .incbin "{group.native_table}",'
                    f"0x{native_cursor * 4:X},0x{(resource_id - native_cursor) * 4:X}"
                )
            pointer = item.archive
            if item.compressed:
                pointer += " + 0x80000000"
            lines.append(
                f"    .dw {pointer} // 0x{resource_id:02X} {item.archive}"
            )
            native_cursor = resource_id + 1
        if native_cursor < group.native_entries:
            lines.append(
                f'    .incbin "{group.native_table}",'
                f"0x{native_cursor * 4:X},0x{(group.native_entries - native_cursor) * 4:X}"
            )
        for resource_id in sorted(reverse):
            name = reverse[resource_id]
            item = active[name]
            lines.append(f"    .dw {item.archive} // 0x{resource_id:02X} {name}")
        lines.extend([f"{table}_end:", ".endautoregion"])
    return lines


def emit_dust_sprite_table(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    """Relocate DustCross selectors and append registered obstacle sprites."""
    dust = config.dust_sprites
    resources = {
        allocations.dust_sprites[item.archive]: item
        for package in packages
        for item in package.dust_sprites
    }
    lines = ["// Compiler-owned DustCross ammo sprite table."]
    for address in dust.references:
        lines.extend([f".org 0x{address:08X}", "    .dw dust_sprite_table"])
    for reclaimed in dust.reclaimed:
        for address in reclaimed.references:
            lines.extend(
                [
                    f".org 0x{address:08X}",
                    f"    mov r0,0x{reclaimed.alias:02X}",
                ]
            )
    lines.extend(
        [
            "",
            ".autoregion",
            ".align 4",
            "dust_sprite_table:",
        ]
    )
    native_cursor = 0
    for kind, item in sorted(resources.items()):
        if kind < dust.native_entries and native_cursor < kind:
            lines.append(
                f'    .incbin "{dust.native_table}",'
                f"0x{native_cursor * 2:X},0x{(kind - native_cursor) * 2:X}"
            )
        elif kind >= dust.native_entries and native_cursor < dust.native_entries:
            lines.append(
                f'    .incbin "{dust.native_table}",'
                f"0x{native_cursor * 2:X},0x{(dust.native_entries - native_cursor) * 2:X}"
            )
            native_cursor = dust.native_entries
        if kind < dust.native_entries:
            native_cursor = kind + 1
        group, sprite_id = allocations.sprites[item.archive]
        lines.append(
            f"    .byte 0x{group:02X},0x{sprite_id:02X} "
            f"// 0x{kind:02X} {item.archive}"
        )
    if native_cursor < dust.native_entries:
        lines.append(
            f'    .incbin "{dust.native_table}",'
            f"0x{native_cursor * 2:X},0x{(dust.native_entries - native_cursor) * 2:X}"
        )
    lines.extend(["dust_sprite_table_end:", ".endautoregion"])
    return lines


def emit_field_object_table(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    """Relocate native field-object render records and append allocated IDs."""
    field = config.field_objects
    resources = {
        item.archive: item
        for package in packages
        for item in package.field_objects
    }
    lines = ["// Compiler-owned field-object ID and sprite table."]
    for address in field.references:
        lines.extend([f".org 0x{address:08X}", "    .dw field_object_sprite_table"])
    lines.extend(
        [
            "",
            ".autoregion",
            ".align 4",
            "field_object_sprite_table:",
            f'    .incbin "{field.native_table}"',
        ]
    )
    for archive, object_id in sorted(
        allocations.field_objects.items(), key=lambda item: item[1]
    ):
        item = resources[archive]
        group, sprite_id = allocations.sprites[archive]
        lines.append(
            f"    .byte 0x{group:02X},0x{sprite_id:02X},"
            f"0x{item.animation:02X},0x{item.palette:02X},0x{item.shadow:02X} "
            f"// 0x{object_id:02X} {archive}"
        )
    lines.extend(["field_object_sprite_table_end:", ".endautoregion"])
    return lines


def emit_song_table(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    lines = ["// Relocated native song table with package-owned songs appended."]
    for address in config.songs.references:
        lines.extend([f".org 0x{address:08X}", "    .dw relocated_song_table"])
    lines.extend(
        [
            "",
            ".autoregion",
            ".align 4",
            "relocated_song_table:",
            f'    .incbin "{config.songs.native_table}"',
        ]
    )
    active = {item.archive: item for package in packages for item in package.songs}
    reverse = {resource_id: name for name, resource_id in allocations.songs.items()}
    for resource_id in range(
        config.songs.native_entries,
        config.songs.native_entries + len(allocations.songs),
    ):
        name = reverse[resource_id]
        item = active[name]
        lines.extend(
            [
                f"    .dw {item.archive} // 0x{resource_id:04X} {name}",
                f"    .dh 0x{allocations.song_players[name]:04X},0x{allocations.song_players[name]:04X}",
            ]
        )
    lines.extend(["relocated_song_table_end:", ".endautoregion"])
    return lines


def emit_text_archives(config: Config) -> list[str]:
    """Emit the fixed BN6 archive installer fed by package-generated text binaries."""
    archives = [archive for group in config.text.groups for archive in group.archives]
    skip_address, skip_target = config.text.folder_edit_skip
    lines = [
        "// Complete chip-text archives with changes declared by packages.",
        f".org 0x{skip_address:08X}",
        f"    b 0x{skip_target:08X}",
    ]
    for archive in archives:
        for address in archive.references:
            lines.extend([f".org 0x{address:08X}", f"    .dw {archive.symbol}"])
    for group in config.text.groups:
        lines.extend(["", ".autoregion"])
        for archive in group.archives:
            lines.extend(
                [
                    ".align 4",
                    f"{archive.symbol}:",
                    f'    .incbin "{archive.binary}"',
                ]
            )
        lines.append(".endautoregion")
    return lines


def generate_text_manifest(config: Config, packages: list[Package]) -> str:
    archives = [
        {
            "name": archive.name,
            "source": archive.source,
            "source_index": archive.source_index,
            "encoding": archive.encoding,
        }
        for group in config.text.groups
        for archive in group.archives
    ]
    entries = [
        {
            "package": item.package,
            "archive": item.archive,
            "index": item.index,
            "value": item.value,
        }
        for package in packages
        for item in package.text
    ]
    return json.dumps({"archives": archives, "entries": entries}, indent=2) + "\n"


def generate(config: Config, packages: list[Package], allocations: Allocations) -> str:
    sections: list[list[str]] = [
        [
            "// Generated by compile_registry.py from C object metadata and sibling .text.toml files.",
            "// Do not edit this file or assign registry IDs by hand.",
            "// IDs follow sorted source paths and ELF symbol order.",
            "",
        ],
        emit_pointer_patches(packages),
        [""],
        emit_section_patches(packages),
        [""],
        emit_linked_call_patches(packages),
        [""],
        emit_attack_tables(config, packages, allocations),
        [""],
        emit_chip_records(config, packages),
        [""],
        emit_object_tables(config, packages, allocations),
        [""],
        emit_sprite_tables(config, packages, allocations),
        [""],
        emit_field_object_table(config, packages, allocations),
        [""],
        emit_dust_sprite_table(config, packages, allocations),
        [""],
        emit_song_table(config, packages, allocations),
        [""],
        emit_text_archives(config),
    ]
    return "\n".join(line for section in sections for line in section) + "\n"


def write_if_changed(path: Path, content: str) -> None:
    if path.exists() and path.read_text() == content:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("config", type=Path)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--text-output", type=Path)
    parser.add_argument("--metadata", type=Path)
    parser.add_argument("--linker-output", type=Path)
    parser.add_argument("--c-output", type=Path)
    parser.add_argument(
        "--check", action="store_true", help="fail if the generated output is stale"
    )
    args = parser.parse_args()

    try:
        config_path = args.config.resolve()
        config = load_config(config_path)
        artifact_name = config_path.stem.removeprefix("config.")
        metadata_path = (
            args.metadata.resolve()
            if args.metadata
            else config.root / f"build/registry-metadata-{artifact_name}.generated.json"
        )
        packages = discover_packages(config, load_metadata(metadata_path))
        output_path = (
            args.output.resolve()
            if args.output
            else config.root / f"build/registry-{artifact_name}.generated.asm"
        )
        text_output_path = (
            args.text_output.resolve()
            if args.text_output
            else config.root / f"build/text-replacements-{artifact_name}.generated.json"
        )
        linker_output_path = (
            args.linker_output.resolve()
            if args.linker_output
            else config.root / f"build/registry-values-{artifact_name}.generated.ld"
        )
        c_output_path = (
            args.c_output.resolve()
            if args.c_output
            else config.root / f"build/registry-values-{artifact_name}.generated.h"
        )
        allocations = validate_and_allocate(config, packages)
        output_text = generate(config, packages, allocations)
        text_manifest = generate_text_manifest(config, packages)
        linker_values = generate_linker_values(packages, allocations)
        c_values = generate_c_values(allocations)
        if args.check:
            stale: list[str] = []
            if not output_path.exists() or output_path.read_text() != output_text:
                stale.append(str(output_path))
            if (
                not text_output_path.exists()
                or text_output_path.read_text() != text_manifest
            ):
                stale.append(str(text_output_path))
            if (
                not linker_output_path.exists()
                or linker_output_path.read_text() != linker_values
            ):
                stale.append(str(linker_output_path))
            if not c_output_path.exists() or c_output_path.read_text() != c_values:
                stale.append(str(c_output_path))
            if stale:
                raise PackageError(
                    "generated package file is stale: " + ", ".join(stale)
                )
        else:
            write_if_changed(output_path, output_text)
            write_if_changed(text_output_path, text_manifest)
            write_if_changed(linker_output_path, linker_values)
            write_if_changed(c_output_path, c_values)
    except PackageError as exc:
        print(f"registry compiler: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
