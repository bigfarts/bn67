import re
import tomllib
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
            re.compile(r"(?m)^\.org 0x08013EA4\n    \.dw 0x08013EC3$"),
        )
        self.assertRegex(
            hooks,
            re.compile(r"(?m)^\.org 0x08013EB4\n    \.dw 0x08013EF1$"),
        )
        self.assertNotRegex(hooks, r"(?m)^\.org 0x08013EA8$")
        self.assertNotRegex(hooks, r"(?m)^\.org 0x08013EB8$")

    def test_bodypack_provides_only_status_guard(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertRegex(
            hooks,
            re.compile(
                r"(?m)^\.if falzar\n"
                r"    \.org 0x0813CA4C\n"
                r"\.else\n"
                r"    \.org 0x0813E82C\n"
                r"\.endif\n"
                r"    push \{lr\}\n"
                r"    mov r0,0\n"
                r"    mov r1,0x52\n"
                r"    mov r2,1\n"
                r"    bl 0x0801379E\n"
                r"    pop \{pc\}\n"
                r"    nop\n"
                r"    nop\n"
                r"    nop$"
            ),
        )

    def test_bodypack_is_presented_as_status_guard(self) -> None:
        text_patch = tomllib.loads((ROOT / "src/hooks.text.toml").read_text())

        self.assertEqual(text_patch["ncp-names"]["0x1C"], "StatGrd")
        self.assertEqual(
            text_patch["ncp-descriptions"]["0x1C"],
            "Prevents\nstatus\nproblems",
        )


if __name__ == "__main__":
    unittest.main()
