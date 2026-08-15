from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class VarSwordTests(unittest.TestCase):
    def test_adds_bn3_element_sonic_command_for_both_sides(self) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()

        self.assertIn(
            '".hword 0x0003,0x0003,0x0021,0x0081,0x0041,0\\n"',
            source,
        )
        self.assertIn(
            '".hword 0x0003,0x0003,0x0011,0x0081,0x0041,0\\n"',
            source,
        )
        self.assertIn(".hword 0x0173", source)

    def test_expands_native_command_table_without_changing_input_window(
        self,
    ) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()

        self.assertEqual(source.count('"movs r1,#24\\n"'), 1)
        self.assertEqual(source.count('"movs r0,#24\\n"'), 1)
        self.assertNotIn("0x080F098A", source)
        self.assertNotIn("0x080EF64A", source)

    def test_element_waves_cycle_fire_aqua_elec_wood(self) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()
        dispatch = source[source.index("NAKED void varsword_element_wave_dispatch"):]

        self.assertIn('"movs r2,#5\\n"', dispatch)
        self.assertIn('"ldrh r3,[r7,#0x12]\\n"', dispatch)
        self.assertIn('"subs r2,r2,r3\\n"', dispatch)
        self.assertIn('"movs r3,#0x30\\n"', dispatch)
        self.assertIn('"ldrb r3,[r7,r3]\\n"', dispatch)

    def test_only_element_sonic_repeats_four_times(self) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()
        init = source[source.index("NAKED void varsword_sonic_boom_init"):]

        self.assertIn('"movs r1,#1\\n"', init)
        self.assertIn('"movs r0,#0x30\\n"', init)
        self.assertIn('"ldrb r0,[r7,r0]\\n"', init)
        self.assertIn('"cmp r0,#10\\n"', init)
        self.assertIn('"adds r1,#3\\n"', init)
        self.assertIn('"strh r1,[r7,#0x12]\\n"', init)
        self.assertIn('"mov pc,lr\\n"', init)

    def test_keeps_all_varsword_patch_assembly_with_its_c_package(self) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()
        hooks = (ROOT / "src/hooks.asm").read_text()

        self.assertNotIn("VarSword", hooks)
        self.assertNotIn("varsword_", hooks)
        self.assertIn(
            "BN67_PATCH_THUMB_POINTER(0x080F0CC4, varsword_sonic_boom_init)",
            source,
        )
        self.assertIn(
            "BN67_PATCH_THUMB_POINTER(0x080EF984, varsword_sonic_boom_init)",
            source,
        )
        self.assertEqual(source.count("BN67_PATCH_SECTION("), 10)

    def test_only_final_element_wave_requests_flashing_invulnerability(self) -> None:
        source = (ROOT / "src/chips/varsword.c").read_text()
        finalize = source[source.index("NAKED void varsword_sonic_shell_finalize"):]
        modifier = source[source.index("NAKED void varsword_sonic_hit_modifier"):]

        self.assertIn('"ldrh r1,[r7,#0x12]\\n"', finalize)
        self.assertIn('"cmp r1,#1\\n"', finalize)
        self.assertIn('"strb r1,[r0,#0x0f]\\n"', finalize)
        self.assertIn('"ldrb r3,[r5,#0x0f]\\n"', modifier)
        self.assertIn('"movs r3,#1\\n"', modifier)
        self.assertIn('"ldrb r3,[r5,#4]\\n"', modifier)
        self.assertIn("BN67_PATCH_SECTION(0x080D10EE", source)
        self.assertIn("BN67_PATCH_SECTION(0x080CF87E", source)


if __name__ == "__main__":
    unittest.main()
