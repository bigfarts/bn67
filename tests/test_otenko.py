from pathlib import Path
import tomllib
import unittest


ROOT = Path(__file__).resolve().parents[1]


class OtenkoTests(unittest.TestCase):
    def test_registers_restored_sprite_as_dust_ammo(self) -> None:
        source = (ROOT / "src/chips/otenko.c").read_text()
        self.assertIn(
            "BN67_FIXED_DUST_SPRITE(0x0E, otenko_battle_sprite);",
            source,
        )

    def test_reclaims_a_separate_duplicate_kind_for_custom_ammo(self) -> None:
        expected_reclaims = {
            "gregar": 0x080D9DF8,
            "falzar": 0x080D8588,
        }

        for variant, duplicate_reference in expected_reclaims.items():
            with self.subTest(variant=variant):
                config = tomllib.loads(
                    (ROOT / f"config.{variant}.toml").read_text()
                )
                dust = config["dust_sprites"]
                self.assertEqual(
                    dust["reclaimed"],
                    [
                        {
                            "kind": 0x0B,
                            "alias": 0x04,
                            "references": [duplicate_reference],
                        },
                    ],
                )

                table = (ROOT / dust["native_table"]).read_bytes()
                kind_04 = table[0x04 * 2 : 0x04 * 2 + 2]
                kind_0b = table[0x0B * 2 : 0x0B * 2 + 2]
                self.assertEqual(kind_0b, kind_04)


if __name__ == "__main__":
    unittest.main()
