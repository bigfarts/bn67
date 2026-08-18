from pathlib import Path
import re
import unittest

from extract_assets import (
    ASSETS,
    BN3_DARK_AURA_ANIMATION_0_POINTER,
    BN3_DARK_AURA_ANIMATION_1_POINTER,
    BN3_DARK_AURA_ANIMATION_2_POINTER,
    BN3_DARK_AURA_ANIMATION_3_POINTER,
    BN3_DARK_AURA_ID,
    BN3_DARK_AURA_LIFEAURA_PALETTE,
    BN3_DARK_AURA_NUMBER_OAM_STARTS,
    BN3_DARK_AURA_PALETTE,
    BN3_DARK_AURA_SPRITE_LENGTH,
    BN3_DARK_AURA_SPRITE_OFFSET,
    prepare_bn3_dark_aura_battle_sprite,
)


ROOT = Path(__file__).resolve().parents[1]


class DarkAuraTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/dark_aura.c").read_text()
        self.text = (ROOT / "src/chips/dark_aura.text.toml").read_text()

    def test_replaces_bug_death_thunder_with_bn3_record(self) -> None:
        self.assertEqual(BN3_DARK_AURA_ID, 0x135)
        self.assertIn("BN67_CHIP_RECORD(0x136)", self.source)
        self.assertIn("EXE6_CHIP_CODE_A", self.source)
        self.assertRegex(self.source, re.compile(r"(?m)^    \.mb = 61,$"))
        self.assertIn(".chip_class = EXE6_CHIP_CLASS_GIGA", self.source)

    def test_uses_bn3_dark_aura_visual_and_lifespan(self) -> None:
        self.assertIn(".object_spawn = { .variant = 0x0F }", self.source)
        self.assertIn(
            'BN67_SPRITE(dark_aura_battle_sprite, '
            '"build/dark_aura_battle_sprite.bin")',
            self.source,
        )
        self.assertIn("dark_aura_visual_sprite_dispatch", self.source)
        self.assertIn("__bn67_sprite_group_dark_aura_battle_sprite", self.source)
        self.assertIn('"ldr r1,=3000\\n"', self.source)

    def test_uses_centered_bn3_aura_with_bn3_dark_palette(self) -> None:
        sprite = next(
            asset
            for asset in ASSETS
            if asset.output == "dark_aura_battle_sprite.bin"
        )
        self.assertEqual(sprite.source, "bn3_blue")
        self.assertEqual(sprite.offset, BN3_DARK_AURA_SPRITE_OFFSET)
        self.assertEqual(sprite.length, BN3_DARK_AURA_SPRITE_LENGTH)

        archive = bytearray(BN3_DARK_AURA_SPRITE_LENGTH)
        archive[BN3_DARK_AURA_ANIMATION_0_POINTER:0x08] = (0x14).to_bytes(4, "little")
        archive[BN3_DARK_AURA_ANIMATION_1_POINTER:0x0C] = (0x50).to_bytes(4, "little")
        archive[BN3_DARK_AURA_ANIMATION_2_POINTER:0x10] = (0x154).to_bytes(4, "little")
        archive[BN3_DARK_AURA_ANIMATION_3_POINTER:0x14] = (0x190).to_bytes(4, "little")
        lifeaura = bytes(range(0x20))
        dark_aura = bytes(range(0x20, 0x40))
        archive[
            BN3_DARK_AURA_LIFEAURA_PALETTE:
            BN3_DARK_AURA_LIFEAURA_PALETTE + 0x20
        ] = lifeaura
        archive[
            BN3_DARK_AURA_PALETTE:BN3_DARK_AURA_PALETTE + 0x20
        ] = dark_aura
        for offset, tile in BN3_DARK_AURA_NUMBER_OAM_STARTS:
            archive[offset:offset + 5] = bytes((tile, 0xF8, 0, 1, 0))

        prepared = prepare_bn3_dark_aura_battle_sprite(bytes(archive))
        self.assertEqual(
            prepared[
                BN3_DARK_AURA_ANIMATION_2_POINTER:
                BN3_DARK_AURA_ANIMATION_2_POINTER + 4
            ],
            (0x14).to_bytes(4, "little"),
        )
        self.assertEqual(
            prepared[
                BN3_DARK_AURA_ANIMATION_3_POINTER:
                BN3_DARK_AURA_ANIMATION_3_POINTER + 4
            ],
            (0x50).to_bytes(4, "little"),
        )
        self.assertEqual(
            prepared[
                BN3_DARK_AURA_LIFEAURA_PALETTE:
                BN3_DARK_AURA_LIFEAURA_PALETTE + 0x20
            ],
            dark_aura,
        )
        for offset, _ in BN3_DARK_AURA_NUMBER_OAM_STARTS:
            self.assertEqual(prepared[offset:offset + 5], b"\xFF" * 5)

    def test_expands_native_threshold_to_three_hundred(self) -> None:
        self.assertIn('"movs r1,#75\\n"', self.source)
        self.assertIn('"lsls r1,r1,#2\\n"', self.source)
        self.assertIn("0x0801A948", self.source)

    def test_restores_bn3_menu_text_and_art(self) -> None:
        self.assertIn('"0x36" = "DarkAura"', self.text)
        self.assertIn('"0x36" = "Dark aura\\nrepels\\nbelow 300"', self.text)
        for asset in ("icon", "image", "palette"):
            self.assertIn(f"build/dark_aura_{asset}.bin", self.source)

    def test_old_bug_death_thunder_patch_is_removed(self) -> None:
        common = (ROOT / "src/common.asm").read_text()
        self.assertNotIn("bug_death_thunder", common)
        self.assertFalse((ROOT / "src/chips/bug_death_thunder.asm").exists())


if __name__ == "__main__":
    unittest.main()
