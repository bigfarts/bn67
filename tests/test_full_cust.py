from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class FullCustTests(unittest.TestCase):
    def test_retains_native_record_with_51_mb_cost(self) -> None:
        source = (ROOT / "src/chips/full_custom.c").read_text()

        self.assertIn("BN67_CHIP_RECORD(0x0ae)", source)
        self.assertIn(".mb = 51", source)
        self.assertIn("EXE6_CHIP_CODE_ASTERISK", source)
        self.assertIn(".family = 0x1C", source)
        self.assertIn(".subfamily = 0x05", source)


if __name__ == "__main__":
    unittest.main()
