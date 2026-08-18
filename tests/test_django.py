from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class DjangoTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/django.c").read_text()

    def test_sunlight_duration_uses_only_damage_amount_bits(self) -> None:
        self.assertIn("DAMAGE_AMOUNT_MASK = 0x07FF", self.source)
        sunlight = self.source[
            self.source.index("static void actor_sunlight"):
            self.source.index("static void actor_cooldown")
        ]
        self.assertIn("self->attack & DAMAGE_AMOUNT_MASK", sunlight)
        self.assertNotIn("self->attack & 0x7FFFu", sunlight)

    def test_paralysis_is_consumed_by_first_successful_pulse(self) -> None:
        self.assertIn("DAMAGE_PARALYZE_FLAG = 0x4000", self.source)
        sunlight = self.source[
            self.source.index("static void actor_sunlight"):
            self.source.index("static void actor_cooldown")
        ]
        self.assertIn(
            "spawn_sunlight_damage(self, work->paralysis_pending != 0)",
            sunlight,
        )
        self.assertIn("work->paralysis_pending = 0;", sunlight)

    def test_actor_rejects_hole_before_attack(self) -> None:
        wait = self.source[
            self.source.index("static void actor_wait"):
            self.source.index("static void actor_ready")
        ]
        panel_check = wait.index("exe6_block_move_check(")
        ready_phase = wait.index("ACTOR_PHASE_READY")
        self.assertLess(panel_check, ready_phase)
        self.assertIn("EXE6_BLOCK_FLAG_SOLID", wait)
        self.assertIn("self->state_word = EXE6_OBJECT_STATE_DESTROY;", wait)


if __name__ == "__main__":
    unittest.main()
