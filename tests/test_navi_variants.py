import unittest
from pathlib import Path
import tomllib


ROOT = Path(__file__).resolve().parents[1]


class NaviVariantTests(unittest.TestCase):
    def test_ex_and_sp_names_are_numbered(self) -> None:
        expected = {
            0x0E1: "ProtoMn2",
            0x0E2: "ProtoMn3",
            0x0E4: "LaserMn2",
            0x0E5: "LaserMn3",
            0x0E7: "ElecMan2",
            0x0E8: "ElecMan3",
            0x0EA: "SlashMn2",
            0x0EB: "SlashMn3",
            0x0ED: "EraseMn2",
            0x0EE: "EraseMn3",
            0x0F0: "NumbrMn2",
            0x0F1: "NumbrMn3",
            0x0F3: "SpoutMn2",
            0x0F4: "SpoutMn3",
            0x0F6: "TmhkMan2",
            0x0F7: "TmhkMan3",
            0x0F9: "TenguMn2",
            0x0FA: "TenguMn3",
            0x0FC: "GrndMan2",
            0x0FD: "GrndMan3",
            0x0FF: "DustMan2",
            0x100: "DustMan3",
            0x102: "BlastMn2",
            0x103: "BlastMn3",
            0x105: "DiveMan2",
            0x106: "DiveMan3",
            0x108: "SerchMn2",
            0x109: "SerchMn3",
            0x10B: "JudgeMn2",
            0x10C: "JudgeMn3",
            0x10E: "ElmntMn2",
            0x10F: "ElmntMn3",
            0x111: "Colonel2",
            0x112: "Colonel3",
            0x114: "Count2",
            0x115: "Count3",
        }

        replacements = {}
        for path in (ROOT / "src").rglob("*.text.toml"):
            definitions = tomllib.loads(path.read_text())
            for entry, value in definitions.get("chip-names-0", {}).items():
                replacements[int(entry, 0)] = value
            for entry, value in definitions.get("chip-names-1", {}).items():
                replacements[0x100 + int(entry, 0)] = value
        self.assertEqual(
            {chip_id: replacements[chip_id] for chip_id in expected},
            expected,
        )

    def test_native_sp_powers_are_fixed_to_table_maximums(self) -> None:
        source = (ROOT / "src/chips/navi-variants.asm").read_text()
        expected = {
            0x0E8: 210,
            0x0EB: 220,
            0x0EE: 210,
            0x0F4: 120,
            0x0F7: 280,
            0x0FA: 160,
            0x0FD: 130,
            0x100: 200,
            0x103: 250,
            0x106: 270,
            0x10C: 190,
            0x10F: 240,
        }
        for chip_id, power in expected.items():
            self.assertIn(f"fix_sp_power 0x{chip_id:03X},{power}", source)

        colonel = (ROOT / "src/chips/colonel.c").read_text()
        count = (ROOT / "src/chips/count.c").read_text()
        self.assertIn("    .power = 300,", colonel)
        self.assertIn(
            "0x115, EXE6_CHIP_CODE_NONE, 4, 89, 0x64, 50, 0x0115,",
            count,
        )
        self.assertNotIn("0x03FA", colonel)
        self.assertNotIn("0x03F9", count)


if __name__ == "__main__":
    unittest.main()
