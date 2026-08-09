import unittest

from extract_assets import ASSETS, decompress_gba_lz77


class ExtractAssetsTests(unittest.TestCase):
    def test_gba_lz77_decompression(self) -> None:
        stream = bytes((0x10, 9, 0, 0, 0x10, ord("A"), ord("B"), ord("C"), 0x30, 0x02))
        self.assertEqual(decompress_gba_lz77(stream), b"ABCABCABC")

    def test_bn6_summon_tables_include_delta_ray(self) -> None:
        summon_tables = {
            asset.source: asset
            for asset in ASSETS
            if asset.output.startswith("attack-family1B-table-")
        }
        self.assertEqual(set(summon_tables), {"bn6_gregar", "bn6_falzar"})
        for asset in summon_tables.values():
            self.assertEqual(asset.offset, 0x2CD5C)
            self.assertEqual(asset.length, 0x1D * 4)


if __name__ == "__main__":
    unittest.main()
