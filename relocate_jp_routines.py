#!/usr/bin/env python3
"""Relocate the original Japanese-only BN6 object routines into English ROMs."""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path


ROM_BASE = 0x08000000


@dataclass(frozen=True)
class Region:
    symbol: str
    start: int
    end: int
    calls: tuple[int, ...]


CALL_SITES = {
    "count_native_main": (
        0x0C, 0x26, 0x32, 0x3C, 0x40, 0x44, 0x4E, 0x52, 0x58, 0x5C,
        0x60, 0xAA, 0xC8, 0x172, 0x17A, 0x196, 0x1C4, 0x1EE, 0x270,
        0x2A2, 0x2BC, 0x2D2, 0x2DE, 0x322, 0x34E, 0x368, 0x376, 0x3E4,
    ),
    "count_native_aux_main": (
        0x0C, 0x32, 0x3E, 0x48, 0x4C, 0x50, 0x62, 0x66, 0x6C, 0x70,
        0x74, 0x96, 0xD8, 0xE2, 0xF4, 0x120, 0x12C, 0x138, 0x14C,
        0x15E, 0x162, 0x16A, 0x17C, 0x180, 0x1A4, 0x1A8, 0x1B4,
        0x1CA, 0x1D6, 0x1E2, 0x1FC, 0x20E, 0x224, 0x22C, 0x232,
        0x2B4, 0x2BA, 0x2E4, 0x2F0, 0x35A, 0x37C, 0x38E, 0x3AA,
        0x3C0, 0x3CE, 0x406, 0x41E,
    ),
    "count_lance_main": (
        0x12, 0x18, 0x32, 0x40, 0x44, 0x56, 0x5A, 0x60, 0x64,
        0x68, 0x6C, 0x74, 0x80, 0x86, 0x8A, 0x8E, 0xAA, 0xAE,
        0xB2, 0xC2, 0xCC, 0xDC, 0xE2, 0x11C, 0x14C, 0x154,
        0x166, 0x172, 0x182,
    ),
    "gregar_child_main": (
        0x0C, 0x26, 0x40, 0x4A, 0x4E, 0x52, 0x64, 0x68, 0x6E, 0x72,
        0x76, 0xCA, 0xDC, 0xE0, 0x114, 0x118, 0x14C, 0x1B4, 0x1C4,
        0x1CC, 0x1D4, 0x1DC, 0x1EC, 0x1F8, 0x20E, 0x264, 0x294,
        0x2BA, 0x2CE, 0x2EC, 0x2F6, 0x308, 0x346, 0x356, 0x3DC,
        0x3F4, 0x410,
    ),
    "falzar_child_main": (
        0x0C, 0x26, 0x42, 0x4C, 0x50, 0x54, 0x62, 0x66, 0x6C, 0x70,
        0x74, 0xC2, 0xD4, 0x104, 0x108, 0x138, 0x1A6, 0x1AC, 0x1B0,
        0x1B8, 0x208, 0x22A, 0x23E,
    ),
    "gregar_controller_main": (
        0x54, 0x58, 0x9A, 0xA2, 0xC8, 0xE8, 0xF0, 0x110, 0x152,
        0x166, 0x17A, 0x19C, 0x1AC, 0x1B2,
    ),
    "falzar_controller_main": (
        0x54, 0x58, 0xA2, 0xAA, 0xBC, 0xEC, 0xF8, 0x152, 0x174,
        0x196, 0x1D8, 0x1EC, 0x200, 0x22E, 0x238, 0x24A, 0x288,
        0x298, 0x302, 0x31A, 0x336, 0x366, 0x376, 0x37C,
    ),
    "gregar_shared_aux_main": (
        0x0C, 0x4A, 0x4E, 0x52, 0x68, 0x6C, 0x72, 0x76, 0x7A, 0x98,
        0xB4,
    ),
}


REGIONS = {
    "gregar": (
        ("count_native_main", 0x0BE82C, 0x0BEC18),
        ("count_native_aux_main", 0x0BEC18, 0x0BF080),
        ("count_lance_main", 0x0CACF8, 0x0CAEA0),
        ("gregar_child_main", 0x0C566C, 0x0C5AC8),
        ("falzar_child_main", 0x0C5AC8, 0x0C5D10),
        ("gregar_controller_main", 0x0EEFF8, 0x0EF1BC),
        ("falzar_controller_main", 0x0EF1BC, 0x0EF56C),
        ("gregar_shared_aux_main", 0x0EF630, 0x0EF6F8),
    ),
    "falzar": (
        ("count_native_main", 0x0BCFCC, 0x0BD3B8),
        ("count_native_aux_main", 0x0BD3B8, 0x0BD820),
        ("count_lance_main", 0x0C9498, 0x0C9640),
        ("gregar_child_main", 0x0C3E0C, 0x0C4268),
        ("falzar_child_main", 0x0C4268, 0x0C44B0),
        ("gregar_controller_main", 0x0EDCC8, 0x0EDE8C),
        ("falzar_controller_main", 0x0EDE8C, 0x0EE23C),
        ("gregar_shared_aux_main", 0x0EE300, 0x0EE3C8),
    ),
}


COMMON_CALL_TARGETS = {
    0x080005CC: 0x080005CC,
    0x08000C72: 0x08000C72,
    0x08001330: 0x08001330,
    0x08001532: 0x08001532,
    0x080026A4: 0x080026A4,
    0x080026E4: 0x080026E4,
    0x08002B30: 0x08002B30,
    0x08002D80: 0x08002D80,
    0x08002DA4: 0x08002DA4,
    0x08002DB0: 0x08002DB0,
    0x08002DD8: 0x08002DD8,
    0x08002E3C: 0x08002E3C,
    0x08002F5C: 0x08002F5C,
    0x08002F90: 0x08002F90,
    0x08003320: 0x08003320,
    0x08003358: 0x08003358,
    0x080033AC: 0x080033AC,
    0x0800343C: 0x08003458,
    0x08006264: 0x08006270,
    0x080062EC: 0x080062F8,
    0x0800A776: 0x0800A18E,
    0x0800A680: 0x0800A098,
    0x0800CF14: 0x0800C8F8,
    0x0800D282: 0x0800CC66,
    0x0800D28E: 0x0800CC72,
    0x0800D2A2: 0x0800CC86,
    0x0800D2C2: 0x0800CCA6,
    0x0800D480: 0x0800CE64,
    0x0800D1F4: 0x0800CBD8,
    0x0800DA1A: 0x0800D3FE,
    0x0800E892: 0x0800E276,
    0x0800E8B8: 0x0800E29C,
    0x0800E8C8: 0x0800E2AC,
    0x0800E8DC: 0x0800E2C0,
    0x0800E8E6: 0x0800E2CA,
    0x0800EA72: 0x0800E456,
    0x0801DE6C: 0x0801DA48,
    0x0801DEF0: 0x0801DACC,
    0x0801C010: 0x0801BBF4,
    0x0801C0C2: 0x0801BCA6,
    0x08019E12: 0x08019892,
    0x0801A534: 0x08019FB4,
    0x0801A58E: 0x0801A00E,
    0x0801A598: 0x0801A018,
    0x0801A5F4: 0x0801A074,
    0x0801A5FC: 0x0801A07C,
    0x0801A654: 0x0801A0D4,
    0x0801A6C0: 0x0801A140,
    0x0802F04A: 0x0802E09A,
    0x08031264: 0x080302A8,
}


VARIANT_CALL_TARGETS = {
    "gregar": {
        0x080BC720: 0x080BA6A0,
        0x080BC748: 0x080BA6C8,
        0x080C9B5A: 0x080C6C16,
        0x080C5F94: 0x080C3064,
        0x080CAF50: 0x080C7BA0,
        0x080DA618: 0x080D6D60,
        0x080E1D48: 0x080DE4E0,
        0x080E4230: 0x080E09D0,
        0x080E4332: 0x080E0AD2,
        0x080E518A: 0x080E1932,
        0x080E6B32: 0x080E266E,
    },
    "falzar": {
        0x080BAEC0: 0x080B8E30,
        0x080BAEE8: 0x080B8E58,
        0x080C82FA: 0x080C53A6,
        0x080C4734: 0x080C17F4,
        0x080C96F0: 0x080C6330,
        0x080D8DB8: 0x080D54F0,
        0x080E04E8: 0x080DCC70,
        0x080E29D0: 0x080DF160,
        0x080E2AD2: 0x080DF262,
        0x080E3E5E: 0x080E05F6,
        0x080E5806: 0x080E1332,
    },
}


POINTER_TARGETS = {
    0x0800343D: 0x08003459,
    0x0800BF33: 0x0800B917,
    0x0800BF69: 0x0800B94D,
    0x0800BFCD: 0x0800B9B1,
    0x0800C2A5: 0x0800BC89,
    0x0800C313: 0x0800BCF7,
    0x0800C351: 0x0800BD35,
    0x0801A0F8: 0x08019B78,
    0x0801721B: 0x08016C8B,
}


VARIANT_POINTER_TARGETS = {
    "gregar": {0x080C7737: 0x080C4807},
    "falzar": {0x080C5ED7: 0x080C2F97},
}


def decode_thumb_bl(data: bytes, offset: int, address: int) -> int:
    first, second = struct.unpack_from("<HH", data, offset)
    if first & 0xF800 != 0xF000 or second & 0xF800 != 0xF800:
        raise ValueError(f"expected Thumb BL at 0x{address:08X}")
    displacement = ((first & 0x7FF) << 12) | ((second & 0x7FF) << 1)
    if displacement & 0x400000:
        displacement -= 0x800000
    return address + 4 + displacement


def region_for_address(regions: tuple[Region, ...], address: int) -> Region | None:
    offset = address - ROM_BASE
    return next((item for item in regions if item.start <= offset < item.end), None)


def asm_bytes(data: bytes) -> str:
    return "    .byte " + ",".join(f"0x{value:02X}" for value in data)


def generate(variant: str, source: bytes) -> str:
    regions = tuple(
        Region(symbol, start, end, CALL_SITES[symbol])
        for symbol, start, end in REGIONS[variant]
    )
    call_targets = COMMON_CALL_TARGETS | VARIANT_CALL_TARGETS[variant]
    pointer_targets = POINTER_TARGETS | VARIANT_POINTER_TARGETS[variant]
    stubs: dict[int, str] = {}
    lines = [
        "/* Generated from the original Japanese BN6 ROM. Do not edit. */",
        ".syntax unified",
        ".cpu arm7tdmi",
        ".thumb",
        '.section .text.jp_routines,"ax",%progbits',
    ]

    for region in regions:
        blob = source[region.start : region.end]
        if len(blob) != region.end - region.start:
            raise ValueError(f"{region.symbol}: source ROM is truncated")
        replacements: dict[int, tuple[int, str]] = {}

        detected_calls = {
            relative
            for relative in range(0, len(blob) - 3, 2)
            if struct.unpack_from("<H", blob, relative)[0] & 0xF800 == 0xF000
            and struct.unpack_from("<H", blob, relative + 2)[0] & 0xF800
            == 0xF800
        }
        unlisted_calls = detected_calls - set(region.calls)
        if unlisted_calls:
            formatted = ", ".join(f"0x{value:X}" for value in sorted(unlisted_calls))
            raise ValueError(
                f"{region.symbol}: unrelocated Thumb BL sites: {formatted}"
            )

        for relative in region.calls:
            source_address = ROM_BASE + region.start + relative
            target = decode_thumb_bl(blob, relative, source_address)
            target_region = region_for_address(regions, target)
            if target_region is region:
                continue
            if target_region is not None:
                expression = (
                    f"{target_region.symbol}+"
                    f"0x{target - ROM_BASE - target_region.start:X}"
                )
            else:
                translated = call_targets.get(target)
                if translated is None:
                    raise ValueError(
                        f"{region.symbol}+0x{relative:X}: no English target for "
                        f"Japanese call 0x{target:08X}"
                    )
                expression = stubs.setdefault(
                    translated, f".Ljp_call_{translated:08x}"
                )
            replacements[relative] = (4, f"    bl {expression}")

        for relative in range(0, len(blob) - 3, 4):
            value = struct.unpack_from("<I", blob, relative)[0]
            target_region = region_for_address(regions, value & ~1)
            if target_region is not None:
                # A Thumb function symbol acquires bit 0 when GNU as emits it
                # as data. Use an untyped base label for internal addresses so
                # data-table pointers remain even, then preserve bit 0 only
                # when it was explicitly present in the Japanese ROM.
                expression = (
                    f".L{target_region.symbol}_base+"
                    f"0x{(value & ~1) - ROM_BASE - target_region.start:X}"
                )
                if value & 1:
                    expression += "+1"
                replacements[relative] = (4, f"    .word {expression}")
            elif value in pointer_targets:
                replacements[relative] = (
                    4,
                    f"    .word 0x{pointer_targets[value]:08X}",
                )

        lines.extend(
            [
                "",
                ".balign 4",
                f".L{region.symbol}_base:",
                f".global {region.symbol}",
                f".type {region.symbol},%function",
                ".thumb_func",
                f"{region.symbol}:",
            ]
        )
        cursor = 0
        for offset, (size, replacement) in sorted(replacements.items()):
            if offset < cursor:
                raise ValueError(
                    f"{region.symbol}: overlapping relocation at 0x{offset:X}"
                )
            if cursor < offset:
                lines.append(asm_bytes(blob[cursor:offset]))
            lines.append(replacement)
            cursor = offset + size
        if cursor < len(blob):
            lines.append(asm_bytes(blob[cursor:]))
        lines.append(f".size {region.symbol},.-{region.symbol}")

    for alias, symbol, offset in (
        ("count_attack_main", "count_native_main", 0x26A),
        ("gregar_attack_main", "gregar_controller_main", 0x174),
        ("falzar_attack_main", "falzar_controller_main", 0x1FA),
    ):
        lines.extend(
            [
                "",
                f".global {alias}",
                f".type {alias},%function",
                f".thumb_set {alias},{symbol}+0x{offset:X}",
            ]
        )

    for target, label in sorted(stubs.items()):
        literal = f"{label}_target"
        lines.extend(
            [
                "",
                ".balign 4",
                ".thumb_func",
                f"{label}:",
                "    push {r3}",
                f"    ldr r3,{literal}",
                "    mov r12,r3",
                "    pop {r3}",
                "    bx r12",
                "    .balign 4",
                f"{literal}:",
                f"    .word 0x{target | 1:08X}",
            ]
        )
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--variant", choices=sorted(REGIONS), required=True)
    parser.add_argument("--source-rom", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    content = generate(args.variant, args.source_rom.read_bytes())
    if not args.output.exists() or args.output.read_text() != content:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
