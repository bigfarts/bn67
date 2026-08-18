from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class LaserManTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/laserman.c").read_text()
        self.abi = (ROOT / "src/abi.h").read_text()
        self.abi_source = (ROOT / "src/abi.c").read_text()

    def test_down_uses_native_uninstall_path(self) -> None:
        apply_selected = self.source[
            self.source.index("static void apply_selected_command"):
            self.source.index("static bool hit_init")
        ]
        self.assertIn("case LASERMAN_COMMAND_DOWN:", apply_selected)
        self.assertIn("exe6_navi_uninstall(target);", apply_selected)
        self.assertIn("clears B+Left and its live base cache", apply_selected)
        self.assertIn("void exe6_navi_uninstall(Exe6Obj *player);", self.abi)
        self.assertIn("NAKED void exe6_navi_uninstall", self.abi_source)
        self.assertIn('"ldr r4,=0x080140EF\\n"', self.abi_source)

        for property_id in ("0x23", "0x1B", "0x1C", "0x1D", "0x52"):
            self.assertNotIn(
                f"exe6_navi_status_set(target_side, {property_id}, 0);",
                apply_selected,
            )
        self.assertNotIn("exe6_battle_hit_status_flag_off(", self.source)

    def test_command_effects_are_inlined_by_direction(self) -> None:
        self.assertNotIn("enum CommandEffect", self.source)
        self.assertNotIn("COMMAND_STREAMS", self.source)
        self.assertNotIn("static const uint16_t COMMAND_", self.source)

        for direction in ("UP", "DOWN", "RIGHT", "LEFT"):
            self.assertIn(
                f"case LASERMAN_COMMAND_{direction}:",
                self.source,
            )

    def test_direction_hit_delays_match_original_stream_timing(self) -> None:
        delay = self.source[
            self.source.index("static uint8_t command_hit_delay"):
            self.source.index("static void beam_command_tick")
        ]
        self.assertIn("case LASERMAN_COMMAND_UP:\n        return 3;", delay)
        self.assertIn("case LASERMAN_COMMAND_DOWN:\n        return 5;", delay)
        self.assertIn("case LASERMAN_COMMAND_RIGHT:", delay)
        self.assertIn("case LASERMAN_COMMAND_LEFT:\n        return 1;", delay)

    def test_status_writes_use_named_properties(self) -> None:
        properties = {
            "ATTACK": "0x01u",
            "RAPID": "0x02u",
            "CHARGE": "0x03u",
            "B_BUTTON": "0x04u",
            "POWER_ATTACK": "0x05u",
            "CUSTOM_LEVEL": "0x0Au",
        }
        for name, value in properties.items():
            self.assertIn(f"#define NAVI_PROPERTY_{name} {value}", self.source)
        self.assertNotRegex(
            self.source,
            r"exe6_navi_status_set\([^;]+,\s*0x[0-9A-Fa-f]+,",
        )

    def test_right_restores_cross_native_charge_shot(self) -> None:
        self.assertIn(
            "1, 6, 11, 18, 20, 39, 12, 22, 15, 25, 40",
            self.source,
        )
        self.assertIn(
            "CROSS_POWER_ATTACKS[active_cross]",
            self.source,
        )
        self.assertIn(
            "exe6_navi_status_set(target_side, NAVI_PROPERTY_B_BUTTON, 0);",
            self.source,
        )
        self.assertIn(
            "NAVI_PROPERTY_POWER_ATTACK, 1);",
            self.source,
        )
        self.assertIn("bool restore_base", self.source)

    def test_command_requires_confirmed_hp_damage(self) -> None:
        initialize = self.source[
            self.source.index("static bool hit_init"):
            self.source.index("static void close_hit")
        ]
        self.assertIn(
            "work->target_hp_before = target == NULL ? 0 : target->hp;",
            initialize,
        )

        contact = self.source[
            self.source.index("static void check_hit_contact"):
            self.source.index("static void hit_update")
        ]
        self.assertIn("hit->received_hit_flags != 0", contact)
        self.assertIn("close_hit(self);", contact)
        self.assertNotIn("target_hp_before =", contact)
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
