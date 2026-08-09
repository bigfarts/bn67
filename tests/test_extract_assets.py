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

    def test_bn6_ephemeral_tables_include_every_native_entry(self) -> None:
        ephemeral_tables = {
            asset.source: asset
            for asset in ASSETS
            if asset.output.startswith("attack-family1C-table-")
        }
        self.assertEqual(set(ephemeral_tables), {"bn6_gregar", "bn6_falzar"})
        self.assertEqual(ephemeral_tables["bn6_gregar"].offset, 0xED730)
        self.assertEqual(ephemeral_tables["bn6_falzar"].offset, 0xEC3F0)
        for asset in ephemeral_tables.values():
            self.assertEqual(asset.length, 0x17 * 4)

    def test_rollarrow_variant_precedes_asset_kind(self) -> None:
        rollarrow_assets = {
            asset.output for asset in ASSETS if asset.output.startswith("rollarrow")
        }
        for variant in range(1, 4):
            self.assertIn(f"rollarrow{variant}-icon.bin", rollarrow_assets)
            self.assertIn(f"rollarrow{variant}-pal.bin", rollarrow_assets)
        self.assertFalse(
            any(
                output.startswith(("rollarrow-icon-", "rollarrow-pal-"))
                for output in rollarrow_assets
            )
        )

    def test_default_navi_icon_is_not_extracted(self) -> None:
        outputs = {asset.output for asset in ASSETS}
        self.assertNotIn("laserman-icon.bin", outputs)
        self.assertNotIn("searchman-icon.bin", outputs)


if __name__ == "__main__":
    unittest.main()
