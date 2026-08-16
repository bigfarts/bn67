from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class LaserManTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/laserman.c").read_text()
        self.abi = (ROOT / "src/abi.h").read_text()

    def test_down_uninstalls_base_abilities_without_clearing_live_cross(self) -> None:
        self.assertIn("uint8_t navi_id;", self.abi)
        self.assertIn(
            "offsetof(struct Exe6NaviStatusWorkFields, navi_id) == 0x29",
            self.abi,
        )
        self.assertIn("uint8_t active_form;", self.abi)
        self.assertIn(
            "offsetof(struct Exe6NaviStatusWorkFields, active_form) == 0x2C",
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
        self.assertIn("target_has_active_cross(target_side)", refresh)
        self.assertIn("status->active_form <= EXE6_STANDARD_CROSS_COUNT", self.source)
        self.assertIn("runtime->b_left =", refresh)
        self.assertIn("exe6_battle_hit_status_flag_off(", refresh)

    def test_right_restores_cross_native_charge_shot(self) -> None:
        self.assertIn("CHARGE_SHOT_RESTORE_CROSS", self.source)
        self.assertIn(
            "1, 6, 11, 18, 20, 39, 12, 22, 15, 25, 40",
            self.source,
        )
        self.assertIn(
            "CROSS_POWER_ATTACKS[status->active_form]",
            self.source,
        )
        self.assertIn("exe6_navi_status_set(target_side, 4, 0);", self.source)
        self.assertIn("property = 5;", self.source)
        self.assertIn("value = 1;", self.source)


if __name__ == "__main__":
    unittest.main()
