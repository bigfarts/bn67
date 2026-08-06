#!/usr/bin/env python3
"""Compile BN6 C metadata and definitions into registry glue."""

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
ASM_SYMBOL_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")
RUNTIME_SOURCE_NAMES = {"abi.c", "runtime.c"}
SONG_PLAYER_FIRST = 0x0C
SONG_PLAYER_LAST = 0x1F

CHIP_CODE_VALUES = {
    letter: index for index, letter in enumerate("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
}
CHIP_CODE_VALUES["*"] = 0x1A
CHIP_ELEMENTS = {
    "fire": 0x00,
    "aqua": 0x01,
    "elec": 0x02,
    "wood": 0x03,
    "bonus": 0x04,
    "sword": 0x05,
    "cursor": 0x06,
    "obstacle": 0x07,
    "wind": 0x08,
    "break": 0x09,
    "null": 0x0A,
}
CHIP_CLASSES = {
    "standard": 0x00,
    "mega": 0x01,
    "giga": 0x02,
    "program-advance": 0x04,
}


class PackageError(Exception):
    pass


@dataclass(frozen=True)
class ObjectClass:
    number: int
    native_id: int
    dispatch_entry: int


@dataclass(frozen=True)
class SpriteGroup:
    number: int
    pointer_address: int
    native_table: str
    native_entries: int


@dataclass(frozen=True)
class SongConfig:
    native_table: str
    native_entries: int
    gregar_references: tuple[int, ...]
    falzar_references: tuple[int, ...]


@dataclass(frozen=True)
class TextArchive:
    name: str
    source: str
    source_index: int
    encoding: str
    native_entries: int
    symbol: str
    binary: str
    gregar_references: tuple[int, ...]
    falzar_references: tuple[int, ...]


@dataclass(frozen=True)
class TextArchiveGroup:
    archives: tuple[TextArchive, ...]


@dataclass(frozen=True)
class TextConfig:
    gregar_folder_edit_skip: tuple[int, int]
    falzar_folder_edit_skip: tuple[int, int]
    groups: tuple[TextArchiveGroup, ...]


@dataclass(frozen=True)
class ChipConfig:
    table_address: int
    record_size: int
    record_count: int


@dataclass(frozen=True)
class AttackRoute:
    family: int
    subfamily: int
    hook_address: int
    gregar_BN6_EXPORT: int | None
    falzar_BN6_EXPORT: int | None


@dataclass(frozen=True)
class Config:
    root: Path
    object_classes: dict[int, ObjectClass]
    sprite_groups: dict[int, SpriteGroup]
    songs: SongConfig
    text: TextConfig
    chips: ChipConfig
    attack_routes: dict[tuple[int, int], AttackRoute]


@dataclass(frozen=True)
class EditionSongConfig:
    native_table: str
    native_entries: int
    references: tuple[int, ...]


@dataclass(frozen=True)
class EditionTextArchive:
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
class EditionTextConfig:
    folder_edit_skip: tuple[int, int]
    archives: tuple[EditionTextArchive, ...]


@dataclass(frozen=True)
class EditionConfig:
    object_classes: dict[int, ObjectClass]
    sprite_groups: dict[int, SpriteGroup]
    songs: EditionSongConfig
    text: EditionTextConfig
    chips: ChipConfig
    attack_routes: dict[tuple[int, int], tuple[int, int | None]]


@dataclass(frozen=True)
class ObjectResource:
    object_class: int
    main: str


@dataclass(frozen=True)
class SpriteResource:
    archive: str


@dataclass(frozen=True)
class SongResource:
    archive: str


@dataclass(frozen=True)
class TextResource:
    package: str
    archive: str
    index: int
    value: str | tuple[str, ...]


ChipValue = int | str | tuple[int, ...]


@dataclass(frozen=True)
class ChipResource:
    package: str
    chip_id: int
    common: tuple[tuple[str, ChipValue], ...]
    gregar: tuple[tuple[str, ChipValue], ...]
    falzar: tuple[tuple[str, ChipValue], ...]


@dataclass(frozen=True)
class AttackResource:
    package: str
    chip_id: int
    main: str


@dataclass(frozen=True)
class PointerPatch:
    symbol: str
    gregar_address: int | None
    falzar_address: int | None


@dataclass(frozen=True)
class Package:
    name: str
    source: str
    definitions: Path
    includes: tuple[str, ...]
    objects: tuple[ObjectResource, ...]
    sprites: tuple[SpriteResource, ...]
    songs: tuple[SongResource, ...]
    text: tuple[TextResource, ...]
    chips: tuple[ChipResource, ...]
    attack: AttackResource | None
    pointer_patches: tuple[PointerPatch, ...]


@dataclass(frozen=True)
class Allocations:
    objects: dict[int, dict[str, int]]
    sprites: dict[str, tuple[int, int]]
    songs: dict[str, int]
    song_players: dict[str, int]


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


def pascal_case(value: str) -> str:
    return "".join(part[0].upper() + part[1:] for part in re.split(r"[-_]", value))


def symbol_case(value: str) -> str:
    return re.sub(r"[-_]", "_", value).upper()


def convention_label(package: str, resource: str, suffix: str) -> str:
    return f"{pascal_case(package)}{pascal_case(resource)}{suffix}"


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


def load_edition_config(path: Path, expected_edition: str) -> EditionConfig:
    context = str(path)
    try:
        raw = tomllib.loads(path.read_text())
    except (OSError, tomllib.TOMLDecodeError) as exc:
        raise PackageError(f"cannot read {path}: {exc}") from exc
    check_keys(
        raw,
        {
            "edition",
            "object_classes",
            "sprite_groups",
            "songs",
            "text",
            "chips",
            "attack_routes",
        },
        context,
    )
    edition = require_str(raw, "edition", context)
    if edition != expected_edition:
        raise PackageError(
            f"{path}: edition must be {expected_edition!r}, got {edition!r}"
        )
    object_classes: dict[int, ObjectClass] = {}
    for index, item in enumerate(require_table_array(raw, "object_classes", context)):
        item_context = f"{path}: object_classes[{index}]"
        check_keys(item, {"number", "native_id", "dispatch_entry"}, item_context)
        number = require_int(item, "number", item_context)
        if number in object_classes:
            raise PackageError(f"{item_context}: duplicate object class {number}")
        object_classes[number] = ObjectClass(
            number,
            require_int(item, "native_id", item_context),
            require_int(item, "dispatch_entry", item_context),
        )

    sprite_groups: dict[int, SpriteGroup] = {}
    for index, item in enumerate(require_table_array(raw, "sprite_groups", context)):
        item_context = f"{path}: sprite_groups[{index}]"
        check_keys(
            item,
            {"number", "pointer_address", "native_table", "native_entries"},
            item_context,
        )
        number = require_int(item, "number", item_context)
        if number in sprite_groups:
            raise PackageError(f"{item_context}: duplicate sprite group 0x{number:X}")
        sprite_groups[number] = SpriteGroup(
            number,
            require_int(item, "pointer_address", item_context),
            require_str(item, "native_table", item_context),
            require_int(item, "native_entries", item_context),
        )

    songs_raw = require_table(raw, "songs", context)
    check_keys(
        songs_raw, {"native_table", "native_entries", "references"}, f"{path}: songs"
    )
    songs = EditionSongConfig(
        require_str(songs_raw, "native_table", f"{path}: songs"),
        require_int(songs_raw, "native_entries", f"{path}: songs"),
        require_int_array(songs_raw, "references", f"{path}: songs"),
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

    attack_routes: dict[tuple[int, int], tuple[int, int | None]] = {}
    for index, item in enumerate(require_table_array(raw, "attack_routes", context)):
        item_context = f"{path}: attack_routes[{index}]"
        check_keys(
            item,
            {"family", "subfamily", "hook_address", "BN6_EXPORT"},
            item_context,
        )
        family = checked_int(
            require_int(item, "family", item_context),
            0,
            0xFF,
            f"{item_context}: family",
        )
        subfamily = checked_int(
            require_int(item, "subfamily", item_context),
            0,
            0xFF,
            f"{item_context}: subfamily",
        )
        key = (family, subfamily)
        if key in attack_routes:
            raise PackageError(
                f"{item_context}: duplicate attack route 0x{family:02X}/0x{subfamily:02X}"
            )
        BN6_EXPORT = item.get("BN6_EXPORT")
        if BN6_EXPORT is not None and (
            isinstance(BN6_EXPORT, bool) or not isinstance(BN6_EXPORT, int)
        ):
            raise PackageError(f"{item_context}: BN6_EXPORT must be an integer")
        attack_routes[key] = (
            require_int(item, "hook_address", item_context),
            BN6_EXPORT,
        )

    text_raw = require_table(raw, "text", context)
    check_keys(
        text_raw,
        {"folder_edit_skip_address", "folder_edit_skip_target", "archives"},
        f"{path}: text",
    )
    archives: list[EditionTextArchive] = []
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
                "native_entries",
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
        if not ASM_SYMBOL_RE.fullmatch(symbol):
            raise PackageError(f"{item_context}: invalid assembly symbol {symbol!r}")
        source = require_str(item, "source", item_context)
        encoding = require_str(item, "encoding", item_context)
        if source not in {"names", "descriptions"}:
            raise PackageError(f"{item_context}: unknown source {source!r}")
        if encoding not in {"name", "description"}:
            raise PackageError(f"{item_context}: unknown encoding {encoding!r}")
        archives.append(
            EditionTextArchive(
                name,
                require_int(item, "region", item_context),
                source,
                require_int(item, "source_index", item_context),
                encoding,
                require_int(item, "native_entries", item_context),
                symbol,
                require_str(item, "binary", item_context),
                require_int_array(item, "references", item_context),
            )
        )
    text = EditionTextConfig(
        (
            require_int(text_raw, "folder_edit_skip_address", f"{path}: text"),
            require_int(text_raw, "folder_edit_skip_target", f"{path}: text"),
        ),
        tuple(archives),
    )
    return EditionConfig(
        object_classes, sprite_groups, songs, text, chips, attack_routes
    )


def static_config(root: Path) -> Config:
    """Load and combine the Gregar and Falzar static backend configurations."""
    resolved_root = root.resolve()
    gregar = load_edition_config(resolved_root / "config.gregar.toml", "gregar")
    falzar = load_edition_config(resolved_root / "config.falzar.toml", "falzar")
    shared = (
        ("object_classes", gregar.object_classes, falzar.object_classes),
        ("sprite_groups", gregar.sprite_groups, falzar.sprite_groups),
        (
            "song layout",
            (gregar.songs.native_table, gregar.songs.native_entries),
            (falzar.songs.native_table, falzar.songs.native_entries),
        ),
        ("chip layout", gregar.chips, falzar.chips),
    )
    for label, gregar_value, falzar_value in shared:
        if gregar_value != falzar_value:
            raise PackageError(
                f"config.gregar.toml and config.falzar.toml disagree on {label}"
            )

    if set(gregar.attack_routes) != set(falzar.attack_routes):
        raise PackageError("Gregar and Falzar configure different attack routes")
    attack_routes: dict[tuple[int, int], AttackRoute] = {}
    for key, (gregar_hook, gregar_native) in gregar.attack_routes.items():
        falzar_hook, falzar_native = falzar.attack_routes[key]
        if gregar_hook != falzar_hook:
            raise PackageError(
                "config.gregar.toml and config.falzar.toml disagree on attack route "
                f"0x{key[0]:02X}/0x{key[1]:02X}"
            )
        if (gregar_native is None) != (falzar_native is None):
            raise PackageError(
                "config.gregar.toml and config.falzar.toml must both define or both "
                f"omit BN6_EXPORT for attack route 0x{key[0]:02X}/0x{key[1]:02X}"
            )
        attack_routes[key] = AttackRoute(
            key[0], key[1], gregar_hook, gregar_native, falzar_native
        )

    falzar_archives = {archive.name: archive for archive in falzar.text.archives}
    if {archive.name for archive in gregar.text.archives} != set(falzar_archives):
        raise PackageError("Gregar and Falzar configure different text archives")
    merged_archives: list[tuple[int, TextArchive]] = []
    for gregar_archive in gregar.text.archives:
        falzar_archive = falzar_archives[gregar_archive.name]
        gregar_layout = (
            gregar_archive.name,
            gregar_archive.region,
            gregar_archive.source,
            gregar_archive.source_index,
            gregar_archive.encoding,
            gregar_archive.native_entries,
            gregar_archive.symbol,
            gregar_archive.binary,
        )
        falzar_layout = (
            falzar_archive.name,
            falzar_archive.region,
            falzar_archive.source,
            falzar_archive.source_index,
            falzar_archive.encoding,
            falzar_archive.native_entries,
            falzar_archive.symbol,
            falzar_archive.binary,
        )
        if gregar_layout != falzar_layout:
            raise PackageError(
                "config.gregar.toml and config.falzar.toml disagree on text archive "
                f"{gregar_archive.name!r}"
            )
        merged_archives.append(
            (
                gregar_archive.region,
                TextArchive(
                    gregar_archive.name,
                    gregar_archive.source,
                    gregar_archive.source_index,
                    gregar_archive.encoding,
                    gregar_archive.native_entries,
                    gregar_archive.symbol,
                    gregar_archive.binary,
                    gregar_archive.references,
                    falzar_archive.references,
                ),
            )
        )
    groups = tuple(
        TextArchiveGroup(
            tuple(
                archive
                for archive_region, archive in merged_archives
                if archive_region == region
            )
        )
        for region in sorted({region for region, _ in merged_archives})
    )
    return Config(
        resolved_root,
        gregar.object_classes,
        gregar.sprite_groups,
        SongConfig(
            gregar.songs.native_table,
            gregar.songs.native_entries,
            gregar.songs.references,
            falzar.songs.references,
        ),
        TextConfig(
            gregar.text.folder_edit_skip,
            falzar.text.folder_edit_skip,
            groups,
        ),
        gregar.chips,
        attack_routes,
    )


CHIP_FIELD_LAYOUT: dict[str, tuple[int, str]] = {
    "codes": (0x00, "bytes"),
    "attack_element": (0x04, "byte"),
    "rarity": (0x05, "byte"),
    "element": (0x06, "byte"),
    "class": (0x07, "byte"),
    "mb": (0x08, "byte"),
    "behavior.effect_flags": (0x09, "byte"),
    "behavior.counter_settings": (0x0A, "byte"),
    "behavior.family": (0x0B, "byte"),
    "behavior.subfamily": (0x0C, "byte"),
    "behavior.dark_soul_usage": (0x0D, "byte"),
    "behavior.unknown_0e": (0x0E, "byte"),
    "behavior.lock_on": (0x0F, "byte"),
    "behavior.parameters": (0x10, "bytes"),
    "behavior.delay": (0x14, "byte"),
    "library.number": (0x15, "byte"),
    "library.flags": (0x16, "byte"),
    "library.lock_on_type": (0x17, "byte"),
    # 0x18 is compiler-owned: reorder_chip_sort.py regenerates it from names.
    "power": (0x1A, "halfword"),
    "library.sort_order": (0x1C, "halfword"),
    "library.gate_usage": (0x1E, "byte"),
    "library.dark_chip_id": (0x1F, "byte"),
    "artwork.icon": (0x20, "word"),
    "artwork.image": (0x24, "word"),
    "artwork.palette": (0x28, "word"),
}

CHIP_TOP_LEVEL_FIELDS = {
    "codes",
    "attack_element",
    "rarity",
    "element",
    "class",
    "mb",
    "power",
    "behavior",
    "library",
    "artwork",
}
CHIP_BEHAVIOR_FIELDS = {
    "effect_flags",
    "counter_settings",
    "family",
    "subfamily",
    "dark_soul_usage",
    "unknown_0e",
    "lock_on",
    "parameters",
    "delay",
}
CHIP_LIBRARY_FIELDS = {
    "number",
    "flags",
    "lock_on_type",
    "sort_order",
    "gate_usage",
    "dark_chip_id",
}
CHIP_ARTWORK_FIELDS = {"icon", "image", "palette"}


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


def chip_text_archive(
    config: Config,
    source: str,
    chip_id: int,
    context: str,
) -> tuple[str, int]:
    """Resolve a semantic chip-text field through the configured archives."""
    source_index, entry_index = divmod(chip_id, 0x100)
    matches = [
        archive
        for group in config.text.groups
        for archive in group.archives
        if archive.source == source and archive.source_index == source_index
    ]
    if len(matches) != 1 or entry_index >= matches[0].native_entries:
        raise PackageError(
            f"{context}: chip 0x{chip_id:03X} has no configured {source} archive"
        )
    return matches[0].name, entry_index


def parse_chip_record(
    table: dict[str, Any], context: str
) -> tuple[tuple[str, ChipValue], ...]:
    check_keys(table, CHIP_TOP_LEVEL_FIELDS, context)
    fields: dict[str, ChipValue] = {}

    if "codes" in table:
        raw_codes = table["codes"]
        if (
            not isinstance(raw_codes, list)
            or not 1 <= len(raw_codes) <= 4
            or not all(
                isinstance(code, str) and code in CHIP_CODE_VALUES for code in raw_codes
            )
        ):
            raise PackageError(
                f"{context}: codes must contain one to four values from A-Z or '*'"
            )
        if len(set(raw_codes)) != len(raw_codes):
            raise PackageError(f"{context}: codes must not contain duplicates")
        fields["codes"] = tuple(
            [CHIP_CODE_VALUES[code] for code in raw_codes]
            + [0xFF] * (4 - len(raw_codes))
        )

    for key in ("attack_element", "rarity", "mb"):
        if key in table:
            fields[key] = checked_int(table[key], 0, 0xFF, f"{context}: {key}")
    if "power" in table:
        fields["power"] = checked_int(table["power"], 0, 0xFFFF, f"{context}: power")

    if "element" in table:
        element = table["element"]
        if not isinstance(element, str) or element not in CHIP_ELEMENTS:
            raise PackageError(
                f"{context}: element must be one of {', '.join(sorted(CHIP_ELEMENTS))}"
            )
        fields["element"] = CHIP_ELEMENTS[element]
    if "class" in table:
        chip_class = table["class"]
        if not isinstance(chip_class, str) or chip_class not in CHIP_CLASSES:
            raise PackageError(
                f"{context}: class must be one of {', '.join(sorted(CHIP_CLASSES))}"
            )
        fields["class"] = CHIP_CLASSES[chip_class]

    behavior = table.get("behavior", {})
    if not isinstance(behavior, dict):
        raise PackageError(f"{context}: behavior must be a table")
    check_keys(behavior, CHIP_BEHAVIOR_FIELDS, f"{context}: behavior")
    for key in CHIP_BEHAVIOR_FIELDS - {"parameters"}:
        if key in behavior:
            fields[f"behavior.{key}"] = checked_int(
                behavior[key], 0, 0xFF, f"{context}: behavior.{key}"
            )
    if "parameters" in behavior:
        parameters = behavior["parameters"]
        if not isinstance(parameters, list) or len(parameters) != 4:
            raise PackageError(
                f"{context}: behavior.parameters must contain exactly four bytes"
            )
        fields["behavior.parameters"] = tuple(
            checked_int(value, 0, 0xFF, f"{context}: behavior.parameters[{index}]")
            for index, value in enumerate(parameters)
        )

    library = table.get("library", {})
    if not isinstance(library, dict):
        raise PackageError(f"{context}: library must be a table")
    check_keys(library, CHIP_LIBRARY_FIELDS, f"{context}: library")
    for key in CHIP_LIBRARY_FIELDS:
        if key not in library:
            continue
        value = library[key]
        if key == "dark_chip_id" and value == "none":
            normalized = 0xFF
        else:
            maximum = 0xFFFF if key == "sort_order" else 0xFF
            normalized = checked_int(value, 0, maximum, f"{context}: library.{key}")
        fields[f"library.{key}"] = normalized

    artwork = table.get("artwork", {})
    if not isinstance(artwork, dict):
        raise PackageError(f"{context}: artwork must be a table")
    check_keys(artwork, CHIP_ARTWORK_FIELDS, f"{context}: artwork")
    for key in CHIP_ARTWORK_FIELDS:
        if key not in artwork:
            continue
        value = artwork[key]
        if isinstance(value, bool) or not (
            isinstance(value, int)
            and 0 <= value <= 0xFFFFFFFF
            or isinstance(value, str)
            and ASM_SYMBOL_RE.fullmatch(value)
        ):
            raise PackageError(
                f"{context}: artwork.{key} must be a 32-bit address or assembly symbol"
            )
        fields[f"artwork.{key}"] = value

    return tuple(sorted(fields.items(), key=lambda item: CHIP_FIELD_LAYOUT[item[0]][0]))


def load_package(source_path: Path, config: Config) -> Package:
    name = source_path.stem
    check_name(name, str(source_path))
    definitions = source_path.with_suffix(".defs.toml")
    if definitions.is_file():
        try:
            raw_definitions = tomllib.loads(definitions.read_text())
        except (OSError, tomllib.TOMLDecodeError) as exc:
            raise PackageError(f"cannot read {definitions}: {exc}") from exc
    else:
        raw_definitions = {}
    check_keys(raw_definitions, {"chips", "text"}, str(definitions))
    chip_values = raw_definitions.get("chips", {})
    text_values = raw_definitions.get("text", {})
    if not isinstance(chip_values, dict):
        raise PackageError(f"{definitions}: chips must be a table")
    if not isinstance(text_values, dict):
        raise PackageError(f"{definitions}: text must be a table")

    text: list[TextResource] = []
    chips: list[ChipResource] = []
    for chip_key, item in chip_values.items():
        context = f"{definitions}:{chip_key}"
        try:
            chip_id = int(chip_key, 0)
        except ValueError as exc:
            raise PackageError(f"{context}: chip ID must be an integer string") from exc
        if not 0 <= chip_id < config.chips.record_count:
            raise PackageError(
                f"{context}: chip ID must be between 0x000 and "
                f"0x{config.chips.record_count - 1:03X}"
            )
        if not isinstance(item, dict):
            raise PackageError(f"{context}: configuration must be a table")
        check_keys(
            item,
            CHIP_TOP_LEVEL_FIELDS | {"name", "description", "gregar", "falzar"},
            context,
        )

        common_table = {
            key: value for key, value in item.items() if key in CHIP_TOP_LEVEL_FIELDS
        }
        common = parse_chip_record(common_table, context)
        variants: dict[str, tuple[tuple[str, ChipValue], ...]] = {}
        for version in ("gregar", "falzar"):
            version_table = item.get(version, {})
            if not isinstance(version_table, dict):
                raise PackageError(f"{context}: {version} must be a table")
            variants[version] = parse_chip_record(
                version_table, f"{context}: {version}"
            )

        if "name" in item:
            display_name = item["name"]
            if not isinstance(display_name, str) or not display_name:
                raise PackageError(f"{context}: name must be a non-empty string")
            archive_name, entry_index = chip_text_archive(
                config, "names", chip_id, context
            )
            text.append(TextResource(name, archive_name, entry_index, display_name))
        if "description" in item:
            description = item["description"]
            if (
                not isinstance(description, list)
                or not description
                or not all(isinstance(line, str) and line for line in description)
            ):
                raise PackageError(
                    f"{context}: description must be a non-empty string array"
                )
            archive_name, entry_index = chip_text_archive(
                config, "descriptions", chip_id, context
            )
            text.append(
                TextResource(name, archive_name, entry_index, tuple(description))
            )

        if (
            not common
            and not variants["gregar"]
            and not variants["falzar"]
            and not ("name" in item or "description" in item)
        ):
            raise PackageError(
                f"{context}: configuration must declare at least one chip detail"
            )
        chips.append(
            ChipResource(
                name,
                chip_id,
                common,
                variants["gregar"],
                variants["falzar"],
            )
        )

    archives = {
        archive.name: archive
        for group in config.text.groups
        for archive in group.archives
    }
    for archive_name, entries in text_values.items():
        context = f"{definitions}:text.{archive_name}"
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
            if isinstance(value, str) and value:
                normalized: str | tuple[str, ...] = value
            elif (
                isinstance(value, list)
                and value
                and all(isinstance(line, str) and line for line in value)
            ):
                normalized = tuple(value)
            else:
                raise PackageError(
                    f"{entry_context}: replacement must be a non-empty string "
                    "or string array"
                )
            text.append(
                TextResource(name, archive_name, entry_index, normalized)
            )

    return Package(
        name,
        source_path.relative_to(config.root).as_posix(),
        definitions if definitions.is_file() else source_path,
        (),
        (),
        (),
        (),
        tuple(text),
        tuple(chips),
        None,
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
        if not isinstance(symbols, list) or not all(isinstance(item, str) for item in symbols):
            raise PackageError(f"{path}: invalid symbols for {entry['name']}")
        result[entry["name"]] = symbols
    return result


def check_resource_label(package: str, label: str, suffix: str) -> None:
    prefix = pascal_case(package)
    if not label.startswith(prefix) or not label.endswith(suffix):
        raise PackageError(
            f"{package}: {label} must use the {prefix}<name>{suffix} convention"
        )
    middle = label[len(prefix):-len(suffix)]
    if not middle:
        raise PackageError(f"{package}: empty resource name in {label}")


def check_snake_resource_label(package: str, label: str, suffix: str) -> None:
    prefix = package.replace("-", "_") + "_"
    if not label.startswith(prefix) or not label.endswith(suffix):
        raise PackageError(
            f"{package}: {label} must use the {prefix}<name>{suffix} convention"
        )
    middle = label[len(prefix):-len(suffix)]
    if not middle or SNAKE_CASE_RE.fullmatch(label) is None:
        raise PackageError(f"{package}: invalid snake_case resource name {label}")


def apply_metadata(package: Package, symbols: list[str]) -> Package:
    includes: list[str] = []
    objects: list[ObjectResource] = []
    sprites: list[SpriteResource] = []
    songs: list[SongResource] = []
    attack: AttackResource | None = None
    patches: list[PointerPatch] = []
    for symbol in symbols:
        prefix = "__bn6_meta__"
        if not symbol.startswith(prefix):
            raise PackageError(f"{package.name}: invalid metadata symbol {symbol}")
        parts = symbol[len(prefix):].split("__")
        kind = parts[0]
        if kind == "include" and len(parts) == 2:
            check_name(parts[1], f"{package.name}: include")
            includes.append(parts[1])
        elif kind == "object" and len(parts) == 3:
            object_class = int(parts[1], 0)
            check_snake_resource_label(package.name, parts[2], "_main")
            objects.append(ObjectResource(object_class, parts[2]))
        elif kind == "sprite" and len(parts) == 2:
            check_snake_resource_label(package.name, parts[1], "_sprite")
            sprites.append(SpriteResource(parts[1]))
        elif kind == "song" and len(parts) == 2:
            check_resource_label(package.name, parts[1], "Song")
            songs.append(SongResource(parts[1]))
        elif kind == "attack" and len(parts) == 3:
            if attack is not None:
                raise PackageError(f"{package.name}: duplicate BN6_ATTACK declaration")
            check_snake_resource_label(package.name, parts[2], "_main")
            attack = AttackResource(package.name, int(parts[1], 0), parts[2])
        elif kind == "pointer" and len(parts) == 4:
            address = checked_int(int(parts[2], 0), 0, 0xFFFFFFFF, symbol)
            if ASM_SYMBOL_RE.fullmatch(parts[3]) is None:
                raise PackageError(f"{package.name}: invalid patch symbol {parts[3]}")
            patches.append(PointerPatch(
                parts[3],
                address if parts[1] != "falzar" else None,
                address if parts[1] != "gregar" else None,
            ))
        else:
            raise PackageError(f"{package.name}: invalid metadata symbol {symbol}")
    return replace(
        package,
        includes=tuple(includes), objects=tuple(objects), sprites=tuple(sprites),
        songs=tuple(songs), attack=attack, pointer_patches=tuple(patches),
    )


def discover_packages(config: Config, metadata: dict[str, list[str]]) -> list[Package]:
    sources = sorted(
        path.resolve()
        for path in config.root.glob("src/*.c")
        if path.name not in RUNTIME_SOURCE_NAMES
    )
    if not sources:
        raise PackageError("no src/*.c implementations were found")
    packages = [load_package(path, config) for path in sources]
    unknown = sorted(set(metadata) - {package.name for package in packages})
    if unknown:
        raise PackageError(f"C metadata has no implementation: {', '.join(unknown)}")
    missing = sorted({package.name for package in packages} - set(metadata))
    if missing:
        raise PackageError(
            "C metadata is missing implementation(s): " + ", ".join(missing)
        )
    by_name = {
        package.name: apply_metadata(package, metadata[package.name])
        for package in packages
    }
    ordered: list[Package] = []
    visiting: list[str] = []

    def visit(name: str) -> None:
        if name in visiting:
            cycle = visiting[visiting.index(name):] + [name]
            raise PackageError("source include cycle: " + " -> ".join(cycle))
        if any(item.name == name for item in ordered):
            return
        if name not in by_name:
            raise PackageError(f"included implementation does not exist: {name}")
        visiting.append(name)
        for included in by_name[name].includes:
            visit(included)
        visiting.pop()
        ordered.append(by_name[name])

    for name in sorted(by_name):
        visit(name)
    return ordered


def validate_and_allocate(config: Config, packages: list[Package]) -> Allocations:
    objects = [item for package in packages for item in package.objects]
    sprites = [item for package in packages for item in package.sprites]
    songs = [item for package in packages for item in package.songs]
    text = [item for package in packages for item in package.text]
    chips = [item for package in packages for item in package.chips]

    for item in objects:
        if item.object_class not in config.object_classes:
            raise PackageError(
                f"{item.main}: unknown object class {item.object_class}"
            )
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
        if item.chip_id in chip_owners:
            raise PackageError(
                f"chip 0x{item.chip_id:03X} is declared by both "
                f"{chip_owners[item.chip_id]} and {item.package}"
            )
        chip_owners[item.chip_id] = item.package

    resolve_attack_routes(config, packages)

    object_allocations: dict[int, dict[str, int]] = {}
    for number in config.object_classes:
        names = [item.main for item in objects if item.object_class == number]
        if len(names) > 0xFF:
            raise PackageError(
                f"object class {number} has more than 255 package objects"
            )
        object_allocations[number] = {
            name: index for index, name in enumerate(names, 1)
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
        item.archive: SONG_PLAYER_FIRST + index
        for index, item in enumerate(songs)
    }
    return Allocations(
        object_allocations, sprite_allocations, song_allocations, song_players
    )


def resolve_attack_routes(
    config: Config,
    packages: list[Package],
) -> dict[tuple[int, int], list[tuple[AttackResource, int]]]:
    """Resolve package attack entries through their semantic chip records."""
    resolved: dict[tuple[int, int], list[tuple[AttackResource, int]]] = {}
    chips_by_id = {
        chip.chip_id: chip for package in packages for chip in package.chips
    }
    for package in packages:
        if package.attack is None:
            continue
        chip = chips_by_id.get(package.attack.chip_id)
        if chip is None:
            raise PackageError(
                f"{package.source}: BN6_ATTACK refers to chip "
                f"0x{package.attack.chip_id:03X}, but no src/*.defs.toml declares it"
            )
        signatures: set[tuple[int, int, int]] = set()
        fields = dict(chip.common)
        required = (
            "behavior.family",
            "behavior.subfamily",
            "behavior.counter_settings",
        )
        if not all(field in fields for field in required):
            raise PackageError(
                f"{package.definitions}: BN6_ATTACK chip 0x{chip.chip_id:03X} requires "
                "family, subfamily, and counter_settings"
            )
        for variant in (chip.gregar, chip.falzar):
            effective = fields | dict(variant)
            values = tuple(effective[field] for field in required)
            if not all(isinstance(value, int) for value in values):
                raise AssertionError("chip routing fields must be integers")
            signatures.add((values[0], values[1], values[2]))
        if len(signatures) != 1:
            raise PackageError(
                f"{package.definitions}: BN6_ATTACK chip must use one "
                "family/subfamily/counter_settings signature in both editions"
            )
        family, subfamily, marker = next(iter(signatures))
        key = (family, subfamily)
        if key not in config.attack_routes:
            raise PackageError(
                f"{package.definitions}: no configured attack route for "
                f"0x{family:02X}/0x{subfamily:02X}"
            )
        resolved.setdefault(key, []).append((package.attack, marker))

    for key, implementations in resolved.items():
        markers: dict[int, str] = {}
        for attack, marker in implementations:
            if marker in markers:
                raise PackageError(
                    f"attack route 0x{key[0]:02X}/0x{key[1]:02X} marker "
                    f"0x{marker:02X} is declared by both {markers[marker]} and {attack.package}"
                )
            markers[marker] = attack.package
        route = config.attack_routes[key]
        if len(implementations) > 1 and (
            route.gregar_BN6_EXPORT is None or route.falzar_BN6_EXPORT is None
        ):
            raise PackageError(
                f"shared attack route 0x{key[0]:02X}/0x{key[1]:02X} needs a "
                "native entry for both editions"
            )
    return resolved


def generate_linker_values(packages: list[Package], allocations: Allocations) -> str:
    """Define C-visible absolute symbols after deterministic allocation."""
    lines = [
        "/* Generated by compile_registry.py from C object metadata. */",
        "/* Absolute symbols are selectors, not addresses in the ROM. */",
    ]
    for package in packages:
        for item in package.objects:
            value = allocations.objects[item.object_class][item.main]
            lines.append(f"__bn6_object_kind_{item.main} = 0x{value:X};")
        for item in package.sprites:
            group, resource_id = allocations.sprites[item.archive]
            lines.append(f"__bn6_sprite_group_{item.archive} = 0x{group:X};")
            lines.append(f"__bn6_sprite_id_{item.archive} = 0x{resource_id:X};")
        for item in package.songs:
            player = allocations.song_players[item.archive]
            song_id = allocations.songs[item.archive]
            lines.append(f"__bn6_song_group_{item.archive} = 0x{player:X};")
            lines.append(f"__bn6_song_id_{item.archive} = 0x{song_id:X};")
    return "\n".join(lines) + "\n"


def emit_chip_field(
    config: Config, chip_id: int, field: str, value: ChipValue
) -> list[str]:
    offset, kind = CHIP_FIELD_LAYOUT[field]
    address = config.chips.table_address + chip_id * config.chips.record_size + offset
    if kind == "bytes":
        assert isinstance(value, tuple)
        rendered = ",".join(f"0x{item:02X}" for item in value)
        directive = f".db {rendered}"
    elif kind == "byte":
        assert isinstance(value, int)
        directive = f".db 0x{value:02X}"
    elif kind == "halfword":
        assert isinstance(value, int)
        directive = f".dh 0x{value:04X}"
    elif kind == "word":
        assert isinstance(value, (int, str))
        rendered = f"0x{value:08X}" if isinstance(value, int) else value
        directive = f".dw {rendered}"
    else:
        raise AssertionError(f"unknown chip field kind: {kind}")
    return [f".org 0x{address:08X}", f"    {directive} // {field}"]


def emit_chip_records(config: Config, packages: list[Package]) -> list[str]:
    lines = ["// Semantic chip-record patches declared by packages."]
    for package in packages:
        for chip in package.chips:
            if not chip.common and not chip.gregar and not chip.falzar:
                continue
            lines.extend(["", f"// {chip.package}: chip 0x{chip.chip_id:03X}"])
            for field, value in chip.common:
                lines.extend(emit_chip_field(config, chip.chip_id, field, value))
            if chip.gregar or chip.falzar:
                lines.append(".if FALZAR")
                for field, value in chip.falzar:
                    lines.extend(emit_chip_field(config, chip.chip_id, field, value))
                lines.append(".else")
                for field, value in chip.gregar:
                    lines.extend(emit_chip_field(config, chip.chip_id, field, value))
                lines.append(".endif")
    return lines


def emit_attack_routes(config: Config, packages: list[Package]) -> list[str]:
    """Install direct entries and discriminator routers for chip attack families."""
    resolved = resolve_attack_routes(config, packages)
    lines = ["// Compiler-owned chip attack-family routes."]
    shared: list[tuple[AttackRoute, list[tuple[AttackResource, int]]]] = []
    for key, implementations in sorted(resolved.items()):
        route = config.attack_routes[key]
        lines.extend(["", f"// family 0x{key[0]:02X}, subfamily 0x{key[1]:02X}"])
        lines.append(f".org 0x{route.hook_address:08X}")
        needs_router = (
            len(implementations) > 1
            or route.gregar_BN6_EXPORT is not None
            or route.falzar_BN6_EXPORT is not None
        )
        if not needs_router:
            lines.append(f".dw {implementations[0][0].main} + 1")
        else:
            label = f"AttackRoute{key[0]:02X}_{key[1]:02X}"
            lines.append(f".dw {label} + 1")
            shared.append((route, implementations))

    if not shared:
        return lines
    lines.extend(["", ".autoregion", ".align 4", "AttackRoutesCodeStart:"])
    for route, implementations in shared:
        label = f"AttackRoute{route.family:02X}_{route.subfamily:02X}"
        compared = sorted(implementations, key=lambda item: (item[1], item[0].package))
        lines.extend(
            [
                "",
                f"{label}:",
                "    push {r0}",
                "    mov r0,r6",
                "    lsr r0,r0,16",
            ]
        )
        for attack, marker in compared:
            lines.extend(
                [
                    f"    cmp r0,0x{marker:02X}",
                    f"    beq {label}_{symbol_case(attack.package)}",
                ]
            )
        lines.append("    pop {r0}")
        if route.gregar_BN6_EXPORT is None or route.falzar_BN6_EXPORT is None:
            raise PackageError(
                f"attack route 0x{route.family:02X}/0x{route.subfamily:02X} "
                "must configure BN6_EXPORT for both editions"
            )
        lines.extend(
            [
                ".if FALZAR",
                f"    ldr r3,=0x{route.falzar_BN6_EXPORT:08X} + 1",
                ".else",
                f"    ldr r3,=0x{route.gregar_BN6_EXPORT:08X} + 1",
                ".endif",
                "    bx r3",
            ]
        )
        for attack, _ in compared:
            lines.extend(
                [
                    f"{label}_{symbol_case(attack.package)}:",
                    "    pop {r0}",
                    f"    ldr r3,={attack.main} + 1",
                    "    bx r3",
                ]
            )
        lines.append("    .pool")
    lines.extend(["", "AttackRoutesCodeEnd:", ".endautoregion"])
    return lines


def emit_object_tables(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    lines = [
        "// Project-owned object dispatch hubs.",
    ]
    for number, object_class in sorted(config.object_classes.items()):
        lines.extend(
            [
                f".org 0x{object_class.dispatch_entry:08X}",
                f".dw CustomType{number}Main + 1",
            ]
        )
    lines.extend(["", ".autoregion", ".align 4", "ObjectTypesCodeStart:"])
    for number in sorted(config.object_classes):
        lines.extend(
            [
                "",
                f"CustomType{number}Main:",
                "    ldr r0,[r5,CUSTOM_OBJECT_KIND]",
                "    sub r0,1",
                f"    cmp r0,{len(allocations.objects[number])}",
                "    bhs CustomObjectInvalid",
                "    lsl r0,r0,2",
                f"    ldr r1,=CustomType{number}Table",
                "    ldr r0,[r1,r0]",
                "    bx r0",
            ]
        )
    lines.extend(
        [
            "",
            "CustomObjectInvalid:",
            "    push {lr}",
            "    EngineCall OBJECT_FREE",
            "    pop {pc}",
        ]
    )
    all_objects = [item for package in packages for item in package.objects]
    for number in sorted(config.object_classes):
        namespace = allocations.objects[number]
        active = {
            item.main: item
            for item in all_objects
            if item.object_class == number
        }
        reverse = {resource_id: name for name, resource_id in namespace.items()}
        lines.extend(["", ".align 4", f"CustomType{number}Table:"])
        for resource_id in range(1, len(namespace) + 1):
            name = reverse[resource_id]
            lines.append(f"    .dw {active[name].main} + 1 // {name} ({resource_id})")
        lines.append(f"CustomType{number}TableEnd:")
    lines.extend(["    .pool", "", "ObjectTypesCodeEnd:", ".endautoregion"])
    return lines


def emit_pointer_patches(packages: list[Package]) -> list[str]:
    lines = ["// Package-declared fixed pointer patches."]
    for package in packages:
        for patch in package.pointer_patches:
            lines.append(f"// {package.name}: {patch.symbol}")
            if patch.gregar_address == patch.falzar_address:
                assert patch.gregar_address is not None
                lines.append(f".org 0x{patch.gregar_address:08X}")
            elif patch.gregar_address is None:
                assert patch.falzar_address is not None
                lines.extend(
                    (
                        ".if FALZAR",
                        f"    .org 0x{patch.falzar_address:08X}",
                        f"    .dw {patch.symbol}",
                        ".endif",
                    )
                )
                continue
            elif patch.falzar_address is None:
                lines.extend(
                    (
                        ".if !FALZAR",
                        f"    .org 0x{patch.gregar_address:08X}",
                        f"    .dw {patch.symbol}",
                        ".endif",
                    )
                )
                continue
            else:
                lines.extend(
                    (
                        ".if FALZAR",
                        f"    .org 0x{patch.falzar_address:08X}",
                        ".else",
                        f"    .org 0x{patch.gregar_address:08X}",
                        ".endif",
                    )
                )
            lines.append(f".dw {patch.symbol}")
    return lines


def emit_sprite_tables(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    lines = ["// Relocated native sprite groups with package-owned archives appended."]
    all_sprites = [item for package in packages for item in package.sprites]
    for number, group in sorted(config.sprite_groups.items()):
        table = f"ImportedSpriteGroup{number:02X}Table"
        lines.extend(
            [f".org 0x{group.pointer_address:08X}", f".dw {table}"]
        )
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
        table = f"ImportedSpriteGroup{number:02X}Table"
        lines.extend(
            [
                "",
                ".autoregion",
                ".align 4",
                f"{table}:",
                f'    .incbin "{group.native_table}"',
            ]
        )
        for resource_id in sorted(reverse):
            name = reverse[resource_id]
            item = active[name]
            lines.append(f"    .dw {item.archive} // {name} (0x{resource_id:02X})")
        lines.extend([f"{table}End:", ".endautoregion"])
    return lines


def emit_song_table(
    config: Config, packages: list[Package], allocations: Allocations
) -> list[str]:
    lines = [
        "// Relocated native song table with package-owned songs appended.",
        ".if FALZAR",
    ]
    for address in config.songs.falzar_references:
        lines.extend(
            [f"    .org 0x{address:08X}", "    .dw RelocatedSongTable"]
        )
    lines.append(".else")
    for address in config.songs.gregar_references:
        lines.extend(
            [f"    .org 0x{address:08X}", "    .dw RelocatedSongTable"]
        )
    lines.extend(
        [
            ".endif",
            "",
            ".autoregion",
            ".align 4",
            "RelocatedSongTable:",
            f'    .incbin "{config.songs.native_table}"',
        ]
    )
    active = {
        item.archive: item for package in packages for item in package.songs
    }
    reverse = {resource_id: name for name, resource_id in allocations.songs.items()}
    for resource_id in range(
        config.songs.native_entries,
        config.songs.native_entries + len(allocations.songs),
    ):
        name = reverse[resource_id]
        item = active[name]
        lines.extend(
            [
                f"    .dw {item.archive} // {name} (0x{resource_id:04X})",
                f"    .dh 0x{allocations.song_players[name]:04X},0x{allocations.song_players[name]:04X}",
            ]
        )
    lines.extend(["RelocatedSongTableEnd:", ".endautoregion"])
    return lines


def emit_text_archives(config: Config) -> list[str]:
    """Emit the fixed BN6 archive installer fed by package-generated text binaries."""
    archives = [archive for group in config.text.groups for archive in group.archives]
    falzar_address, falzar_target = config.text.falzar_folder_edit_skip
    gregar_address, gregar_target = config.text.gregar_folder_edit_skip
    lines = [
        "// Complete chip-text archives with changes declared by packages.",
        ".if FALZAR",
        f"    .org 0x{falzar_address:08X}",
        f"    b 0x{falzar_target:08X}",
    ]
    for archive in archives:
        for address in archive.falzar_references:
            lines.extend(
                [f"    .org 0x{address:08X}", f"    .dw {archive.symbol}"]
            )
    lines.extend(
        [
            ".else",
            f"    .org 0x{gregar_address:08X}",
            f"    b 0x{gregar_target:08X}",
        ]
    )
    for archive in archives:
        for address in archive.gregar_references:
            lines.extend(
                [f"    .org 0x{address:08X}", f"    .dw {archive.symbol}"]
            )
    lines.append(".endif")
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
            "value": list(item.value) if isinstance(item.value, tuple) else item.value,
        }
        for package in packages
        for item in package.text
    ]
    return json.dumps({"archives": archives, "entries": entries}, indent=2) + "\n"


def generate(config: Config, packages: list[Package], allocations: Allocations) -> str:
    sections: list[list[str]] = [
        [
            "// Generated by compile_registry.py from C object metadata and src/*.defs.toml.",
            "// Do not edit this file or assign registry IDs by hand.",
            "// IDs follow dependency order, sorted source paths, and ELF symbol order.",
            "",
        ],
        emit_pointer_patches(packages),
        [""],
        emit_attack_routes(config, packages),
        [""],
        emit_chip_records(config, packages),
        [""],
        emit_object_tables(config, packages, allocations),
        [""],
        emit_sprite_tables(config, packages, allocations),
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
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--text-output", type=Path)
    parser.add_argument("--metadata", type=Path)
    parser.add_argument("--linker-output", type=Path)
    parser.add_argument(
        "--check", action="store_true", help="fail if the generated output is stale"
    )
    args = parser.parse_args()

    try:
        config = static_config(args.root)
        metadata_path = (
            args.metadata.resolve()
            if args.metadata
            else config.root / "build/registry-metadata.generated.json"
        )
        packages = discover_packages(config, load_metadata(metadata_path))
        output_path = (
            args.output.resolve()
            if args.output
            else config.root / "build/registry.generated.asm"
        )
        text_output_path = (
            args.text_output.resolve()
            if args.text_output
            else config.root / "build/text-replacements.generated.json"
        )
        linker_output_path = (
            args.linker_output.resolve()
            if args.linker_output
            else config.root / "build/registry-values.generated.ld"
        )
        allocations = validate_and_allocate(config, packages)
        output_text = generate(config, packages, allocations)
        text_manifest = generate_text_manifest(config, packages)
        linker_values = generate_linker_values(packages, allocations)
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
            if stale:
                raise PackageError(
                    "generated package file is stale: " + ", ".join(stale)
                )
        else:
            write_if_changed(output_path, output_text)
            write_if_changed(text_output_path, text_manifest)
            write_if_changed(linker_output_path, linker_values)
    except PackageError as exc:
        print(f"registry compiler: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
