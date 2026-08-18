from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]


class BugDeathThunderTests(unittest.TestCase):
    def setUp(self) -> None:
        self.source = (ROOT / "src/chips/bug_death_thunder.asm").read_text()

    def test_all_charge_levels_take_sixty_frames(self) -> None:
        self.assertRegex(
            self.source,
            re.compile(r"(?m)^\.org 0x08020558\n    \.dh 60,60,60,60,60$"),
        )

    def test_post_fire_chip_lockout_is_ten_frames_in_both_editions(self) -> None:
        self.assertRegex(
            self.source,
            re.compile(
                r"(?m)^\.if falzar\n"
                r"    \.org 0x080EC826\n"
                r"\.else\n"
                r"    \.org 0x080EDB66\n"
                r"\.endif\n"
                r"    mov r0,10$"
            ),
        )

    def test_patch_is_included_by_both_rom_builds(self) -> None:
        common = (ROOT / "src/common.asm").read_text()

        self.assertIn('.include "src/chips/bug_death_thunder.asm"', common)


if __name__ == "__main__":
    unittest.main()
