from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class StatGuardTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.source = (ROOT / "src/ncps/stat_guard.c").read_text()
        cls.text = (ROOT / "src/ncps/stat_guard.text.toml").read_text()

    def test_replaces_bodypack_through_the_ncp_registry(self) -> None:
        self.assertIn("BN67_FIXED_NCP(\n    0x1C,", self.source)
        self.assertNotIn("BN67_PATCH", self.source)

    def test_is_a_pink_program_part_with_the_native_bug_group(self) -> None:
        declaration = self.source.split("BN67_FIXED_NCP(", 1)[1].split(")", 1)[0]
        self.assertIn(
            "stat_guard_ncp_main,\n    1,\n    0,\n    3,\n    0xFF,\n    0xFF,\n    0xFF",
            declaration,
        )

    def test_sets_only_status_guard(self) -> None:
        self.assertIn(
            "exe6_cur_pet_navi_stats_set(0, STATUS_GUARD_PROPERTY, 1)",
            self.source,
        )
        self.assertIn("#define STATUS_GUARD_PROPERTY 0x52u", self.source)

    def test_uses_distinct_uncompressed_and_compressed_shapes(self) -> None:
        self.assertIn("STAT_GUARD_UNCOMPRESSED_SHAPE", self.source)
        self.assertIn("STAT_GUARD_COMPRESSED_SHAPE", self.source)
        self.assertIn('".byte 0,0,0,1,0,0,0\\n"', self.source)
        self.assertIn('".byte 0,0,1,1,1,0,0\\n"', self.source)

    def test_name_and_description_replace_bodypack_text(self) -> None:
        self.assertIn('stat_guard_program_ncp = "StatGrd"', self.text)
        self.assertIn('"Prevents\\nstatus\\nproblems"', self.text)


if __name__ == "__main__":
    unittest.main()
