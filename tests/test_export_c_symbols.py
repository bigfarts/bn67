from pathlib import Path
import subprocess
import unittest
from unittest.mock import patch

from export_c_symbols import symbols


class ExportCSymbolTests(unittest.TestCase):
    @patch("export_c_symbols.subprocess.run")
    def test_exports_lowercase_snake_case_symbols(self, run) -> None:
        run.return_value = subprocess.CompletedProcess(
            [],
            0,
            stdout="08801234 T chaos_lord_attack_main\n",
        )
        self.assertEqual(
            symbols("arm-none-eabi-nm", Path("gameplay.elf")),
            [(0x08801234, "chaos_lord_attack_main")],
        )

    @patch("export_c_symbols.subprocess.run")
    def test_rejects_non_snake_case_symbols(self, run) -> None:
        run.return_value = subprocess.CompletedProcess(
            [],
            0,
            stdout="08801234 T ChaoslordAttackMain\n",
        )
        with self.assertRaisesRegex(ValueError, "must be snake_case"):
            symbols("arm-none-eabi-nm", Path("gameplay.elf"))


if __name__ == "__main__":
    unittest.main()
