from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class LaserManTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/laserman.c").read_text()
        self.abi = (ROOT / "src/abi.h").read_text()

    def test_down_keeps_only_innate_cross_abilities(self) -> None:
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
        for property_id in ("0x23", "0x1B", "0x1C", "0x1D"):
            self.assertIn(
                f"exe6_navi_status_set(target_side, {property_id}, 0);",
                apply_effect,
            )

        refresh = self.source[
            self.source.index("static void refresh_target_player"):
            self.source.index("static void apply_selected_command")
        ]
        self.assertIn("CROSS_INNATE_STATUS_FLAGS[active_cross]", refresh)
        self.assertIn("!cross_supplies_ability", refresh)
        self.assertIn("CROSS_B_LEFTS[active_cross]", refresh)
        self.assertIn("EXE6_HIT_STATUS_FLAG_AIR_SHOES", self.source)
        self.assertIn("EXE6_HIT_STATUS_FLAG_SUPER_ARMOR", self.source)
        self.assertIn("0x10, UINT8_MAX, 0x2A", self.source)
        self.assertIn("exe6_battle_hit_status_flag_off(", refresh)

    def test_right_restores_cross_native_charge_shot(self) -> None:
        self.assertIn("CHARGE_SHOT_RESTORE_CROSS", self.source)
        self.assertIn(
            "1, 6, 11, 18, 20, 39, 12, 22, 15, 25, 40",
            self.source,
        )
        self.assertIn(
            "CROSS_POWER_ATTACKS[active_cross]",
            self.source,
        )
        self.assertIn("exe6_navi_status_set(target_side, 0x04, 0);", self.source)
        self.assertIn("exe6_navi_status_set(target_side, 0x05, 1);", self.source)

    def test_command_effect_requires_confirmed_hp_damage(self) -> None:
        contact = self.source[
            self.source.index("static void check_hit_contact"):
            self.source.index("static void hit_update")
        ]
        self.assertIn("hit->received_hit_flags != 0", contact)
        self.assertIn("work->target_hp_before = target_hp_before;", contact)
        self.assertIn("self->phase = HIT_PHASE_CONFIRM_DAMAGE;", contact)
        self.assertNotIn("apply_selected_command(self);", contact)

        confirm = self.source[
            self.source.index("static void confirm_hit_damage"):
            self.source.index("static void check_hit_contact")
        ]
        self.assertIn("target->hp < work->target_hp_before", confirm)
        self.assertIn("apply_selected_command(self);", confirm)
        self.assertIn("work->confirm_timer == 0", confirm)


if __name__ == "__main__":
    unittest.main()
