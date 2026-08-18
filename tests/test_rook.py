from pathlib import Path
import unittest

from extract_assets import ASSETS, BN3_ROOK_ID


ROOT = Path(__file__).resolve().parents[1]


class RookTests(unittest.TestCase):
    def test_replaces_attack_plus_ten_with_asterisk_code_only(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("BN67_CHIP_RECORD(0x0c0)", source)
        codes = source[source.index(".codes = {"):source.index(".attack_element")]
        self.assertEqual(codes.count("EXE6_CHIP_CODE_"), 4)
        self.assertEqual(codes.count("EXE6_CHIP_CODE_ASTERISK"), 1)
        self.assertEqual(codes.count("EXE6_CHIP_CODE_NONE"), 3)
        for code in ("D", "N", "U"):
            self.assertNotIn(f"EXE6_CHIP_CODE_{code},", codes)
        self.assertNotIn("BN67_CHIP_RECORD(0x0bc)", source)

    def test_retains_bn3_rook_stats_and_break_only_damage(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("static const uint16_t ROOK_HP = 500;", source)
        self.assertIn(".mb = 30", source)
        self.assertIn(".element = EXE6_CHIP_ELEMENT_OBSTACLE", source)
        self.assertIn("EXE6_HIT_TYPE_FLAG_GUARD_PIERCING", source)
        self.assertIn("exe6_enemy_life_sub(damage);", source)
        self.assertIn("static const uint16_t ROOK_LIFETIME = 0x0708;", source)

    def test_airshot_and_rackets_push_one_panel_without_damage(
        self,
    ) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn(
            "static const uint8_t AIRSHOT_HIT_MODIFIER = 0x61;",
            source,
        )
        self.assertIn(
            "static const uint8_t RACKET_HIT_MODIFIER = 0x49;",
            source,
        )
        self.assertIn(
            "modifier == AIRSHOT_HIT_MODIFIER ||",
            source,
        )
        self.assertIn("modifier == RACKET_HIT_MODIFIER;", source)
        self.assertIn(
            "int32_t direction = -(int32_t)exe6_calc_pl_em_dir_spd_for(obj);",
            source,
        )
        self.assertIn("(int32_t)obj->block_x + direction", source)
        self.assertIn("obj_block_damage(hit);", source)
        self.assertIn("obj->target_block_x = (uint8_t)block_x;", source)

    def test_push_uses_native_airshot_knockback_speed(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn(
            "static const int32_t AIRSHOT_PUSH_SPEED = 0x000A0000;",
            source,
        )
        self.assertIn(
            "obj->velocity_x = direction * AIRSHOT_PUSH_SPEED;",
            source,
        )
        push = source[
            source.index("static void obj_push_update"):
            source.index("static void obj_update")
        ]
        self.assertIn("int32_t next_x = obj->x + obj->velocity_x;", push)
        self.assertIn("obj->x = target_x;", push)
        self.assertIn("obj->block_x = obj->target_block_x;", push)
        self.assertIn("exe6_pos_to_block();", push)
        self.assertIn("exe6_battle_hit_block_pos_set();", push)

    def test_push_attacks_destroy_rook_at_occupied_panel(
        self,
    ) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        push = source[
            source.index(
                "static enum PushStartResult obj_begin_one_panel_push"
            ):
            source.index("static void obj_update")
        ]
        self.assertIn("EXE6_BLOCK_FLAG_SOLID", push)
        self.assertIn("return;", push)
        for flag in (
            "EXE6_BLOCK_FLAG_SUPPORT_OBJECT",
            "EXE6_BLOCK_FLAG_SIDE_1_NAVI",
            "EXE6_BLOCK_FLAG_SIDE_0_NAVI",
            "EXE6_BLOCK_FLAG_SIDE_1_HIT",
            "EXE6_BLOCK_FLAG_SIDE_0_HIT",
        ):
            self.assertIn(flag, source)
        handling = source[
            source.index("if (obj_received_push_attack(hit))"):
            source.index("} else if (damage != 0)")
        ]
        self.assertIn("obj_block_damage(hit);", handling)
        self.assertIn(
            "enum PushStartResult push = obj_begin_one_panel_push(obj);",
            handling,
        )
        self.assertIn("if (push == PUSH_START_BLOCKED)", handling)
        self.assertIn("obj_begin_damage_destroy(obj);", handling)
        self.assertIn("return;", handling)

        occupancy = push[
            push.index("uint32_t block_flags"):
            push.index("obj->target_block_x")
        ]
        self.assertIn("return PUSH_START_BLOCKED;", occupancy)

        invalid_panel = push[
            push.index("if (exe6_block_move_check("):
            push.index("uint32_t block_flags")
        ]
        self.assertIn("return PUSH_START_NONE;", invalid_panel)
        self.assertNotIn("PUSH_START_BLOCKED", invalid_panel)

    def test_uses_bn3_rook_art_and_battle_sprite(self) -> None:
        self.assertEqual(BN3_ROOK_ID, 0x99)
        sprite = next(
            asset for asset in ASSETS if asset.output == "rook_battle_sprite.bin"
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
        self.assertIn("bn67_deployable_placement_check(", source)
        self.assertIn("BN67_USE_SONG(signal_red_spawn_song);", source)
        self.assertIn(
            "exe6_sound_req(BN67_SONG_ID(signal_red_spawn_song));",
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
        self.assertIn("BN67_FIELD_OBJECT(rook_battle_sprite, 4, 0, 1);", source)
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
        self.assertIn("exe6_cube_entry(obj, obj->owner, 0);", source)
        self.assertIn(
            "obj->name_id = (uint16_t)BN67_FIELD_OBJECT_ID(rook_battle_sprite);",
            source,
        )

    def test_junkman_and_blizzardball_use_allocated_field_object_ids(self) -> None:
        abi = (ROOT / "src/abi.h").read_text()
        self.assertIn("uint16_t name_id;", abi)
        self.assertIn(
            "EXE6_HIT_SECONDARY_FLAG_FIELD_OBJECT_REMOVAL 0x00008000",
            abi,
        )

        expectations = (
            ("rook.c", "rook_battle_sprite", 4),
            ("signal_red.c", "signal_red_battle_sprite", 0),
        )
        for filename, archive, animation in expectations:
            source = (ROOT / "src/chips" / filename).read_text()
            self.assertIn(
                f"BN67_FIELD_OBJECT({archive}, {animation}, 0, 1);",
                source,
            )
            self.assertIn(
                f"obj->name_id = (uint16_t)BN67_FIELD_OBJECT_ID({archive});",
                source,
            )
            self.assertIn(
                "EXE6_HIT_SECONDARY_FLAG_FIELD_OBJECT_REMOVAL",
                source,
            )
            self.assertIn("static bool obj_handle_removal_request", source)
            self.assertIn("uint32_t erase_result = exe6_cube_erase2();", source)
            self.assertIn("if (erase_result == 2)", source)
            self.assertIn("FIELD_OBJECT_REMOVAL_EFFECT = 0x14", source)
            self.assertIn(
                "FIELD_OBJECT_REMOVAL_EFFECT_HEIGHT = 0x000C0000",
                source,
            )

            update = source[
                source.index("static void obj_update"):
                source.index("static void obj_init")
            ]
            removal_request = update.index(
                "if (obj_handle_removal_request(obj))"
            )
            event_pause = update.rindex("if (exe6_battle_event_busy_check()")
            self.assertLess(removal_request, event_pause)

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

        source = (ROOT / "src/chips/signal_red.c").read_text()
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

    def test_lifespan_refreshes_visibility_before_applying_each_blink(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        update = source[
            source.index("static void obj_update"):
            source.index("static void obj_init")
        ]
        refresh = update.index(
            "obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;"
        )
        lifespan = update.index("exe6_cube_life_span_check();")
        self.assertLess(refresh, lifespan)

    def test_spawn_uses_support_object_panel_occupancy(self) -> None:
        abi = (ROOT / "src/abi.h").read_text()
        self.assertIn("EXE6_BLOCK_FLAG_NEUTRAL_SUPPORT_OBJECT 0x00800000", abi)
        self.assertIn("EXE6_BLOCK_FLAG_SIDE_1_SUPPORT_OBJECT 0x01000000", abi)
        self.assertIn("EXE6_BLOCK_FLAG_SIDE_0_SUPPORT_OBJECT 0x02000000", abi)

        runtime = (ROOT / "src/runtime.c").read_text()
        self.assertIn("bn67_deployable_placement_check(", runtime)
        self.assertIn("BN67_DEPLOYABLE_PLACEMENT_INVALID", runtime)
        self.assertIn("EXE6_BLOCK_FLAG_SUPPORT_OBJECT", runtime)
        self.assertIn("BN67_DEPLOYABLE_PLACEMENT_OCCUPIED", runtime)

        for filename, deployable_slot in (("rook.c", 0), ("signal_red.c", 1)):
            source = (ROOT / "src/chips" / filename).read_text()

            placement = source[
                source.index("static void obj_normal_update"):
                source.index("static void obj_store_dust_ammo")
            ]
            self.assertIn("bn67_deployable_placement_check(", placement)
            self.assertIn("obj_begin_placement_failure(obj);", placement)
            self.assertIn("obj_begin_damage_destroy(obj);", placement)
            self.assertIn(
                f"exe6_cube_entry(obj, obj->owner, {deployable_slot});",
                placement,
            )

            spawn = source[
                source.index("static Exe6Obj *spawn_persistent"):
                source.index("static void launch_effect")
            ]
            self.assertNotIn("exe6_cube_entry", spawn)

        signal_red = (ROOT / "src/chips/signal_red.c").read_text()
        self.assertIn(
            "if (obj->work[OPPONENT_CHIPS_DISABLED_WORK] == 0)",
            signal_red,
        )

    def test_rook_keeps_owner_collision_and_extends_the_windbreak_check(self) -> None:
        source = (ROOT / "src/chips/rook.c").read_text()
        self.assertIn("EXE6_HIT_TYPE_13;", source)
        self.assertIn("EXE6_HIT_TYPE_14;", source)
        self.assertIn("#if FALZAR", source)
        self.assertNotIn("#ifdef FALZAR", source)
        self.assertIn(
            "BN67_PATCH_SECTION(0x080CD418, 0x080E42D0, "
            "rook_windbreak_filter_main);",
            source,
        )
        self.assertIn(
            "BN67_PATCH_SECTION(0x080CEC88, 0x080E42D0, "
            "rook_windbreak_filter_main);",
            source,
        )
        self.assertIn('"lsls r1,r1,#23\\n"', source)
        self.assertIn('"ldrb r2,[r5,#0x16]\\n"', source)

    def test_text_identifies_rook(self) -> None:
        text = (ROOT / "src/chips/rook.text.toml").read_text()
        self.assertIn('"0xC0" = "Rook"', text)
        self.assertIn('"0xC0" = "Protects\\nyou from\\nattacks"', text)


if __name__ == "__main__":
    unittest.main()
