#!/usr/bin/env python3
"""Extract every ROM-backed build asset from a validated set of source ROMs."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Asset:
    source: str
    output: str
    offset: int
    length: int
    lz77: bool = False


BN3_ROM_BASE = 0x08000000
BN3_CHIP_DATA = 0x11510
BN3_CHIP_RECORD_SIZE = 0x20
BN3_FOLDERBACK_ID = 0x12F
BN3_IMAGE_WIDTH = 64
BN3_IMAGE_HEIGHT = 56
EXE6_IMAGE_WIDTH = 56
EXE6_IMAGE_HEIGHT = 48


def decompress_gba_lz77(data: bytes) -> bytes:
    """Expand a BIOS-compatible type-0x10 LZ77 stream."""
    if len(data) < 4 or data[0] != 0x10:
        raise ValueError("asset is not a GBA type-0x10 LZ77 stream")
    output_size = int.from_bytes(data[1:4], "little")
    source = 4
    output = bytearray()
    while len(output) < output_size:
        if source >= len(data):
            raise ValueError("truncated GBA LZ77 flag byte")
        flags = data[source]
        source += 1
        for bit in range(7, -1, -1):
            if len(output) >= output_size:
                break
            if flags & (1 << bit):
                if source + 2 > len(data):
                    raise ValueError("truncated GBA LZ77 back-reference")
                first, second = data[source:source + 2]
                source += 2
                length = (first >> 4) + 3
                displacement = ((first & 0x0F) << 8 | second) + 1
                if displacement > len(output):
                    raise ValueError("GBA LZ77 back-reference precedes output")
                for _ in range(length):
                    if len(output) >= output_size:
                        break
                    output.append(output[-displacement])
            else:
                if source >= len(data):
                    raise ValueError("truncated GBA LZ77 literal")
                output.append(data[source])
                source += 1
    return bytes(output)


def decompress_sprite_archive(data: bytes) -> bytes:
    """Expand a compressed sprite and remove its nested size prefix."""
    output = decompress_gba_lz77(data)
    if (
        len(output) >= 4
        and output[0] == 0
        and int.from_bytes(output[1:4], "little") == len(output)
    ):
        return output[4:]
    return output


def read_bn3_pointer(rom: bytes, offset: int) -> int:
    address = struct.unpack_from("<I", rom, offset)[0]
    rom_offset = address - BN3_ROM_BASE
    if not 0 <= rom_offset < len(rom):
        raise ValueError(f"pointer 0x{address:08X} at 0x{offset:X} is outside the BN3 ROM")
    return rom_offset


def decode_4bpp_tiles(data: bytes, width: int, height: int) -> list[list[int]]:
    if width % 8 or height % 8:
        raise ValueError("tile dimensions must be multiples of eight")
    expected = width * height // 2
    if len(data) != expected:
        raise ValueError(f"expected 0x{expected:X} image bytes, got 0x{len(data):X}")
    pixels = [[0] * width for _ in range(height)]
    tiles_wide = width // 8
    for tile_index in range((width // 8) * (height // 8)):
        tile_x = (tile_index % tiles_wide) * 8
        tile_y = (tile_index // tiles_wide) * 8
        tile = data[tile_index * 32:(tile_index + 1) * 32]
        for y in range(8):
            for x in range(8):
                packed = tile[y * 4 + x // 2]
                pixels[tile_y + y][tile_x + x] = (packed >> (4 * (x & 1))) & 0x0F
    return pixels


def encode_4bpp_tiles(pixels: list[list[int]]) -> bytes:
    height = len(pixels)
    width = len(pixels[0]) if pixels else 0
    if width % 8 or height % 8 or any(len(row) != width for row in pixels):
        raise ValueError("pixels must form a rectangular, tile-aligned image")
    encoded = bytearray()
    for tile_y in range(0, height, 8):
        for tile_x in range(0, width, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    low = pixels[tile_y + y][tile_x + x]
                    high = pixels[tile_y + y][tile_x + x + 1]
                    if not 0 <= low < 16 or not 0 <= high < 16:
                        raise ValueError("4bpp palette index is outside 0-15")
                    encoded.append(low | (high << 4))
    return bytes(encoded)


def extract_folderback_art(rom: bytes) -> tuple[bytes, bytes, bytes]:
    """Extract BN3 FolderBack menu art and crop it to BN6's chip-art size."""
    record = BN3_CHIP_DATA + BN3_FOLDERBACK_ID * BN3_CHIP_RECORD_SIZE
    if record + BN3_CHIP_RECORD_SIZE > len(rom):
        raise ValueError("BN3 FolderBack chip record is outside the ROM")
    icon_offset = read_bn3_pointer(rom, record + 0x14)
    image_offset = read_bn3_pointer(rom, record + 0x18)
    palette_offset = read_bn3_pointer(rom, record + 0x1C)

    icon = rom[icon_offset:icon_offset + 0x80]
    source_image = rom[image_offset:image_offset + 0x700]
    palette = rom[palette_offset:palette_offset + 0x20]
    if len(icon) != 0x80 or len(source_image) != 0x700 or len(palette) != 0x20:
        raise ValueError("BN3 FolderBack art is truncated")

    pixels = decode_4bpp_tiles(source_image, BN3_IMAGE_WIDTH, BN3_IMAGE_HEIGHT)
    crop_x = (BN3_IMAGE_WIDTH - EXE6_IMAGE_WIDTH) // 2
    crop_y = (BN3_IMAGE_HEIGHT - EXE6_IMAGE_HEIGHT) // 2
    cropped = [
        row[crop_x:crop_x + EXE6_IMAGE_WIDTH]
        for row in pixels[crop_y:crop_y + EXE6_IMAGE_HEIGHT]
    ]
    return icon, encode_4bpp_tiles(cropped), palette


ASSETS = (
    # BN3 Blue: FolderBack's original rumble sample. Its menu art is decoded
    # and cropped separately below because BN3 stores it at 64x56, not 56x48.
    Asset("bn3_blue", "folderback-rumble-sample.bin", 0x215B68, 0x354E),

    # BN5 ProtoMan: Jealousy menu art and chip-delete overlay.
    Asset("bn5_protoman", "jealousy-icon.bin", 0x748F38, 0x80),
    Asset("bn5_protoman", "jealousy-image.bin", 0x7250E8, 0x540),
    Asset("bn5_protoman", "jealousy-palette.bin", 0x734188, 0x20),
    Asset("bn5_protoman", "jealousy-effect-tiles.bin", 0x6FAD2C, 0x100),
    Asset("bn5_protoman", "jealousy-effect-palette.bin", 0x6FAE2C, 0x20),

    # BN5 ProtoMan: SearchMan library art, Navi archive, and both reticles.
    Asset("bn5_protoman", "searchman-image.bin", 0x728568, 0x540),
    Asset("bn5_protoman", "searchman-pal-base.bin", 0x7343C8, 0x20),
    Asset("bn5_protoman", "searchman-pal-sp.bin", 0x7343E8, 0x20),
    Asset("bn5_protoman", "searchman-battle-sprite.bin", 0x254F64, 0xABFC),
    Asset("bn5_protoman", "searchman-reticle-alt.bin", 0x358410, 0x5B8),
    Asset("bn5_protoman", "searchman-reticle.bin", 0x3589C8, 0x460),

    # BN5 ProtoMan: ChaosLord menu art and complete controller assets. The
    # teardown slice intentionally starts on the archive header at 0x389E68.
    Asset("bn5_protoman", "chaoslord-icon.bin", 0x749C38, 0x80),
    Asset("bn5_protoman", "chaoslord-image.bin", 0x72FE28, 0x540),
    Asset("bn5_protoman", "chaoslord-palette.bin", 0x734AE8, 0x20),
    Asset("bn5_protoman", "chaoslord-bass-sprite.bin", 0x2D3304, 0x1081C),
    Asset("bn5_protoman", "chaoslord-apparition-sprite.bin", 0x398024, 0x186C),
    Asset("bn5_protoman", "chaoslord-aura-sprite.bin", 0x2E3B20, 0x56E0, lz77=True),
    Asset("bn5_protoman", "chaoslord-teardown-sprite.bin", 0x389E68, 0x11F0),
    Asset("bn5_protoman", "chaoslord-trig.bin", 0x5CD0, 0x280),

    # BN4 Blue Moon: SignalRed menu art, traffic light, and placement sample.
    Asset("bn4_blue_moon", "signalred-icon.bin", 0x746EEC, 0x80),
    Asset("bn4_blue_moon", "signalred-image.bin", 0x73A8EC, 0x540),
    Asset("bn4_blue_moon", "signalred-palette.bin", 0x73FAEC, 0x20),
    Asset("bn4_blue_moon", "signalred-battle-sprite.bin", 0x381C30, 0x694),
    Asset("bn4_blue_moon", "signalred-spawn-sample.bin", 0x17C834, 0x891),

    # BN4 Blue Moon: BugChain menu art, aura, and SFX 0x15D sample.
    Asset("bn4_blue_moon", "bugchain-icon.bin", 0x74626C, 0x80),
    Asset("bn4_blue_moon", "bugchain-image.bin", 0x7315EC, 0x540),
    Asset("bn4_blue_moon", "bugchain-palette.bin", 0x73F1AC, 0x20),
    Asset("bn4_blue_moon", "bugchain-battle-sprite.bin", 0x380CA4, 0xF8C),
    Asset("bn4_blue_moon", "bugchain-sound-sample.bin", 0x1970A0, 0x4B1),

    # BN5 Colonel: BugCharge menu art, Gospel archive, and charge sample.
    Asset("bn5_colonel", "bugcharge-icon.bin", 0x74AE3C, 0x80),
    Asset("bn5_colonel", "bugcharge-image.bin", 0x730664, 0x540),
    Asset("bn5_colonel", "bugcharge-palette.bin", 0x735D64, 0x20),
    Asset("bn5_colonel", "bugcharge-gospel-sprite.bin", 0x3216D4, 0xA84),
    Asset("bn5_colonel", "bugcharge-charge-sample.bin", 0x191A80, 0x676),

    # BN4 Blue Moon: shared Navi summon sample plus RollArrow menu art,
    # sprites, and firing sample.
    Asset("bn4_blue_moon", "common-navi-summon-sample.bin", 0x184D70, 0xF3E),
    Asset("bn4_blue_moon", "rollarrow1-icon.bin", 0x74476C, 0x80),
    Asset("bn4_blue_moon", "rollarrow2-icon.bin", 0x7447EC, 0x80),
    Asset("bn4_blue_moon", "rollarrow3-icon.bin", 0x74486C, 0x80),
    Asset("bn4_blue_moon", "rollarrow-image.bin", 0x729D2C, 0x540),
    Asset("bn4_blue_moon", "rollarrow1-pal.bin", 0x73EAEC, 0x20),
    Asset("bn4_blue_moon", "rollarrow2-pal.bin", 0x73EB0C, 0x20),
    Asset("bn4_blue_moon", "rollarrow3-pal.bin", 0x73EB2C, 0x20),
    Asset("bn4_blue_moon", "rollarrow-actor-sprite.bin", 0x2A5A10, 0xAC58),
    Asset("bn4_blue_moon", "rollarrow-projectile-sprite.bin", 0x35E5C0, 0x160),
    Asset("bn4_blue_moon", "rollarrow-fire-sample.bin", 0x1D2AFC, 0xC34),

    # BN4 Blue Moon: LaserMan library art, shared sprite, and fire SFX. Extraction
    # expands the source archive so packages never need compression metadata.
    Asset("bn4_blue_moon", "laserman-image.bin", 0x73842C, 0x540),
    Asset("bn4_blue_moon", "laserman-pal-base.bin", 0x73F94C, 0x20),
    Asset("bn4_blue_moon", "laserman-pal-sp.bin", 0x73F96C, 0x20),
    Asset("bn4_blue_moon", "laserman-battle-sprite.bin", 0x339B6C, 0x395C, lz77=True),
    Asset("bn4_blue_moon", "laserman-fire-sample.bin", 0x1BCFF8, 0x144E),

    # Native BN6 attack dispatch prefixes. The registry relocates each one and
    # appends package attacks in the remaining 8-bit subfamily namespace.
    Asset("exe6_gregar", "attack-family15-table-gregar.bin", 0x2CCB4, 0xA8),
    Asset("exe6_gregar", "attack-family1B-table-gregar.bin", 0x2CD5C, 0x74),
    Asset("exe6_gregar", "attack-family1C-table-gregar.bin", 0xED730, 0x5C),
    Asset("exe6_falzar", "attack-family15-table-falzar.bin", 0x2CCB4, 0xA8),
    Asset("exe6_falzar", "attack-family1B-table-falzar.bin", 0x2CD5C, 0x74),
    Asset("exe6_falzar", "attack-family1C-table-falzar.bin", 0xEC3F0, 0x5C),

    # Native BN6 object dispatch prefixes. The registry relocates each class
    # to a complete 256-entry object-ID table before appending package objects.
    Asset("exe6_gregar", "object-class1-table-gregar.bin", 0x3C9C, 0x17C),
    Asset("exe6_gregar", "object-class3-table-gregar.bin", 0x3EC4, 0x354),
    Asset("exe6_gregar", "object-class4-table-gregar.bin", 0x42C8, 0x248),
    Asset("exe6_falzar", "object-class1-table-falzar.bin", 0x3C9C, 0x17C),
    Asset("exe6_falzar", "object-class3-table-falzar.bin", 0x3EC4, 0x354),
    Asset("exe6_falzar", "object-class4-table-falzar.bin", 0x42C8, 0x248),

    # Native BN6 sprite pointer tables that receive appended archives.
    Asset("exe6_gregar", "sprite-group08-table-gregar.bin", 0x31DA4, 0x5C),
    Asset("exe6_gregar", "sprite-group0C-table-gregar.bin", 0x31E00, 0x1A4),
    Asset("exe6_gregar", "sprite-group10-table-gregar.bin", 0x31FA4, 0x170),
    Asset("exe6_gregar", "sprite-group14-table-gregar.bin", 0x32114, 0x80),
    Asset("exe6_falzar", "sprite-group08-table-falzar.bin", 0x31DA4, 0x5C),
    Asset("exe6_falzar", "sprite-group0C-table-falzar.bin", 0x31E00, 0x1A4),
    Asset("exe6_falzar", "sprite-group10-table-falzar.bin", 0x31FA4, 0x170),
    Asset("exe6_falzar", "sprite-group14-table-falzar.bin", 0x32114, 0x80),

    # Complete native BN6 song tables, relocated before imported cues append.
    Asset("exe6_gregar", "song-table-gregar.bin", 0x159F48, 0xED0),
    Asset("exe6_falzar", "song-table-falzar.bin", 0x1583F8, 0xED0),

    # BN5 ProtoMan: DeathPhoenix menu art, Navi, and strike/flame archive.
    Asset("bn5_protoman", "deathphoenix-icon.bin", 0x749CB8, 0x80),
    Asset("bn5_protoman", "deathphoenix-image.bin", 0x730368, 0x540),
    Asset("bn5_protoman", "deathphoenix-palette.bin", 0x734B08, 0x20),
    Asset("bn5_protoman", "deathphoenix-battle-sprite.bin", 0x333400, 0x20F4),
    Asset("bn5_protoman", "deathphoenix-strike-sprite.bin", 0x36F074, 0x748),
)


def extract_assets(roms: dict[str, bytes], output_dir: Path) -> tuple[int, int]:
    """Validate every source range, then write all extracted assets."""
    outputs: list[tuple[str, bytes]] = []
    output_names: set[str] = set()

    for asset in ASSETS:
        if asset.output in output_names:
            raise ValueError(f"duplicate output name: {asset.output}")
        if asset.source not in roms:
            raise ValueError(f"unknown ROM source for {asset.output}: {asset.source}")
        if asset.offset < 0 or asset.length <= 0:
            raise ValueError(f"invalid range for {asset.output}")
        end = asset.offset + asset.length
        rom = roms[asset.source]
        if end > len(rom):
            raise ValueError(
                f"{asset.output} range 0x{asset.offset:X}-0x{end:X} "
                f"exceeds {asset.source} size 0x{len(rom):X}"
            )
        output_names.add(asset.output)
        data = rom[asset.offset:end]
        if asset.lz77:
            try:
                data = decompress_sprite_archive(data)
            except ValueError as exc:
                raise ValueError(f"cannot decompress {asset.output}: {exc}") from exc
        outputs.append((asset.output, data))

    folderback_outputs = zip(
        (
            "folderback-icon.bin",
            "folderback-image.bin",
            "folderback-palette.bin",
        ),
        extract_folderback_art(roms["bn3_blue"]),
        strict=True,
    )
    for name, data in folderback_outputs:
        if name in output_names:
            raise ValueError(f"duplicate output name: {name}")
        output_names.add(name)
        outputs.append((name, data))

    output_dir.mkdir(parents=True, exist_ok=True)
    for name, data in outputs:
        path = output_dir / name
        written = path.write_bytes(data)
        if written != len(data):
            raise OSError(f"short write for {path}: wrote {written} of {len(data)} bytes")

    return len(outputs), sum(len(data) for _, data in outputs)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--bn5-protoman", required=True, type=Path)
    parser.add_argument("--bn5-colonel", required=True, type=Path)
    parser.add_argument("--exe6-gregar", required=True, type=Path)
    parser.add_argument("--exe6-falzar", required=True, type=Path)
    parser.add_argument("--bn4-blue-moon", required=True, type=Path)
    parser.add_argument("--bn3-blue", required=True, type=Path)
    args = parser.parse_args()

    rom_paths = {
        "bn5_protoman": args.bn5_protoman,
        "bn5_colonel": args.bn5_colonel,
        "exe6_gregar": args.exe6_gregar,
        "exe6_falzar": args.exe6_falzar,
        "bn4_blue_moon": args.bn4_blue_moon,
        "bn3_blue": args.bn3_blue,
    }
    roms = {name: path.read_bytes() for name, path in rom_paths.items()}
    count, total_bytes = extract_assets(roms, args.output_dir)
    print(f"extracted {count} ROM assets ({total_bytes:#x} bytes) to {args.output_dir}")


if __name__ == "__main__":
    main()
