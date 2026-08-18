#!/usr/bin/env python3
"""Extract every ROM-backed build asset from a validated set of source ROMs."""

from __future__ import annotations

import argparse
import base64
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
BN3_ROOK_ID = 0x99
BN3_FOLDERBACK_ID = 0x12F
BN3_IMAGE_WIDTH = 64
BN3_IMAGE_HEIGHT = 56
EXE6_IMAGE_WIDTH = 56
EXE6_IMAGE_HEIGHT = 48
EXE6_ICON_SIZE = 16
EXE6_ICON_BORDER_INDEX = 5

# Duo is one of BN4's two event chips.  Its chip record points at the event
# loader's EWRAM cache instead of directly at ROM, so the image and palette
# cannot be represented by an ordinary static ROM slice.  These are the exact
# cache bytes produced by the US Blue Moon event loader, captured by the Tango
# headless reference probe in tools/duo_reference_probe.rs.  The icon is a
# normal ROM-backed asset and remains in ASSETS below.
DUO_IMAGE = base64.b64decode(
    "ZlNVVWZbVVVmK1VVZrdVVXa2UlVmdytSdne3O2d3d7clJVJVJVVSVSVVUiJVVSIzJSUz2yUyyu4zM+zMzMzrrlVVIjIlIjPLMrPs"
    "7svu7u7u7u6s7s2KVotmhliqZocZM8vc7u3u7u7u7u6s7r1od3lmZnZlZmZ3VWVmZlZVZXbuzpqj3pqZeniYmWaHiGlmd4hnZndo"
    "ZmZ3dmZmZndnZmdmZWVmZmZmZmZmZmZmZmZmZmaXZmaXqmaXq6qovLuqZpaZmWaYmZmGqZmamZqamaqpqZmqqZk5qpk5I6o5I1Jm"
    "dnd3ZnZ2dmdmZ2d8ZmZm3XpmZt3di2bd3d3N3N3d3Wdn6K52ZtauZmZ2d2ZmdnhmZmZ4ZmZmeLu7u3fd3d1nqWeIHoiHyB9mmPrP"
    "dqr933Z3FxFWVVURZlVlFVZVVVYRVVVVERFVZRERcXcRRHFnRER3VkQUd1VBdGdVQXFWtWZ3Z3Zmd1ZmZlZVZlVVVWZVVVVmVVVW"
    "ZpVVVmXNVVZly7urqry8u6rLu6uquLy6qse7u6q2u6uqpruqOZarOTM5IyVVOVIlVTlSJVU5UiVVOVIlVTlSJVUjUiVSIlIlUtvd"
    "3d2ou9zdJTIzyyUlIjNVJSUyVSVVMlUlVSJVJSUl3d3dad3d3Wvd3d1d3d3dbdzd3b3b3d29M7vLrCIzy71mVVVlVlVVZlfY/s5X"
    "pu19dnaHyGZ22+5qdsjuembG7hVnpv52ZtX/ZpZndnuGmap+h5m6nneZuq53qbrOiKmqv1VWZXxlVWWHZlVlqlZVZatWVWa7VlVm"
    "u1dVZrtXVYaGuszMZoiIiGZ3d4hmdoeIZnaIeGaHiHeHiHd3iHh3d8wjJVKIPCJSiMkiUpfJI1KImTxSh5k8UniYySOHmMk5VSUl"
    "JVUlJSJVJSUyVSUlMlUlJTVVJSUlVSUlJVUlJSUyw+69w+59Zpyad2acqndmnKp3ZsOqd2bDqnhmw6poZspmpe3YaIbt1mxm7MZt"
    "VuuGfVbZVp1mx2bcZpVW2GZV7pqqqu6tqqrumoiort68iu7MmqrujWep7I53prmNd4arWIeIqpiJd6qoqneqqKp4mqeqeImXqnl4"
    "hap6V3Wqenh3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d4iZvIeImcl3iJiod4eImXeHl5l3d3Z3d2Zmd2dmZnZVJSUl"
    "VXUlMlVVJbNVVTXrJVU17SVVNe5VVTLeVVUyz8OqaWbDqWlW/plYVf+sV1X/rpqZy82qeKuqiZerqoqXVcVoVlVVZlZVVVVVVVVV"
    "VWdVVVV3d3eIiYiHd4iIZ2aFi3dnVWV3V1VVVVVVVVVVVVVmdlaGmKp2qaqqmImIiFVlqopVVaqaVVWqmlV1mZmHqaqqqqqqmaqa"
    "eHeIeHd3d3d3Z3d3Z2Z3h4h2qJp5ZpqId2d3d3dnd3d3Z3d3d3dmZnd2ZnZ3Z3d3d2dmd3dnZnd3Z2Z2d1ZmdmdVZmZWVSUls78l"
    "ks6+JerPvZX835qV7O+rsuz/rcLO2625vqqaq6p6iaqqeomqqnmJqqp5d5mqeGaqiXdlmnh3ZYl3V1WIiGZliIhmZoh4ZlZ3d2dW"
    "ZmZ3VmZmdmdmZmZ2ZmZmh5mZiYiZmYmImZmJiJmZiYiZmYmImZmJiJmZiYiIiIiYiIh3d4iId3eIiHd3iIh3d4iId3eIiHeHiIiY"
    "qpmqqqp3d3d3d3d3d3d3d6h3d5eqd5iqqqmqqqqqqqp4qpp4d2ZmZVWZiWdVqqpoVaqKZ1Wad2ZVeGdmVXdmVlVnZlVV",
    validate=True,
)
DUO_PALETTE = base64.b64decode(
    "AAA5GEAZBjKfKYAIYxzFJCcxiTnLQQ1GcVL1Ynlv3Xs=",
    validate=True,
)
assert len(DUO_IMAGE) == 0x540
assert len(DUO_PALETTE) == 0x20


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
                first, second = data[source : source + 2]
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
        raise ValueError(
            f"pointer 0x{address:08X} at 0x{offset:X} is outside the BN3 ROM"
        )
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
        tile = data[tile_index * 32 : (tile_index + 1) * 32]
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


def normalize_exe6_icon_border(icon: bytes) -> bytes:
    """Remap a source icon's inner frame to EXE6's shared palette index."""
    pixels = decode_4bpp_tiles(icon, EXE6_ICON_SIZE, EXE6_ICON_SIZE)
    first = 1
    last = EXE6_ICON_SIZE - 2
    for coordinate in range(first, last + 1):
        pixels[first][coordinate] = EXE6_ICON_BORDER_INDEX
        pixels[last][coordinate] = EXE6_ICON_BORDER_INDEX
        pixels[coordinate][first] = EXE6_ICON_BORDER_INDEX
        pixels[coordinate][last] = EXE6_ICON_BORDER_INDEX
    return encode_4bpp_tiles(pixels)


def extract_bn3_chip_art(
    rom: bytes,
    chip_id: int,
    chip_name: str,
) -> tuple[bytes, bytes, bytes]:
    """Extract BN3 menu art and crop it to BN6's chip-art size."""
    record = BN3_CHIP_DATA + chip_id * BN3_CHIP_RECORD_SIZE
    if record + BN3_CHIP_RECORD_SIZE > len(rom):
        raise ValueError(f"BN3 {chip_name} chip record is outside the ROM")
    icon_offset = read_bn3_pointer(rom, record + 0x14)
    image_offset = read_bn3_pointer(rom, record + 0x18)
    palette_offset = read_bn3_pointer(rom, record + 0x1C)

    icon = rom[icon_offset : icon_offset + 0x80]
    source_image = rom[image_offset : image_offset + 0x700]
    palette = rom[palette_offset : palette_offset + 0x20]
    if len(icon) != 0x80 or len(source_image) != 0x700 or len(palette) != 0x20:
        raise ValueError(f"BN3 {chip_name} art is truncated")

    pixels = decode_4bpp_tiles(source_image, BN3_IMAGE_WIDTH, BN3_IMAGE_HEIGHT)
    crop_x = (BN3_IMAGE_WIDTH - EXE6_IMAGE_WIDTH) // 2
    crop_y = (BN3_IMAGE_HEIGHT - EXE6_IMAGE_HEIGHT) // 2
    cropped = [
        row[crop_x : crop_x + EXE6_IMAGE_WIDTH]
        for row in pixels[crop_y : crop_y + EXE6_IMAGE_HEIGHT]
    ]
    return normalize_exe6_icon_border(icon), encode_4bpp_tiles(cropped), palette


ASSETS = (
    # Japanese BN6: event-chip menu art and the two removed battle archives.
    # These offsets come directly from the original BR5J/BR6J chip records and
    # sprite pointer tables; no redraws or palette conversions are involved.
    Asset("exe6_jp_gregar", "gun_del_sol_ex_icon.bin", 0x74880C, 0x80),
    Asset("exe6_jp_gregar", "gun_del_sol_ex_image.bin", 0x717B4C, 0x540),
    Asset("exe6_jp_gregar", "gun_del_sol_ex_palette.bin", 0x745C6C, 0x20),
    Asset("exe6_jp_gregar", "otenko_icon.bin", 0x74CB8C, 0x80),
    Asset("exe6_jp_gregar", "otenko_image.bin", 0x72EACC, 0x540),
    Asset("exe6_jp_gregar", "otenko_palette.bin", 0x746D4C, 0x20),
    Asset("exe6_jp_gregar", "otenko_battle_sprite.bin", 0x33962C, 0x690),
    # Crossover's subtype-1 partner uses group 0x0C/index 0x0F. The complete
    # compressed Django archive, including the GunDelSol overlay animations, is
    # identical in Japanese Gregar and Falzar. The English releases replace its
    # table entry with a white dot and redirect the overlay effect selectors.
    Asset(
        "exe6_jp_gregar",
        "django_crossover_sprite.bin",
        0x30B980,
        0x21CC,
    ),
    Asset("exe6_jp_gregar", "count_icon.bin", 0x74E48C, 0x80),
    Asset("exe6_jp_gregar", "count_image.bin", 0x740B8C, 0x540),
    Asset("exe6_jp_gregar", "count_pal_base.bin", 0x747A4C, 0x20),
    Asset("exe6_jp_gregar", "count_pal_ex.bin", 0x747A6C, 0x20),
    Asset("exe6_jp_gregar", "count_pal_sp.bin", 0x747A8C, 0x20),
    Asset("exe6_jp_gregar", "count_battle_sprite.bin", 0x2EF3C0, 0x33DC),
    Asset("exe6_jp_gregar", "double_beast_icon.bin", 0x74EC8C, 0x80),
    Asset("exe6_jp_gregar", "double_beast_image.bin", 0x74304C, 0x540),
    Asset("exe6_jp_gregar", "double_beast_palette.bin", 0x747BAC, 0x20),
    Asset("exe6_jp_gregar", "gregar_icon.bin", 0x74ED0C, 0x80),
    Asset("exe6_jp_gregar", "gregar_image.bin", 0x74358C, 0x540),
    Asset("exe6_jp_gregar", "gregar_battle_sprite.bin", 0x357464, 0x1E40),
    Asset("exe6_jp_falzar", "falzar_icon.bin", 0x750E58, 0x80),
    Asset("exe6_jp_falzar", "falzar_image.bin", 0x745658, 0x540),
    Asset("exe6_jp_gregar", "falzar_battle_sprite.bin", 0x3557D8, 0x1A2C),
    # EXE4.5: BlackWeapon's Advance Battle Chip menu art. Unlike the BN4
    # operation-battle placeholder, EXE4.5 contains the complete library art.
    Asset("exe45", "black_weapon_icon.bin", 0x7640B0, 0x80),
    Asset("exe45", "black_weapon_image.bin", 0x755CF0, 0x540),
    Asset("exe45", "black_weapon_palette.bin", 0x75CEF0, 0x20),
    # BN3 Blue: the shared group-0x10/id-0x3D chess-piece archive. Rook is
    # animation 4; its menu art is decoded and cropped separately below because
    # BN3 stores it at 64x56, not 56x48.
    Asset("bn3_blue", "rook_battle_sprite.bin", 0x2CD434, 0x20A0),
    # BN3 Blue: FolderBack's original rumble sample. Its menu art uses the
    # same decoded-and-cropped path as Rook.
    Asset("bn3_blue", "folder_back_rumble_sample.bin", 0x215B68, 0x354E),
    # BN5 ProtoMan: Jealousy menu art and chip-delete overlay.
    Asset("bn5_protoman", "jealousy_icon.bin", 0x748F38, 0x80),
    Asset("bn5_protoman", "jealousy_image.bin", 0x7250E8, 0x540),
    Asset("bn5_protoman", "jealousy_palette.bin", 0x734188, 0x20),
    Asset("bn5_protoman", "jealousy_effect_tiles.bin", 0x6FAD2C, 0x100),
    Asset("bn5_protoman", "jealousy_effect_palette.bin", 0x6FAE2C, 0x20),
    # BN5 ProtoMan: Django's library art and complete base-chip sequence. The
    # actor, sunlight, and coffin archives use the animation layouts expected
    # by the restored standalone attack and remain separate from Crossover.
    # Django3 uses DjangoSP's original palette. Django2's Japanese BN6 green
    # background colors are defined beside its chip record in django.c.
    Asset("bn5_protoman", "django_icon.bin", 0x7493B8, 0x80),
    Asset("bn5_protoman", "django_image.bin", 0x72D968, 0x540),
    Asset(
        "bn5_protoman",
        "django_palette.bin",
        0x7349C8,
        0x20,
    ),
    Asset("bn5_protoman", "django3_palette.bin", 0x7349E8, 0x20),
    Asset("bn5_protoman", "django_battle_sprite.bin", 0x31F1D0, 0x1C50),
    Asset("bn5_protoman", "django_sun_sprite.bin", 0x322158, 0x8EC),
    Asset("bn5_protoman", "django_coffin_sprite.bin", 0x322A44, 0x580),
    # BN5 ProtoMan: SearchMan library art, Navi archive, and both reticles.
    Asset("bn5_protoman", "searchman_image.bin", 0x728568, 0x540),
    Asset("bn5_protoman", "searchman_pal_base.bin", 0x7343C8, 0x20),
    Asset("bn5_protoman", "searchman_pal_sp.bin", 0x7343E8, 0x20),
    Asset("bn5_protoman", "searchman_battle_sprite.bin", 0x254F64, 0xABFC),
    Asset("bn5_protoman", "searchman_reticle_alt.bin", 0x358410, 0x5B8),
    Asset("bn5_protoman", "searchman_reticle.bin", 0x3589C8, 0x460),
    # BN5: NumberMan's library art, full Navi archive, and animated die.
    Asset("bn5_colonel", "numberman_image.bin", 0x72AD24, 0x540),
    Asset("bn5_colonel", "numberman_pal_base.bin", 0x735804, 0x20),
    Asset("bn5_colonel", "numberman_pal_sp.bin", 0x735824, 0x20),
    Asset("bn5_colonel", "numberman_battle_sprite.bin", 0x2918A0, 0x8824),
    Asset("bn5_colonel", "numberman_die_sprite.bin", 0x2F8D08, 0x16A0),
    # BN5 ProtoMan: ChaosLord menu art and complete controller assets. The
    # teardown slice intentionally starts on the archive header at 0x389E68.
    Asset("bn5_protoman", "chaos_lord_icon.bin", 0x749C38, 0x80),
    Asset("bn5_protoman", "chaos_lord_image.bin", 0x72FE28, 0x540),
    Asset("bn5_protoman", "chaos_lord_palette.bin", 0x734AE8, 0x20),
    Asset("bn5_protoman", "chaos_lord_bass_sprite.bin", 0x2D3304, 0x1081C),
    Asset("bn5_protoman", "chaos_lord_apparition_sprite.bin", 0x398024, 0x186C),
    Asset("bn5_protoman", "chaos_lord_aura_sprite.bin", 0x2E3B20, 0x56E0, lz77=True),
    Asset("bn5_protoman", "chaos_lord_teardown_sprite.bin", 0x389E68, 0x11F0),
    Asset("bn5_protoman", "chaos_lord_trig.bin", 0x5CD0, 0x280),
    # BN4 Blue Moon: SignalRed menu art, traffic light, and placement sample.
    Asset("bn4_blue_moon", "signal_red_icon.bin", 0x746EEC, 0x80),
    Asset("bn4_blue_moon", "signal_red_image.bin", 0x73A8EC, 0x540),
    Asset("bn4_blue_moon", "signal_red_palette.bin", 0x73FAEC, 0x20),
    Asset("bn4_blue_moon", "signal_red_battle_sprite.bin", 0x381C30, 0x694),
    Asset("bn4_blue_moon", "signal_red_spawn_sample.bin", 0x17C834, 0x891),
    # BN4 Blue Moon: BlackWeapon's near-black MegaMan palette. The same
    # 16-color palette is repeated for the relevant animation groups.
    Asset("bn4_blue_moon", "black_weapon_dark_palette.bin", 0x21B7F4, 0x20),
    # BN4 Blue Moon: BugChain menu art, aura, and SFX 0x15D sample.
    Asset("bn4_blue_moon", "bug_chain_icon.bin", 0x74626C, 0x80),
    Asset("bn4_blue_moon", "bug_chain_image.bin", 0x7315EC, 0x540),
    Asset("bn4_blue_moon", "bug_chain_palette.bin", 0x73F1AC, 0x20),
    Asset("bn4_blue_moon", "bug_chain_battle_sprite.bin", 0x380CA4, 0xF8C),
    Asset("bn4_blue_moon", "bug_chain_sound_sample.bin", 0x1970A0, 0x4B1),
    # BN4 Blue Moon: Duo's event icon and combined group-8/id-24 body/fist
    # archive. The source expands to the exact EWRAM archive selected by the
    # native event renderer; decompress_sprite_archive removes its size prefix
    # before the BN6 sprite registry installs it.
    Asset("bn4_blue_moon", "duo_icon.bin", 0x7471EC, 0x80),
    Asset("bn4_blue_moon", "duo_battle_sprite.bin", 0x33FC3C, 0x5833, lz77=True),
    Asset("bn4_blue_moon", "duo_arrival_sound_sample.bin", 0x17D0C8, 0x227D),
    Asset("bn4_blue_moon", "duo_fist_sound_sample.bin", 0x1DD24C, 0x1166),
    # BN5 Colonel: BugCharge menu art, Gospel archive, and charge sample.
    Asset("bn5_colonel", "bug_charge_icon.bin", 0x74AE3C, 0x80),
    Asset("bn5_colonel", "bug_charge_image.bin", 0x730664, 0x540),
    Asset("bn5_colonel", "bug_charge_palette.bin", 0x735D64, 0x20),
    Asset("bn5_colonel", "bug_charge_gospel_sprite.bin", 0x3216D4, 0xA84),
    Asset("bn5_colonel", "bug_charge_charge_sample.bin", 0x191A80, 0x676),
    # BN4 Blue Moon: shared Navi summon sample.
    Asset("bn4_blue_moon", "common_navi_summon_sample.bin", 0x184D70, 0xF3E),
    # BN4 Blue Moon: LaserMan library art, shared sprite, and fire SFX. Extraction
    # expands the source archive so packages never need compression metadata.
    Asset("bn4_blue_moon", "laserman_image.bin", 0x73842C, 0x540),
    Asset("bn4_blue_moon", "laserman_pal_base.bin", 0x73F94C, 0x20),
    Asset("bn4_blue_moon", "laserman_pal_sp.bin", 0x73F96C, 0x20),
    Asset("bn4_blue_moon", "laserman_battle_sprite.bin", 0x339B6C, 0x395C, lz77=True),
    Asset("bn4_blue_moon", "laserman_fire_sample.bin", 0x1BCFF8, 0x144E),
    # Native BN6 attack dispatch prefixes. The registry relocates each one and
    # appends package attacks in the remaining 8-bit subfamily namespace.
    Asset("exe6_gregar", "attack_family15_table_gregar.bin", 0x2CCB4, 0xA8),
    Asset("exe6_gregar", "attack_family1B_table_gregar.bin", 0x2CD5C, 0x74),
    Asset("exe6_gregar", "attack_family1C_table_gregar.bin", 0xED730, 0x5C),
    Asset("exe6_falzar", "attack_family15_table_falzar.bin", 0x2CCB4, 0xA8),
    Asset("exe6_falzar", "attack_family1B_table_falzar.bin", 0x2CD5C, 0x74),
    Asset("exe6_falzar", "attack_family1C_table_falzar.bin", 0xEC3F0, 0x5C),
    # Native BN6 object dispatch prefixes. The registry relocates each class
    # to a complete 256-entry object-ID table before appending package objects.
    Asset("exe6_gregar", "object_class1_table_gregar.bin", 0x3C9C, 0x17C),
    Asset("exe6_gregar", "object_class3_table_gregar.bin", 0x3EC4, 0x354),
    Asset("exe6_gregar", "object_class4_table_gregar.bin", 0x42C8, 0x248),
    Asset("exe6_falzar", "object_class1_table_falzar.bin", 0x3C9C, 0x17C),
    Asset("exe6_falzar", "object_class3_table_falzar.bin", 0x3EC4, 0x354),
    Asset("exe6_falzar", "object_class4_table_falzar.bin", 0x42C8, 0x248),
    # Native BN6 sprite pointer tables that receive appended archives.
    Asset("exe6_gregar", "sprite_group08_table_gregar.bin", 0x31DA4, 0x5C),
    Asset("exe6_gregar", "sprite_group0C_table_gregar.bin", 0x31E00, 0x1A4),
    Asset("exe6_gregar", "sprite_group10_table_gregar.bin", 0x31FA4, 0x170),
    Asset("exe6_gregar", "sprite_group14_table_gregar.bin", 0x32114, 0x80),
    Asset("exe6_falzar", "sprite_group08_table_falzar.bin", 0x31DA4, 0x5C),
    Asset("exe6_falzar", "sprite_group0C_table_falzar.bin", 0x31E00, 0x1A4),
    Asset("exe6_falzar", "sprite_group10_table_falzar.bin", 0x31FA4, 0x170),
    Asset("exe6_falzar", "sprite_group14_table_falzar.bin", 0x32114, 0x80),
    # Native DustCross ammo sprite selectors. The registry appends imported
    # obstacle sprites and redirects every suction/firing lookup to one table.
    Asset("exe6_gregar", "dust_sprite_table_gregar.bin", 0xEAC00, 0x1E),
    Asset("exe6_falzar", "dust_sprite_table_falzar.bin", 0xE98C0, 0x1E),
    # Native field-object NameID render records (IDs 0xCD through 0xEB).
    # The registry relocates this prefix and allocates the remaining IDs through
    # 0xFF for imported obstacles consumed by JunkMan and BlizzardBall.
    Asset("exe6_gregar", "field_object_table_gregar.bin", 0x21220, 0x9B),
    Asset("exe6_falzar", "field_object_table_falzar.bin", 0x21220, 0x9B),
    # Complete native BN6 song tables, relocated before imported cues append.
    Asset("exe6_gregar", "song_table_gregar.bin", 0x159F48, 0xED0),
    Asset("exe6_falzar", "song_table_falzar.bin", 0x1583F8, 0xED0),
    # BN5 ProtoMan: DeathPhoenix menu art, Navi, and strike/flame archive.
    Asset("bn5_protoman", "death_phoenix_icon.bin", 0x749CB8, 0x80),
    Asset("bn5_protoman", "death_phoenix_image.bin", 0x730368, 0x540),
    Asset("bn5_protoman", "death_phoenix_palette.bin", 0x734B08, 0x20),
    Asset("bn5_protoman", "death_phoenix_battle_sprite.bin", 0x333400, 0x20F4),
    Asset("bn5_protoman", "death_phoenix_strike_sprite.bin", 0x36F074, 0x748),
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
        data = rom[asset.offset : end]
        if asset.lz77:
            try:
                data = decompress_sprite_archive(data)
            except ValueError as exc:
                raise ValueError(f"cannot decompress {asset.output}: {exc}") from exc
        outputs.append((asset.output, data))

    for prefix, chip_id, chip_name in (
        ("rook", BN3_ROOK_ID, "Rook"),
        ("folder_back", BN3_FOLDERBACK_ID, "FolderBack"),
    ):
        chip_art_outputs = zip(
            (
                f"{prefix}_icon.bin",
                f"{prefix}_image.bin",
                f"{prefix}_palette.bin",
            ),
            extract_bn3_chip_art(roms["bn3_blue"], chip_id, chip_name),
            strict=True,
        )
        for name, data in chip_art_outputs:
            if name in output_names:
                raise ValueError(f"duplicate output name: {name}")
            output_names.add(name)
            outputs.append((name, data))

    for name, data in (
        ("duo_image.bin", DUO_IMAGE),
        ("duo_palette.bin", DUO_PALETTE),
    ):
        if name in output_names:
            raise ValueError(f"duplicate output name: {name}")
        output_names.add(name)
        outputs.append((name, data))

    output_dir.mkdir(parents=True, exist_ok=True)
    for name, data in outputs:
        path = output_dir / name
        written = path.write_bytes(data)
        if written != len(data):
            raise OSError(
                f"short write for {path}: wrote {written} of {len(data)} bytes"
            )

    return len(outputs), sum(len(data) for _, data in outputs)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", required=True, type=Path)
    parser.add_argument("--bn5-protoman", required=True, type=Path)
    parser.add_argument("--bn5-colonel", required=True, type=Path)
    parser.add_argument("--exe6-gregar", required=True, type=Path)
    parser.add_argument("--exe6-falzar", required=True, type=Path)
    parser.add_argument("--exe6-jp-gregar", required=True, type=Path)
    parser.add_argument("--exe6-jp-falzar", required=True, type=Path)
    parser.add_argument("--bn4-blue-moon", required=True, type=Path)
    parser.add_argument("--bn3-blue", required=True, type=Path)
    parser.add_argument("--exe45", required=True, type=Path)
    args = parser.parse_args()

    rom_paths = {
        "bn5_protoman": args.bn5_protoman,
        "bn5_colonel": args.bn5_colonel,
        "exe6_gregar": args.exe6_gregar,
        "exe6_falzar": args.exe6_falzar,
        "exe6_jp_gregar": args.exe6_jp_gregar,
        "exe6_jp_falzar": args.exe6_jp_falzar,
        "bn4_blue_moon": args.bn4_blue_moon,
        "bn3_blue": args.bn3_blue,
        "exe45": args.exe45,
    }
    roms = {name: path.read_bytes() for name, path in rom_paths.items()}
    count, total_bytes = extract_assets(roms, args.output_dir)
    print(f"extracted {count} ROM assets ({total_bytes:#x} bytes) to {args.output_dir}")


if __name__ == "__main__":
    main()
