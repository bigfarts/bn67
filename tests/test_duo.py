from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class DuoTests(unittest.TestCase):
    def test_spawn_rejects_player_in_hidden_panel_footprint(self) -> None:
        source = (ROOT / "src/chips/duo.c").read_text()
        occupancy = source[
            source.index("static bool entrance_has_player"):
            source.index("static void begin_entry")
        ]
        self.assertIn("uint32_t first_x = owner * 4u + 1u;", occupancy)
        self.assertIn("block_x < first_x + 2u", occupancy)
        self.assertIn("block_y <= 3", occupancy)
        self.assertIn("EXE6_BLOCK_FLAG_SIDE_1_HIT", occupancy)
        self.assertIn("EXE6_BLOCK_FLAG_SIDE_0_HIT", occupancy)
        self.assertIn("exe6_block_move_check(", occupancy)
        self.assertIn("return true;", occupancy)

        attack = source[source.index("BN67_SUMMON_ATTACK(0x133") :]
        gate = attack.index("if (entrance_has_player(owner->owner))")
        spawn = attack.index("Exe6Obj *actor = exe6_em_open(")
        self.assertLess(attack.index("break_obstacles();"), gate)
        self.assertLess(gate, spawn)


if __name__ == "__main__":
    unittest.main()
