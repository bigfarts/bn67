#include "abi.h"

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STRINGIFY_INNER(value) #value
#define STRINGIFY(value) STRINGIFY_INNER(value)

#define NATIVE_WRAPPER(name, address, return_type, arguments) \
    NAKED return_type name arguments \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4-r7,lr}\n" \
            "ldr r4,=" STRINGIFY(address) "+1\n" \
            "mov r12,r4\n" \
            "adr r4,1f\n" \
            "adds r4,#1\n" \
            "mov lr,r4\n" \
            "ldr r4,[sp,#0]\n" \
            "bx r12\n" \
            ".balign 4\n" \
            "1:\n" \
            "pop {r4-r7,pc}\n" \
        ); \
    }

#define NATIVE_WRAPPER_R4(name, address, return_type, arguments) \
    NAKED return_type name arguments \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4-r7,lr}\n" \
            "ldr r4,=" STRINGIFY(address) "+1\n" \
            "mov r12,r4\n" \
            "adr r4,1f\n" \
            "adds r4,#1\n" \
            "mov lr,r4\n" \
            "ldr r4,[sp,#20]\n" \
            "bx r12\n" \
            ".balign 4\n" \
            "1:\n" \
            "pop {r4-r7,pc}\n" \
        ); \
    }

#define NATIVE_CHAIN(name, first_address, second_address) \
    NAKED void name(Object *source) \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r4-r7,lr}\n" \
            "ldr r4,=" STRINGIFY(first_address) "+1\n" \
            "mov r12,r4\n" \
            "adr r4,1f\n" \
            "adds r4,#1\n" \
            "mov lr,r4\n" \
            "ldr r4,[sp,#0]\n" \
            "bx r12\n" \
            ".balign 4\n" \
            "1:\n" \
            "ldr r4,=" STRINGIFY(second_address) "+1\n" \
            "mov r12,r4\n" \
            "adr r4,2f\n" \
            "adds r4,#1\n" \
            "mov lr,r4\n" \
            "ldr r4,[sp,#0]\n" \
            "bx r12\n" \
            ".balign 4\n" \
            "2:\n" \
            "pop {r4-r7,pc}\n" \
        ); \
    }

NATIVE_WRAPPER(bn6_play_sound, 0x080005CC, void, (uint32_t sound))
NATIVE_WRAPPER(bn6_display_setup, 0x08000AC8, void, (const void *source, void *destination, uint32_t size))
NATIVE_WRAPPER_R4(bn6_palette_write, 0x08002378, void, (uint32_t unused, uint32_t color, uint32_t count, uint32_t cache, uintptr_t destination))
NATIVE_WRAPPER(bn6_palette_restore, 0x0800239A, void, (uint32_t cache))
NATIVE_WRAPPER(bn6_self_sprite_load_animation_data, 0x080026A4, void, (void))
NATIVE_WRAPPER(bn6_self_sprite_update, 0x080026C4, void, (void))
NATIVE_WRAPPER(bn6_self_sprite_load, 0x080026E4, void, (uint32_t mode, uint32_t group, uint32_t index))
NATIVE_WRAPPER(bn6_self_sprite_set_animation, 0x08002DA4, void, (uint32_t animation))
NATIVE_WRAPPER(bn6_self_sprite_set_palette, 0x08002D80, void, (uint32_t palette))
NATIVE_WRAPPER(bn6_self_sprite_flash_white, 0x08002DB0, void, (void))
NATIVE_WRAPPER(bn6_sprite_get_palette, 0x08002D8C, uint32_t, (Object *object))
NATIVE_WRAPPER(bn6_self_sprite_set_scale, 0x08002ED0, void, (uint32_t scale))
NATIVE_WRAPPER(bn6_sprite_get_scale, 0x08002EDC, uint32_t, (Object *object))
NATIVE_WRAPPER(bn6_self_sprite_set_blend, 0x08002EF6, void, (uint32_t low, uint32_t high))
NATIVE_WRAPPER(bn6_self_sprite_set_priority, 0x08002E14, void, (uint32_t priority))
NATIVE_WRAPPER(bn6_self_sprite_get_frame_flags, 0x08002DEA, uint32_t, (void))
NATIVE_WRAPPER(bn6_self_sprite_set_blend_mode, 0x08002C7A, void, (uint32_t blend))
NATIVE_WRAPPER(bn6_self_sprite_property_2cce, 0x08002CCE, void, (void))
NATIVE_WRAPPER(bn6_self_death_sprite_special, 0x08003006, void, (void))
NATIVE_WRAPPER(bn6_self_sprite_enable_shadow, 0x08002E3C, void, (void))
NATIVE_WRAPPER(bn6_self_sprite_set_flip, 0x08002F5C, void, (uint32_t flip))
NATIVE_WRAPPER(bn6_self_sprite_hide_piece, 0x08002FBE, void, (uint32_t piece_index))
NATIVE_WRAPPER(bn6_self_sprite_no_shadow, 0x08002F90, void, (void))
NATIVE_CHAIN(bn6_self_sprite_copy_visibility, 0x08002CE0, 0x08002C7E)
NATIVE_CHAIN(bn6_self_sprite_copy_palette_bits, 0x08002DC8, 0x08002DB4)
NATIVE_CHAIN(bn6_self_sprite_copy_special_bits, 0x08002F3E, 0x08002F02)
NATIVE_WRAPPER(bn6_self_object_free, 0x08003458, void, (void))
NATIVE_WRAPPER(bn6_battle_is_time_stopped, 0x0800A098, uint32_t, (void))
NATIVE_WRAPPER(bn6_battle_is_over, 0x0800A18E, uint32_t, (void))
NATIVE_WRAPPER(bn6_lock_battle_state, 0x0800A028, void, (void))
NATIVE_WRAPPER(bn6_rebuild_folder, 0x0800A318, void, (void))
NATIVE_WRAPPER(bn6_shuffle_chip_queue, 0x0800A570, void, (void *queue, uint32_t reserved, uint32_t regular, uint32_t size))
NATIVE_WRAPPER(bn6_link_battle_active, 0x0800A8F8, uint32_t, (void))
NATIVE_WRAPPER(bn6_clear_hand, 0x0800A954, void, (void))
NATIVE_WRAPPER(bn6_compare_local_side, 0x0800A9EC, uint32_t, (uint32_t side))
NATIVE_WRAPPER(bn6_draw_delete_overlay, 0x0800AE90, void, (uint32_t duration, uint32_t animation))
NATIVE_WRAPPER(bn6_battle_get_config_flags, 0x0802D246, uint32_t, (void))
NATIVE_WRAPPER(bn6_panel_is_valid_xy, 0x0800CC72, uint32_t, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(bn6_panel_matches_flags, 0x0800CC86, uint32_t, (uint32_t x, uint32_t y, uint32_t required_flags, uint32_t excluded_flags))
NATIVE_WRAPPER(bn6_panel_get_parameters, 0x0800D5F0, uint32_t, (uint32_t x, uint32_t owner))
NATIVE_WRAPPER(bn6_panel_get_flags, 0x0800C8F8, uint32_t, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(bn6_panel_set_flash, 0x0800CBD8, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(bn6_panel_crack_from_solid, 0x0800C938, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(bn6_panel_crack, 0x0800CA8C, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(bn6_angle_from_vector, 0x0800117C, uint32_t, (int32_t y, int32_t x))
NATIVE_WRAPPER(bn6_self_object_side_direction, 0x0800E2C0, int32_t, (void))
NATIVE_WRAPPER(bn6_self_panel_is_valid_object, 0x0800CC66, uint32_t, (void))
NATIVE_WRAPPER(bn6_self_object_set_coords, 0x0800E29C, void, (void))
NATIVE_WRAPPER(bn6_self_object_load_navi_sprite, 0x0801BE2A, void, (uint32_t selector))
NATIVE_WRAPPER(bn6_self_object_update_panel, 0x0800E2AC, void, (void))
NATIVE_WRAPPER(bn6_self_object_apply_damage, 0x0800E2D8, void, (uint32_t damage))
NATIVE_WRAPPER(bn6_self_object_get_flip, 0x0800E456, uint32_t, (void))
NATIVE_WRAPPER(bn6_object_register_deployable, 0x0800F614, void, (Object *object, uint32_t owner, uint32_t slot))
NATIVE_WRAPPER(bn6_self_object_unregister_deployable, 0x0800F656, void, (void))
NATIVE_WRAPPER(bn6_self_deployable_lifetime_update, 0x0800F672, void, (void))
NATIVE_WRAPPER(bn6_self_object_update_timed_removal, 0x0800F8CE, uint32_t, (void))
NATIVE_WRAPPER(bn6_self_object_store_dust_ammo, 0x0800F90E, void, (uint32_t kind))
NATIVE_WRAPPER(bn6_player_object_for_side, 0x080103BC, Object *, (uint32_t side))
NATIVE_WRAPPER(bn6_chip_list, 0x08010018, const uint8_t *, (uint32_t side))
NATIVE_WRAPPER(bn6_player_properties_for_side, 0x08013682, uint8_t *, (uint32_t side))
NATIVE_WRAPPER(bn6_player_property_set_for_side, 0x080136B0, void, (uint32_t side, uint32_t property, uint32_t value))
NATIVE_WRAPPER(bn6_player_property_for_side, 0x080136CC, uint32_t, (uint32_t side, uint32_t property))
NATIVE_WRAPPER(bn6_input_state_for_side, 0x0800A0F4, const uint8_t *, (uint32_t side))
NATIVE_WRAPPER(bn6_set_hud_gauge, 0x0801DFA2, void, (uint32_t value))
NATIVE_WRAPPER(bn6_begin_local_custom, 0x0801E15C, void, (void))
NATIVE_WRAPPER(bn6_player_data, 0x0802E070, uint8_t *, (uint32_t side))
NATIVE_WRAPPER(bn6_gauge_subtract, 0x0802E04E, void, (uint32_t side, uint32_t amount))
NATIVE_WRAPPER(bn6_screen_shake_set, 0x080302A8, void, (uint32_t intensity, uint32_t duration))
NATIVE_WRAPPER(bn6_battle_set_control_flags, 0x08001382, void, (uint32_t control_flags))
NATIVE_WRAPPER(bn6_battle_clear_control_flags, 0x0800138E, void, (uint32_t control_flags))
NATIVE_WRAPPER(bn6_self_collision_create, 0x08019892, Collision *, (void))
NATIVE_WRAPPER(bn6_collision_free, 0x080198CE, void, (Collision *collision))
NATIVE_WRAPPER(
    bn6_collision_setup,
    0x08019FB4,
    void,
    (
        Collision *collision,
        Bn6CollisionType self_collision_type,
        Bn6CollisionType target_collision_type,
        uint32_t hit_modifier
    )
)
NATIVE_WRAPPER(bn6_collision_remove, 0x0801A00E, void, (Collision *collision))
NATIVE_WRAPPER(bn6_self_collision_present, 0x0801A018, void, (uint32_t unused, uint32_t region))
NATIVE_WRAPPER(bn6_self_collision_update_panel, 0x0801A04C, void, (void))
NATIVE_WRAPPER(bn6_collision_clear_region, 0x0801A074, void, (Collision *collision))
NATIVE_WRAPPER(bn6_self_collision_spawn_effect, 0x0801A0D4, void, (void))
NATIVE_WRAPPER(bn6_self_collision_set_hit_effect, 0x0801A140, void, (Bn6HitEffect effect))
NATIVE_WRAPPER(bn6_self_collision_set_extended_effect, 0x0801A4D0, void, (uint32_t low, uint32_t high))
NATIVE_WRAPPER(bn6_self_collision_get_secondary_flags, 0x0801A180, uint32_t, (void))
NATIVE_WRAPPER(bn6_self_field_collision_update, 0x0801AD12, void, (void))
NATIVE_WRAPPER(bn6_self_object_update, 0x0801BBAC, void, (void))
NATIVE_WRAPPER(bn6_self_object_update_timestop, 0x0801BBF4, void, (void))
NATIVE_WRAPPER(bn6_rng_next, 0x0800151C, uint32_t, (void))
NATIVE_WRAPPER(bn6_battle_rng, 0x08001532, uint32_t, (void))
NATIVE_WRAPPER(bn6_memory_clear, 0x080008B4, void, (void *destination, uint32_t size))
NATIVE_WRAPPER(bn6_memory_copy, 0x08000920, void, (const void *source, void *destination, uint32_t size))
NATIVE_WRAPPER(bn6_shuffle_bytes, 0x08000C72, void, (void *bytes, uint32_t count, uint32_t range))
NATIVE_WRAPPER(bn6_bugfix_clear_runtime_state, 0x0801E658, void, (void))

NAKED void bn6_object_invoke(Object *object, uintptr_t entry)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "adds r5,r0,#0\n"
        "mov r12,r1\n"
        "adr r4,1f\n"
        "adds r4,#1\n"
        "mov lr,r4\n"
        "ldr r4,[sp,#0]\n"
        "bx r12\n"
        ".balign 4\n"
        "1:\n"
        "pop {r4-r7,pc}\n"
    );
}

#if FALZAR
#define BATTLE_EFFECT_SPAWN_ADDRESS 0x080E05F6
#else
#define BATTLE_EFFECT_SPAWN_ADDRESS 0x080E1932
#endif

NATIVE_WRAPPER_R4(
    bn6_spawn_battle_effect,
    BATTLE_EFFECT_SPAWN_ADDRESS,
    Object *,
    (uint32_t unused, int32_t x, int32_t y, int32_t z, uint32_t effect)
)

NAKED Object *bn6_spawn_type1(uint32_t type, uint32_t implicit_r4)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,=0x08003321\n"
        "mov r12,r4\n"
        "adds r4,r1,#0\n"
        "mov lr,pc\n"
        "bx r12\n"
        "adds r0,r5,#0\n"
        "pop {r4-r7,pc}\n"
    );
}

NAKED void bn6_player_clear_collision_status_flags(
    Object *player,
    uint32_t status_flags
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "adds r5,r0,#0\n"
        "adds r0,r1,#0\n"
        "ldr r4,=0x0801A15D\n"
        "mov r12,r4\n"
        "mov lr,pc\n"
        "bx r12\n"
        "pop {r4-r7,pc}\n"
    );
}

#if FALZAR
#define PANEL_DAMAGE_ADDRESS 0x080C53A6
#else
#define PANEL_DAMAGE_ADDRESS 0x080C6C16
#endif

NAKED Object *bn6_spawn_panel_damage(
    uint32_t panel_x,
    uint32_t panel_y,
    uint32_t parameter,
    uint32_t unused,
    Bn6PanelDamageProperties properties,
    uint32_t attack,
    uint32_t mode
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,=" STRINGIFY(PANEL_DAMAGE_ADDRESS) "+1\n"
        "mov r12,r4\n"
        "adr r4,1f\n"
        "adds r4,#1\n"
        "mov lr,r4\n"
        "ldr r4,[sp,#20]\n"
        "ldr r6,[sp,#24]\n"
        "ldr r7,[sp,#28]\n"
        "bx r12\n"
        ".balign 4\n"
        "1:\n"
        "pop {r4-r7,pc}\n"
    );
}

#if FALZAR
#define SAVED_NAVI_INTRO_ADDRESS 0x080E1332
#else
#define SAVED_NAVI_INTRO_ADDRESS 0x080E266E
#endif

NATIVE_WRAPPER(
    bn6_saved_navi_intro,
    SAVED_NAVI_INTRO_ADDRESS,
    void,
    (Object *owner, uint32_t mode)
)

NAKED void bn6_saved_navi_dispatch(
    uintptr_t entry,
    Object *owner,
    uint32_t panel_x,
    uint32_t panel_y,
    uint32_t parameter,
    uintptr_t data,
    uint32_t properties,
    uint8_t *completion
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "mov r12,r0\n"
        "adds r4,r3,#0\n"
        "adr r0,1f\n"
        "adds r0,#1\n"
        "mov lr,r0\n"
        "adds r5,r1,#0\n"
        "adds r0,r2,#0\n"
        "adds r1,r4,#0\n"
        "ldr r2,[sp,#20]\n"
        "ldr r6,[sp,#24]\n"
        "ldr r4,[sp,#28]\n"
        "ldr r7,[sp,#32]\n"
        "mov r3,r12\n"
        "bx r12\n"
        ".balign 4\n"
        "1:\n"
        "pop {r4-r7,pc}\n"
    );
}

NATIVE_WRAPPER(bn6_self_type4_timestop_init, 0x0800B916, void, (void))
NATIVE_WRAPPER(bn6_self_type4_timestop_intro, 0x0800B94C, void, (void))
NATIVE_WRAPPER(bn6_self_type4_timestop_freeze, 0x0800B9B0, void, (void))
NATIVE_WRAPPER(bn6_self_type4_timestop_outro, 0x0800BC88, void, (void))
NATIVE_WRAPPER(bn6_self_type4_timestop_free, 0x0800BD34, void, (void))

NAKED Object *bn6_spawn_type3(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t implicit_r4
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,[sp,#20]\n"
        "ldr r7,=0x08003359\n"
        "mov r12,r7\n"
        "mov lr,pc\n"
        "bx r12\n"
        "adds r0,r5,#0\n"
        "pop {r4-r7,pc}\n"
    );
}

NAKED Object *bn6_spawn_type4(uint32_t type, uint32_t implicit_r4)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,=0x080033AD\n"
        "mov r12,r4\n"
        "adds r4,r1,#0\n"
        "mov lr,pc\n"
        "bx r12\n"
        "adds r0,r5,#0\n"
        "pop {r4-r7,pc}\n"
    );
}

NAKED Object *bn6_spawn_type4_at(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    uint32_t implicit_r4
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,[sp,#20]\n"
        "ldr r7,=0x080033AD\n"
        "mov r12,r7\n"
        "mov lr,pc\n"
        "bx r12\n"
        "adds r0,r5,#0\n"
        "pop {r4-r7,pc}\n"
    );
}

NAKED uint32_t bn6_object_front_direction_for(Object *object)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "adds r5,r0,#0\n"
        "ldr r4,=0x0800E2CB\n"
        "mov r12,r4\n"
        "ldr r4,[sp,#0]\n"
        "mov lr,pc\n"
        "bx r12\n"
        "pop {r4-r7,pc}\n"
    );
}

NATIVE_WRAPPER(bn6_panel_to_coords, 0x0800E276, uint64_t, (uint32_t x, uint32_t y))
