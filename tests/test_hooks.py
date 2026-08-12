import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class GlobalHookTests(unittest.TestCase):
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
