from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]


class BeastTimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = (ROOT / "src/ncps/beast_time.c").read_text()
        cls.text = (ROOT / "src/ncps/beast_time.text.toml").read_text()

    def test_replaces_millions_without_a_runtime_order_hook(self) -> None:
        self.assertIn("BN67_NCP(\n    0x16,", self.source)
        self.assertNotIn("order_pack", self.source)
        self.assertNotIn("BN67_PATCH", self.source)

    def test_has_three_plus_part_colors_and_emotion_bug(self) -> None:
        declaration = self.source.split("BN67_NCP(", 1)[1].split(")", 1)[0]
        self.assertIn(
            "beast_time_ncp_main,\n    2,\n    1,\n    1,\n    3,\n    2,\n    0xFF",
            declaration,
        )

    def test_both_shapes_are_the_same_centered_two_by_two_square(self) -> None:
        self.assertEqual(self.source.count("BEAST_TIME_SHAPE\n);"), 2)
        self.assertIn(
            '".byte 0,0,1,1,0,0,0\\n" \\\n'
            '    ".byte 0,0,1,1,0,0,0\\n"',
            self.source,
        )

    def test_each_piece_adds_one_beast_out_turn(self) -> None:
        self.assertIn(
            "EXE6_EXPORT_NCP(main, BN67_JOIN(main, _fn))",
            (ROOT / "src/runtime.h").read_text(),
        )
        self.assertIn(
            "exe6_cur_pet_navi_stats_get(0, BEAST_TIME_BONUS_PROPERTY)",
            self.source,
        )
        self.assertIn("bonus + 1", self.source)

    def test_battle_initializer_adds_bonus_to_native_three_turn_base(self) -> None:
        self.assertIn("USED void beast_time_counter_init(void)", self.source)
        self.assertIn(
            "uint32_t turns = BEAST_OUT_BASE_TURNS +",
            self.source,
        )
        self.assertIn(
            "exe6_cur_pet_navi_stats_set(navi, BEAST_OUT_TURNS_PROPERTY, turns)",
            self.source,
        )
        hooks = (ROOT / "src/ncps/beast_time.asm").read_text()
        self.assertIn(".org 0x080141AC", hooks)
        self.assertIn("ldr r0,=beast_time_counter_init + 1", hooks)
        self.assertIn(".org 0x0800B1A2", hooks)
        self.assertIn("bl beast_time_link_counter_veneer", hooks)

    def test_repurposes_the_entire_millions_byte(self) -> None:
        self.assertNotIn("nibble", self.source)
        self.assertNotIn("BEAST_TIME_BONUS_STEP", self.source)
        hooks = (ROOT / "src/ncps/beast_time.asm").read_text()
        self.assertIn(".org 0x0809FCAA", hooks)
        self.assertIn("Millions' old field-reward check", hooks)

    def test_beast_hooks_live_with_the_ncp(self) -> None:
        common = (ROOT / "src/common.asm").read_text()
        self.assertIn('.include "src/ncps/beast_time.asm"', common)
        global_hooks = (ROOT / "src/hooks.asm").read_text()
        self.assertNotIn("beast_time", global_hooks)

    def test_name_and_description_replace_millions_text(self) -> None:
        self.assertIn('"0x16" = "BeastT+1"', self.text)
        self.assertNotIn("beast_time_program_ncp =", self.text)
        self.assertIn('"BeastOut\\neffect\\n+1 turn"', self.text)

    def test_patch_readmes_document_beast_time_instead_of_forced_five(self) -> None:
        root_readme = (ROOT / "README.md").read_text()
        self.assertIn("BeastT+1", root_readme)
        packaged_readme = (ROOT / "tangopatch/README.md").read_text()
        self.assertIn("Beast Time +1", packaged_readme)
        for readme in (root_readme, packaged_readme):
            self.assertIn("Millions", readme)
            self.assertRegex(readme, re.compile(r"2x2\s+square"))
            self.assertNotIn("five Beast Out turns", readme)


if __name__ == "__main__":
    unittest.main()
