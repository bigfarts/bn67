from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class NumberManTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/numberman.c").read_text()

    def test_actor_rejects_hole_before_throw(self) -> None:
        wait = self.source[
            self.source.index("static void actor_wait"):
            self.source.index("static uint8_t roll_die")
        ]
        panel_check = wait.index("exe6_block_move_check(")
        throw_phase = wait.index("ACTOR_PHASE_THROW")
        self.assertLess(panel_check, throw_phase)
        self.assertIn("EXE6_BLOCK_FLAG_SOLID", wait)
        self.assertIn("self->state_word = EXE6_OBJECT_STATE_DESTROY;", wait)


if __name__ == "__main__":
    unittest.main()
