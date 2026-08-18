import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class GlobalHookTests(unittest.TestCase):
    def test_link_battles_add_installed_piece_bonus_without_forced_five(self) -> None:
        hooks = (ROOT / "src/ncps/beast_time.asm").read_text()

        self.assertNotIn("mov r0,5", hooks)
        self.assertRegex(
            hooks,
            re.compile(
                r"(?m)^\.org 0x0800B1A2\n"
                r"    bl beast_time_link_counter_veneer\n"
                r"    nop$"
            ),
        )

    def test_beast_out_initializer_uses_installed_piece_bonus(self) -> None:
        hooks = (ROOT / "src/ncps/beast_time.asm").read_text()

        self.assertIn(".org 0x080141AC", hooks)
        self.assertIn("ldr r0,=beast_time_counter_init + 1", hooks)

    def test_repurposed_millions_stat_does_not_grant_millions_reward(self) -> None:
        hooks = (ROOT / "src/ncps/beast_time.asm").read_text()

        self.assertRegex(
            hooks,
            re.compile(
                r"(?m)^\.org 0x0809FCAA\n"
                r"    mov r5,0\n"
                r"    nop\n"
                r"    nop$"
            ),
        )

    def test_aquaneedle_keeps_stagger_without_fixed_invulnerability(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertRegex(
            hooks,
            re.compile(
                r"(?m)^\.if falzar\n"
                r"    \.org 0x080CE5BE\n"
                r"\.else\n"
                r"    \.org 0x080CFE2E\n"
                r"\.endif\n"
                r"    mov r3,0x01$"
            ),
        )

    def test_replaced_add_on_chips_are_not_consumed(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x08029238\n    nop$"),
        )
        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x0802923C\n    nop$"),
        )

    def test_status_bug_replaces_only_green_invulnerability_entries(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertNotRegex(hooks, r"(?m)^\.org 0x08013E7E$")
        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x08013EA4\n    \.dw 0x08013EC3$"),
        )
        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x08013EB4\n    \.dw 0x08013EF1$"),
        )
        self.assertNotRegex(hooks, r"(?m)^\.org 0x08013EA8$")
        self.assertNotRegex(hooks, r"(?m)^\.org 0x08013EB8$")

    def test_status_guard_is_not_a_raw_hook(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertNotIn("StatGrd", hooks)
        self.assertNotIn("0x0813CA4C", hooks)
        self.assertNotIn("0x0813E82C", hooks)


if __name__ == "__main__":
    unittest.main()
