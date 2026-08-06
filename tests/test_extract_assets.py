import unittest

from extract_assets import decompress_gba_lz77


class ExtractAssetsTests(unittest.TestCase):
    def test_gba_lz77_decompression(self) -> None:
        stream = bytes((0x10, 9, 0, 0, 0x10, ord("A"), ord("B"), ord("C"), 0x30, 0x02))
        self.assertEqual(decompress_gba_lz77(stream), b"ABCABCABC")


if __name__ == "__main__":
    unittest.main()
