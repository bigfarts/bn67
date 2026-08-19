from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class SlashCrossTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/slash_cross.c").read_text()
        self.dispatch = (ROOT / "src/cross_b_left.c").read_text()

    def test_routes_slash_cross_b_left_to_native_moonblade(self) -> None:
        self.assertIn("#define SLASH_CROSS_ACTIVE_FORM 3", self.dispatch)
        self.assertIn("#define SLASH_CROSS_B_LEFT_ACTION 0x40", self.source)
        self.assertIn("switch (status->active_form)", self.dispatch)
        self.assertIn("case SLASH_CROSS_ACTIVE_FORM:", self.dispatch)
        self.assertIn("return slash_cross_b_left_init_work;", self.dispatch)
        self.assertNotIn("SLASH_CROSS", (ROOT / "src/heat_cross.c").read_text())

    def test_uses_flat_130_damage_and_native_chip_counter_word(self) -> None:
        self.assertIn("#define SLASH_CROSS_B_LEFT_DAMAGE 130", self.source)
        self.assertIn(
            "#define SLASH_CROSS_MOON_BLADE_COUNTER_FRAMES 0x1E",
            self.source,
        )
        init = self.source[
            self.source.index("USED uint32_t slash_cross_b_left_init_work"):
        ]
        self.assertIn("work->attack_bonus = 0", init)
        self.assertIn("SLASH_CROSS_MOON_BLADE_COUNTER_FRAMES << 16", init)
        self.assertIn("SLASH_CROSS_B_LEFT_DAMAGE", init)
        self.assertIn("return SLASH_CROSS_B_LEFT_ACTION", init)


if __name__ == "__main__":
    unittest.main()
