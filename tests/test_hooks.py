import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class GlobalHookTests(unittest.TestCase):
    def test_link_battles_start_with_five_beast_turns(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x0800B1A2\n    mov r0,5$"),
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
            re.compile(r"(?m)^\.org 0x08013EA8\n    \.dw 0x08013EE7$"),
        )
        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x08013EB8\n    \.dw 0x08013F15$"),
        )


if __name__ == "__main__":
    unittest.main()
