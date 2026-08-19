from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class HeatCrossTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/heat_cross.c").read_text()

    def test_hooks_each_editions_heat_cross_fire_arm_return(self) -> None:
        self.assertIn("#define HEAT_CROSS_FIRE_ARM_RETURN 0x080ECD74", self.source)
        self.assertIn("#define HEAT_CROSS_FIRE_ARM_RETURN 0x080EE0B4", self.source)
        self.assertIn("BN67_PATCH_SECTION(", self.source)
        self.assertIn("heat_cross_charge_shot_after_fire_arm", self.source)

    def test_restores_native_work_after_forward_attack_before_added_burns(self) -> None:
        dispatch = self.source[
            self.source.index("NAKED void heat_cross_charge_shot_after_fire_arm"):
        ]
        restore = dispatch.index('"pop {r7}\\n"')
        surrounding = dispatch.index('"bl heat_cross_spawn_surrounding_burns\\n"')
        self.assertLess(restore, surrounding)
        self.assertIn('"movs r0,#0x0a\\n"', dispatch)
        self.assertIn('"strb r0,[r5,#0x10]\\n"', dispatch)

    def test_adds_only_upper_lower_and_rear_adjacent_burns(self) -> None:
        surrounding = self.source[
            self.source.index("static USED void heat_cross_spawn_surrounding_burns"):
            self.source.index("NAKED void heat_cross_charge_shot_after_fire_arm")
        ]
        self.assertIn("block_y - 1u", surrounding)
        self.assertIn("block_y + 1u", surrounding)
        self.assertIn("(int32_t)block_x - front", surrounding)
        self.assertEqual(surrounding.count("heat_cross_try_spawn_burn("), 3)
        self.assertNotIn("(int32_t)block_x + front", surrounding)

    def test_added_burns_reuse_fire_arms_resolved_attack(self) -> None:
        spawn = self.source[
            self.source.index("static void heat_cross_try_spawn_burn"):
            self.source.index("static USED void heat_cross_spawn_surrounding_burns")
        ]
        self.assertIn("work->element", spawn)
        self.assertIn("HEAT_CROSS_BURN_PARAMETERS", spawn)
        self.assertIn("#define HEAT_CROSS_BURN_PARAMETERS 0x00001E04", self.source)
        self.assertIn("work->attack", spawn)
        self.assertIn(
            "EXE6_BLOCK_FLAG_VALID | EXE6_BLOCK_FLAG_SOLID",
            spawn,
        )


if __name__ == "__main__":
    unittest.main()
