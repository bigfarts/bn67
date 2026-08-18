from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class StatGuardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = (ROOT / "src/ncps/status_guard.c").read_text()
        cls.text = (ROOT / "src/ncps/status_guard.text.toml").read_text()

    def test_replaces_bodypack_through_the_ncp_registry(self) -> None:
        self.assertIn("BN67_NCP(\n    0x1C,", self.source)
        self.assertNotIn("BN67_PATCH", self.source)

    def test_is_a_pink_program_part_with_the_native_bug_group(self) -> None:
        declaration = self.source.split("BN67_NCP(", 1)[1].split(")", 1)[0]
        self.assertIn(
            "status_guard_ncp_main,\n    1,\n    0,\n    3,\n    0xFF,\n    0xFF,\n    0xFF",
            declaration,
        )

    def test_sets_only_status_guard(self) -> None:
        self.assertIn(
            "exe6_cur_pet_navi_stats_set(0, STATUS_GUARD_PROPERTY, 1)",
            self.source,
        )
        self.assertIn("#define STATUS_GUARD_PROPERTY 0x52u", self.source)

    def test_uninstall_and_sunmoon_bluemoon_remove_status_guard(self) -> None:
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertIn(
            "exe6_navi_status_set(player->owner, STATUS_GUARD_PROPERTY, 0)",
            self.source,
        )
        self.assertIn(
            ".org 0x080141BC\n"
            "status_guard_uninstall_veneer:\n"
            "    ldr r0,=status_guard_uninstall_main + 1",
            hooks,
        )
        self.assertIn(
            ".org 0x0801414A\n    bl status_guard_uninstall_veneer",
            hooks,
        )
        self.assertIn('"ldr r3,=0x0801469D\\n"', self.source)

    def test_uses_distinct_uncompressed_and_compressed_shapes(self) -> None:
        self.assertIn("STATUS_GUARD_UNCOMPRESSED_SHAPE", self.source)
        self.assertIn("STATUS_GUARD_COMPRESSED_SHAPE", self.source)
        self.assertIn('".byte 0,0,0,1,0,0,0\\n"', self.source)
        self.assertIn('".byte 0,0,1,1,1,0,0\\n"', self.source)

    def test_name_and_description_replace_bodypack_text(self) -> None:
        self.assertIn('"0x1C" = "StatGrd"', self.text)
        self.assertNotIn("status_guard_program_ncp =", self.text)
        self.assertIn('"Prevents\\nstatus\\nproblems"', self.text)


if __name__ == "__main__":
    unittest.main()
