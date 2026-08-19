from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class HeatCrossTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/heat_cross.c").read_text()

    def test_routes_heat_cross_b_left_as_a_standalone_attack(self) -> None:
        self.assertIn("#define HEAT_CROSS_B_LEFT_INPUT_GATE 0x08013130", self.source)
        self.assertIn("#define HEAT_CROSS_B_LEFT_INIT_DISPATCH 0x08011796", self.source)
        self.assertIn("heat_cross_b_left_input_gate", self.source)
        self.assertIn("heat_cross_b_left_init_dispatch", self.source)
        self.assertNotIn("FIRE_ARM", self.source)

    def test_dispatches_only_active_heat_cross_to_custom_action(self) -> None:
        self.assertIn("#define HEAT_CROSS_ACTIVE_FORM 1", self.source)
        active = self.source[
            self.source.index("static USED bool heat_cross_active_for_player"):
            self.source.index("static NAKED Exe6Obj *heat_cross_burn_spawn_native")
        ]
        self.assertIn("status->active_form == HEAT_CROSS_ACTIVE_FORM", active)
        action = self.source[
            self.source.index("NAKED void heat_cross_persistent_action_dispatch"):
        ]
        self.assertIn("HEAT_CROSS_B_LEFT_MARKER", action)
        self.assertIn("HEAT_CROSS_PERSISTENT_ACTION_NATIVE", action)

    def test_adds_five_panel_plus_including_center(self) -> None:
        plus = self.source[
            self.source.index("static void heat_cross_spawn_center_burn"):
            self.source.index("static USED uint32_t heat_cross_b_left_init_work")
        ]
        self.assertIn("(int32_t)block_x + front", plus)
        self.assertIn("(int32_t)block_x - front", plus)
        self.assertIn("block_y - 1u", plus)
        self.assertIn("block_y + 1u", plus)
        self.assertEqual(plus.count("heat_cross_try_spawn_burn("), 5)
        center = plus[
            plus.index("static void heat_cross_spawn_center_burn"):
            plus.index("static void heat_cross_spawn_outer_burns")
        ]
        self.assertEqual(center.count("heat_cross_try_spawn_burn("), 1)
        self.assertIn("player->block_x", center)
        self.assertIn("player->block_y", center)

    def test_has_fixed_50_damage_and_native_burnsquare_flames(self) -> None:
        self.assertIn("#define HEAT_CROSS_B_LEFT_DAMAGE 50", self.source)
        self.assertIn("#define HEAT_CROSS_BURN_PARAMETERS 0x00001E04", self.source)
        init = self.source[
            self.source.index("static USED uint32_t heat_cross_b_left_init_work"):
            self.source.index("static USED void heat_cross_b_left_action_update")
        ]
        self.assertIn("HEAT_CROSS_B_LEFT_HIT_PROPERTIES", init)
        self.assertIn("HEAT_CROSS_B_LEFT_DAMAGE", init)
        self.assertIn("work->attack_bonus = 0", init)

    def test_uses_native_minibomb_throw_pose(self) -> None:
        self.assertIn(
            "#define HEAT_CROSS_MINIBOMB_THROW_POSE 0x06",
            self.source,
        )

    def test_spawns_at_native_minibomb_lowered_hand_tick(self) -> None:
        update = self.source[
            self.source.index("static USED void heat_cross_b_left_action_update"):
            self.source.index("NAKED void heat_cross_b_left_input_gate")
        ]
        pose = update.index(
            "set_animation_immediate(player, HEAT_CROSS_MINIBOMB_THROW_POSE)"
        )
        release_tick = update.index("HEAT_CROSS_MINIBOMB_RELEASE_TICK")
        spawn = update.index("heat_cross_spawn_center_burn(player, work)")
        self.assertLess(pose, release_tick)
        self.assertLess(release_tick, spawn)
        self.assertIn("work->timer = 1", update)
        self.assertIn("work->timer < HEAT_CROSS_MINIBOMB_RELEASE_TICK", update)
        self.assertIn("HEAT_CROSS_PHASE_WAIT_FOR_THROW_RELEASE", update)
        self.assertIn("HEAT_CROSS_PHASE_BURNS_ACTIVE", update)
        self.assertNotIn("EXE6_ANIMATION_FRAME_FLAG_END", update)
        self.assertIn("#define HEAT_CROSS_MINIBOMB_RELEASE_TICK 9", self.source)
        self.assertIn(
            "set_animation_immediate(player, HEAT_CROSS_MINIBOMB_THROW_POSE)",
            self.source,
        )

    def test_staggers_outer_flames_by_ten_frames(self) -> None:
        self.assertIn("#define HEAT_CROSS_BURNER_SPREAD_DELAY 10", self.source)
        update = self.source[
            self.source.index("static USED void heat_cross_b_left_action_update"):
            self.source.index("NAKED void heat_cross_b_left_input_gate")
        ]
        center = update.index("heat_cross_spawn_center_burn(player, work)")
        spread_phase = update.index("HEAT_CROSS_PHASE_WAIT_FOR_OUTER_BURNS", center)
        delay = update.index("work->timer < HEAT_CROSS_BURNER_SPREAD_DELAY")
        outer = update.index("heat_cross_spawn_outer_burns(player, work)")
        self.assertLess(center, spread_phase)
        self.assertLess(spread_phase, delay)
        self.assertLess(delay, outer)
        self.assertIn("++work->timer", update[spread_phase:outer])


if __name__ == "__main__":
    unittest.main()
