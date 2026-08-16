from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class LaserManTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/laserman.c").read_text()
        self.abi = (ROOT / "src/abi.h").read_text()

    def test_down_uninstalls_base_abilities_without_clearing_live_cross(self) -> None:
        self.assertIn("uint8_t active_cross;", self.abi)
        self.assertIn(
            "offsetof(struct Exe6NaviStatusWorkFields, active_cross) == 0x29",
            self.abi,
        )

        apply_effect = self.source[
            self.source.index("static void apply_command_effect"):
            self.source.index("static void refresh_target_player")
        ]
        self.assertIn("exe6_navi_status_set(target_side, property, value);", apply_effect)

        refresh = self.source[
            self.source.index("static void refresh_target_player"):
            self.source.index("static void apply_selected_command")
        ]
        self.assertIn("if (disables_base_ability)", refresh)
        self.assertIn("status->active_cross != 0", refresh)
        self.assertIn("runtime->b_left =", refresh)
        self.assertIn("exe6_battle_hit_status_flag_off(", refresh)


if __name__ == "__main__":
    unittest.main()
