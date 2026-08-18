#!/usr/bin/env python3
"""Rebuild BN6's title layer with a transparent full-screen PNG overlay."""

from __future__ import annotations

import argparse
import struct
import zlib
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class TitleConfig:
    stream_offset: int
    decoded_size: int
    encoded_capacity: int
    tilemap_offset: int
    colors: dict[str, int]
    gradient: tuple[int, ...]


CONFIGS = {
    "gregar": TitleConfig(
        stream_offset=0x7F3040,
        decoded_size=0x8E84,
        encoded_capacity=0x4CB9,
        tilemap_offset=0x7F7CFC,
        colors={
            "black": 0x13,
            "gray_dark": 0x18,
            "gray": 0x1B,
            "brown": 0x49,
            "orange_dark": 0x3E,
            "orange": 0x3D,
            "gold": 0x45,
            "yellow": 0x44,
            "light": 0x30,
            "cream": 0x2F,
            "white": 0x2E,
        },
        gradient=(
            0x3E, 0x3D, 0x3C, 0x41, 0x42, 0x45, 0x44,
            0x46, 0x33, 0x31, 0x30, 0x2F, 0x2E,
        ),
    ),
    "falzar": TitleConfig(
        stream_offset=0x7F4394,
        decoded_size=0x8D84,
        encoded_capacity=0x4B1C,
        tilemap_offset=0x7F8EB0,
        colors={
            "black": 0x00,
            "gray_dark": 0x0C,
            "gray": 0x0A,
            "brown": 0x14,
            "orange_dark": 0x37,
            "orange": 0x36,
            "gold": 0x2F,
            "yellow": 0x2E,
            "light": 0x4F,
            "cream": 0x4A,
            "white": 0x49,
        },
        gradient=(
            0x37, 0x36, 0x34, 0x33, 0x31, 0x30, 0x2F,
            0x2E, 0x2A, 0x3D, 0x4F, 0x4A, 0x49,
        ),
    ),
}

OVERLAY_PATH = Path(__file__).resolve().parent / "assets" / "title_screen_overlay.png"
REFERENCE_GRADIENT = (
    (230, 131, 32),
    (230, 148, 41),
    (230, 164, 41),
    (246, 172, 32),
    (255, 180, 24),
    (255, 197, 24),
    (255, 213, 16),
    (255, 222, 57),
    (255, 230, 98),
    (255, 222, 139),
    (255, 238, 164),
    (255, 246, 197),
    (255, 255, 222),
)


def read_rgba_png(path: Path) -> tuple[int, int, bytes]:
    """Read the fixed 8-bit, non-interlaced RGBA overlay using stdlib only."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"not a PNG: {path}")
    cursor = 8
    width = height = 0
    compressed = bytearray()
    while cursor < len(data):
        length = int.from_bytes(data[cursor : cursor + 4], "big")
        chunk_type = data[cursor + 4 : cursor + 8]
        chunk = data[cursor + 8 : cursor + 8 + length]
        cursor += 12 + length
        if chunk_type == b"IHDR":
            width, height, depth, color_type, compression, filtering, interlace = (
                struct.unpack(">IIBBBBB", chunk)
            )
            if (depth, color_type, compression, filtering, interlace) != (8, 6, 0, 0, 0):
                raise ValueError(f"overlay must be non-interlaced 8-bit RGBA: {path}")
        elif chunk_type == b"IDAT":
            compressed.extend(chunk)
        elif chunk_type == b"IEND":
            break
    if not width or not height:
        raise ValueError(f"PNG has no IHDR: {path}")

    raw = zlib.decompress(compressed)
    stride = width * 4
    expected = height * (stride + 1)
    if len(raw) != expected:
        raise ValueError(f"unexpected PNG data size {len(raw)}; expected {expected}")

    def paeth(left: int, above: int, upper_left: int) -> int:
        estimate = left + above - upper_left
        left_distance = abs(estimate - left)
        above_distance = abs(estimate - above)
        diagonal_distance = abs(estimate - upper_left)
        if left_distance <= above_distance and left_distance <= diagonal_distance:
            return left
        if above_distance <= diagonal_distance:
            return above
        return upper_left

    output = bytearray()
    previous = bytearray(stride)
    source = 0
    for _ in range(height):
        filter_type = raw[source]
        source += 1
        scanline = bytearray(raw[source : source + stride])
        source += stride
        for index in range(stride):
            left = scanline[index - 4] if index >= 4 else 0
            above = previous[index]
            upper_left = previous[index - 4] if index >= 4 else 0
            if filter_type == 1:
                scanline[index] = (scanline[index] + left) & 0xFF
            elif filter_type == 2:
                scanline[index] = (scanline[index] + above) & 0xFF
            elif filter_type == 3:
                scanline[index] = (scanline[index] + ((left + above) // 2)) & 0xFF
            elif filter_type == 4:
                scanline[index] = (
                    scanline[index] + paeth(left, above, upper_left)
                ) & 0xFF
            elif filter_type != 0:
                raise ValueError(f"unsupported PNG filter {filter_type}")
        output.extend(scanline)
        previous = scanline
    return width, height, bytes(output)


def decompress_lz77(data: bytes, offset: int = 0) -> tuple[bytes, int]:
    if data[offset] != 0x10:
        raise ValueError(f"expected GBA LZ77 stream at {offset:#x}")
    output_size = int.from_bytes(data[offset + 1 : offset + 4], "little")
    source = offset + 4
    output = bytearray()
    while len(output) < output_size:
        flags = data[source]
        source += 1
        for bit in range(7, -1, -1):
            if len(output) >= output_size:
                break
            if flags & (1 << bit):
                pair = int.from_bytes(data[source : source + 2], "big")
                source += 2
                count = (pair >> 12) + 3
                distance = (pair & 0xFFF) + 1
                if distance > len(output):
                    raise ValueError(f"invalid LZ77 back-reference at {source - 2:#x}")
                for _ in range(count):
                    output.append(output[-distance])
                    if len(output) >= output_size:
                        break
            else:
                output.append(data[source])
                source += 1
    return bytes(output), source - offset


def compress_lz77(data: bytes) -> bytes:
    """Encode BIOS 0x10 LZ77 with a 4 KiB/18-byte window and lazy matches."""
    if len(data) > 0xFFFFFF:
        raise ValueError("GBA LZ77 output length exceeds 24-bit header")
    output = bytearray((0x10, len(data) & 0xFF, (len(data) >> 8) & 0xFF, len(data) >> 16))
    positions: dict[bytes, list[int]] = {}
    source = 0

    def remember(position: int) -> None:
        if position + 2 >= len(data):
            return
        key = data[position : position + 3]
        entries = positions.setdefault(key, [])
        entries.append(position)
        cutoff = position - 0x1000
        while entries and entries[0] < cutoff:
            del entries[0]

    def find_match(position: int, extra: int | None = None) -> tuple[int, int]:
        if position + 2 >= len(data):
            return 0, 0
        candidates = list(positions.get(data[position : position + 3], ()))
        # When considering a literal, that byte becomes available as a
        # one-byte-distance match at the next position.
        if extra is not None and data[extra : extra + 3] == data[position : position + 3]:
            candidates.append(extra)
        best_length = 0
        best_distance = 0
        for candidate in reversed(candidates):
            distance = position - candidate
            if distance > 0x1000:
                break
            length = 3
            limit = min(18, len(data) - position)
            while length < limit and data[candidate + length] == data[position + length]:
                length += 1
            if length > best_length:
                best_length = length
                best_distance = distance
                if length == 18:
                    break
        return best_length, best_distance

    while source < len(data):
        flag_offset = len(output)
        output.append(0)
        flags = 0
        for bit in range(7, -1, -1):
            if source >= len(data):
                break
            best_length, best_distance = find_match(source)
            if 3 <= best_length < 18 and source + 1 < len(data):
                next_length, _ = find_match(source + 1, extra=source)
                if next_length > best_length:
                    best_length = 0
            if best_length >= 3:
                flags |= 1 << bit
                pair = ((best_length - 3) << 12) | (best_distance - 1)
                output.extend(pair.to_bytes(2, "big"))
                consumed = best_length
            else:
                output.append(data[source])
                consumed = 1
            for position in range(source, source + consumed):
                remember(position)
            source += consumed
        output[flag_offset] = flags
    return bytes(output)


MAP_WIDTH = 32
MAP_HEIGHT = 20
MAP_SIZE = MAP_WIDTH * MAP_HEIGHT * 2


def rebuild_title(
    decoded: bytes,
    source_map: bytes,
    config: TitleConfig,
) -> tuple[bytes, bytes]:
    """Rebuild the complete 256x160 title layer and its complete tile map."""
    if len(decoded) != config.decoded_size:
        raise ValueError(f"unexpected title asset size {len(decoded):#x}")
    if len(source_map) != MAP_SIZE:
        raise ValueError(f"unexpected title map size {len(source_map):#x}")

    # Flatten the entire native layer, resolving every tile-map flip. The
    # replacement map can then be fully uniform and independent of either
    # version's original atlas layout.
    canvas = bytearray(MAP_WIDTH * 8 * MAP_HEIGHT * 8)
    canvas_width = MAP_WIDTH * 8
    for cell in range(MAP_WIDTH * MAP_HEIGHT):
        entry = int.from_bytes(source_map[cell * 2 : cell * 2 + 2], "little")
        tile_id = entry & 0x3FF
        tile_start = 4 + tile_id * 64
        tile = decoded[tile_start : tile_start + 64]
        if len(tile) != 64:
            raise ValueError(f"title map references missing tile {tile_id:#x}")
        flip_x = bool(entry & 0x400)
        flip_y = bool(entry & 0x800)
        cell_x = (cell % MAP_WIDTH) * 8
        cell_y = (cell // MAP_WIDTH) * 8
        for pixel_y in range(8):
            source_y = 7 - pixel_y if flip_y else pixel_y
            for pixel_x in range(8):
                source_x = 7 - pixel_x if flip_x else pixel_x
                canvas[(cell_y + pixel_y) * canvas_width + cell_x + pixel_x] = (
                    tile[source_y * 8 + source_x]
                )

    # Overlay the complete visible-screen PNG at (0, 0). Every transparent
    # pixel preserves the native layer; every opaque reference color maps to
    # the equivalent palette entry for the selected edition.
    overlay_width, overlay_height, overlay = read_rgba_png(OVERLAY_PATH)
    if (overlay_width, overlay_height) != (240, 160):
        raise ValueError(
            f"title overlay must be 240x160, got {overlay_width}x{overlay_height}"
        )
    overlay_palette = {
        (0, 0, 0): config.colors["black"],
        (90, 90, 90): config.colors["gray"],
        (164, 131, 65): config.colors["brown"],
        **dict(zip(REFERENCE_GRADIENT, config.gradient, strict=True)),
    }
    for y in range(overlay_height):
        for x in range(overlay_width):
            offset = (y * overlay_width + x) * 4
            red, green, blue, alpha = overlay[offset : offset + 4]
            if alpha == 0:
                continue
            if alpha != 0xFF:
                raise ValueError(
                    f"title overlay has partial alpha at ({x}, {y}): {alpha}"
                )
            rgb = (red, green, blue)
            if rgb not in overlay_palette:
                raise ValueError(
                    f"title overlay has unmapped color {rgb} at ({x}, {y})"
                )
            canvas[y * canvas_width + x] = overlay_palette[rgb]

    # Retile the whole layer and deduplicate exact 8x8 blocks. Every map entry
    # is a plain sequential atlas reference—no per-version gaps or edge hacks.
    tile_ids: dict[bytes, int] = {}
    tiles: list[bytes] = []
    rebuilt_map = bytearray()
    for tile_y in range(MAP_HEIGHT):
        for tile_x in range(MAP_WIDTH):
            tile = b"".join(
                bytes(
                    canvas[
                        (tile_y * 8 + pixel_y) * canvas_width + tile_x * 8 :
                        (tile_y * 8 + pixel_y) * canvas_width + tile_x * 8 + 8
                    ]
                )
                for pixel_y in range(8)
            )
            if tile not in tile_ids:
                tile_ids[tile] = len(tiles)
                tiles.append(tile)
            rebuilt_map.extend(tile_ids[tile].to_bytes(2, "little"))
    if len(tiles) > 0x400:
        raise ValueError(f"rebuilt title uses too many tiles: {len(tiles)}")

    tile_data = b"".join(tiles)
    rebuilt_size = 4 + len(tile_data)
    rebuilt = bytes((0,)) + rebuilt_size.to_bytes(3, "little") + tile_data
    return rebuilt, bytes(rebuilt_map)


def build(rom: bytes, variant: str) -> tuple[bytes, int, bytes]:
    config = CONFIGS[variant]
    decoded, consumed = decompress_lz77(rom, config.stream_offset)
    if consumed != config.encoded_capacity:
        raise ValueError(
            f"unexpected {variant} compressed title size {consumed:#x}; "
            f"expected {config.encoded_capacity:#x}"
        )
    source_map = rom[config.tilemap_offset : config.tilemap_offset + MAP_SIZE]
    edited, rebuilt_map = rebuild_title(decoded, source_map, config)
    encoded = compress_lz77(edited)
    if len(encoded) > config.encoded_capacity:
        raise ValueError(
            f"edited {variant} title grew to {len(encoded):#x}, beyond "
            f"its {config.encoded_capacity:#x}-byte slot"
        )
    check, check_consumed = decompress_lz77(encoded)
    if check != edited or check_consumed != len(encoded):
        raise ValueError(f"{variant} title LZ77 round trip failed")
    return encoded.ljust(config.encoded_capacity, b"\0"), len(encoded), rebuilt_map


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("variant", choices=CONFIGS)
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("map_output", type=Path)
    args = parser.parse_args()
    encoded, used, rebuilt_map = build(args.rom.read_bytes(), args.variant)
    args.output.write_bytes(encoded)
    args.map_output.write_bytes(rebuilt_map)
    print(
        f"{args.variant}: title-screen 67 uses {used:#x}/"
        f"{CONFIGS[args.variant].encoded_capacity:#x} compressed bytes"
    )


if __name__ == "__main__":
    main()
