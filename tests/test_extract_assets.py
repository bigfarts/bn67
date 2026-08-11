import unittest

from extract_assets import (
    ASSETS,
    decode_4bpp_tiles,
    decompress_gba_lz77,
    encode_4bpp_tiles,
    normalize_exe6_icon_border,
)


class ExtractAssetsTests(unittest.TestCase):
    def test_gba_lz77_decompression(self) -> None:
        stream = bytes((0x10, 9, 0, 0, 0x10, ord("A"), ord("B"), ord("C"), 0x30, 0x02))
        self.assertEqual(decompress_gba_lz77(stream), b"ABCABCABC")

    def test_exe6_summon_tables_include_delta_ray(self) -> None:
        summon_tables = {
            asset.source: asset
            for asset in ASSETS
            if asset.output.startswith("attack-family1B-table-")
        }
        self.assertEqual(set(summon_tables), {"exe6_gregar", "exe6_falzar"})
        for asset in summon_tables.values():
            self.assertEqual(asset.offset, 0x2CD5C)
            self.assertEqual(asset.length, 0x1D * 4)

    def test_exe6_ephemeral_tables_include_every_native_entry(self) -> None:
        ephemeral_tables = {
            asset.source: asset
            for asset in ASSETS
            if asset.output.startswith("attack-family1C-table-")
        }
        self.assertEqual(set(ephemeral_tables), {"exe6_gregar", "exe6_falzar"})
        self.assertEqual(ephemeral_tables["exe6_gregar"].offset, 0xED730)
        self.assertEqual(ephemeral_tables["exe6_falzar"].offset, 0xEC3F0)
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

    def test_exe6_icon_border_uses_shared_palette_index(self) -> None:
        pixels = [[3] * 16 for _ in range(16)]
        for coordinate in range(16):
            pixels[0][coordinate] = 9
            pixels[15][coordinate] = 9
            pixels[coordinate][0] = 9
            pixels[coordinate][15] = 9
        icon = normalize_exe6_icon_border(encode_4bpp_tiles(pixels))
        normalized = decode_4bpp_tiles(icon, 16, 16)

        self.assertTrue(all(pixel == 9 for pixel in normalized[0]))
        self.assertTrue(all(pixel == 9 for pixel in normalized[15]))
        for coordinate in range(1, 15):
            self.assertEqual(normalized[1][coordinate], 5)
            self.assertEqual(normalized[14][coordinate], 5)
            self.assertEqual(normalized[coordinate][1], 5)
            self.assertEqual(normalized[coordinate][14], 5)
        self.assertEqual(normalized[2][2], 3)


if __name__ == "__main__":
    unittest.main()
