from pathlib import Path
import unittest

from extract_assets import ASSETS, BN3_ROOK_ID


ROOT = Path(__file__).resolve().parents[1]


class RookTests(unittest.TestCase):
    def test_replaces_attack_plus_ten_with_requested_codes(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x0c0)", source)
        codes = source[source.index(".codes = {"):source.index(".attack_element")]
        self.assertEqual(codes.count("EXE6_CHIP_CODE_"), 4)
        for code in ("D", "N", "U", "ASTERISK"):
            self.assertIn(f"EXE6_CHIP_CODE_{code}", codes)
        self.assertNotIn("EXE6_CHIP_CODE_NONE", codes)
        self.assertNotIn("BN67_CHIP_RECORD(0x0bc)", source)

    def test_retains_bn3_rook_stats_and_break_only_damage(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("static const uint16_t ROOK_HP = 500;", source)
        self.assertIn(".mb = 30", source)
        self.assertIn(".element = EXE6_CHIP_ELEMENT_OBSTACLE", source)
        self.assertIn("EXE6_HIT_TYPE_FLAG_GUARD_PIERCING", source)
        self.assertIn("exe6_enemy_life_sub(damage);", source)
        self.assertIn("static const uint16_t ROOK_LIFETIME = 0x0708;", source)

    def test_uses_bn3_rook_art_and_battle_sprite(self) -> None:
        self.assertEqual(BN3_ROOK_ID, 0x99)
        sprite = next(
            asset for asset in ASSETS if asset.output == "rook-battle-sprite.bin"
        )
        self.assertEqual(sprite.source, "bn3_blue")
        self.assertEqual(sprite.offset, 0x2CD434)
        self.assertEqual(sprite.length, 0x20A0)

    def test_dims_places_with_native_cue_and_registers_dust_ammo(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("EXE6_CHIP_EFFECT_FLAG_DIMMING", source)
        self.assertIn("BN67_EFFECT(rook_controller_main)", source)
        self.assertIn("exe6_event_chip_common_telop();", source)
        self.assertIn("static const uint16_t STARTUP_TICKS = 3;", source)
        self.assertIn("exe6_block_move_check(", source)
        self.assertIn("BN67_USE_SONG(signalred_spawn_song);", source)
        self.assertIn(
            "exe6_sound_req(BN67_SONG_ID(signalred_spawn_song));",
            source,
        )
        self.assertIn("static const uint32_t SPAWN_BLOB_EFFECT = 0x15;", source)
        self.assertIn("obj_show_spawn_blob(obj);", source)
        self.assertNotIn("SUMMON_ANIMATION_FLAG", source)
        self.assertIn("exe6_cube_guard_mark_check();", source)
        self.assertIn("hit->damage_buckets[i] = 0;", source)
        self.assertNotIn("rook_guard_mark_main", source)
        self.assertNotIn("EXE6_HIT_EFFECT_PING", source)
        self.assertIn("BN67_DUST_SPRITE(rook_battle_sprite);", source)
        self.assertIn(
            "exe6_cube_set_dust_suikomi_efc(BN67_DUST_KIND(rook_battle_sprite));",
            source,
        )
        self.assertIn("static const uint8_t ROOK_ANIMATION = 4;", source)
        self.assertIn("exe6_obj_char_init(\n        0x80,", source)
        self.assertIn("ROOK_ANIMATION | (ROOK_ANIMATION << 8)", source)
        self.assertIn("exe6_obj_dma_seq_set(ROOK_ANIMATION);", source)
        self.assertIn("exe6_obj_clt_set(0);", source)
        self.assertIn("uint8_t owner_side = owner->owner;", source)
        self.assertIn("obj->work[SPAWNER_SIDE_WORK] = owner_side;", source)
        self.assertIn(
            "exe6_obj_flip_set(obj->work[SPAWNER_SIDE_WORK] ^ 1u);",
            source,
        )
        self.assertIn("exe6_cube_entry(obj, owner_side, 0);", source)

    def test_native_spawn_animations_are_visible_without_effect_overlay(self) -> None:
        rook = (ROOT / "src/chips/rook.c").read_text()
        rook_success = rook[
            rook.index("static void obj_normal_update"):
            rook.index("static void obj_store_dust_ammo")
        ]
        self.assertNotIn("obj_show_spawn_blob(obj);", rook_success)
        rook_init = rook[
            rook.index("static void obj_init"):
            rook.index("static Exe6Obj *spawn_persistent")
        ]
        self.assertIn("obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;", rook_init)

        source = (ROOT / "src/chips/signalred.c").read_text()
        self.assertIn("static const uint32_t SPAWN_BLOB_EFFECT = 0x15;", source)
        success = source[
            source.index("static void obj_normal_update"):
            source.index("static void obj_store_dust_ammo")
        ]
        self.assertNotIn("obj_show_spawn_blob(obj);", success)
        self.assertIn("play_spawn_sound(obj);", success)
        init = source[
            source.index("static void obj_init"):
            source.index("static Exe6Obj *spawn_persistent")
        ]
        self.assertIn("obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;", init)

    def test_text_identifies_rook(self) -> None:
        text = (ROOT / "src/chips/rook.text.toml").read_text()
        self.assertIn('"0xC0" = "Rook"', text)
        self.assertIn('"0xC0" = "Protects\\nyou from\\nattacks"', text)


if __name__ == "__main__":
    unittest.main()
