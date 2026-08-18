#include "abi.h"

/*
 * Native names mirror the recovered MEGAMAN6_GXX_BR5E00 symbols. Source-file
 * prefixes are omitted and CamelCase is normalized to lower snake case.
 */

#pragma GCC diagnostic ignored "-Wunused-parameter"

#define STRINGIFY_INNER(value) #value
#define STRINGIFY(value) STRINGIFY_INNER(value)

/*
 * BN6 callees preserve the high registers they use.  Tail-call ordinary
 * r0-r2 ABI entries so the native return goes straight to the C caller; its
 * BL already put an odd Thumb address in lr.  The far jump is unavoidable
 * because linked C code is outside the Thumb-1 BL range of the base ROM.
 */
#define NATIVE_WRAPPER(name, address, return_type, arguments) \
    NAKED return_type name arguments \
    { \
        __asm__( \
            ".syntax unified\n" \
            "ldr r3,=" STRINGIFY(address) "+1\n" \
            "bx r3\n" \
        ); \
    }

/* Preserve a fourth C argument while borrowing r3 for the far target. */
#define NATIVE_WRAPPER_R3(name, address, return_type, arguments) \
    NAKED return_type name arguments \
    { \
        __asm__( \
            ".syntax unified\n" \
            "push {r3}\n" \
            "ldr r3,=" STRINGIFY(address) "+1\n" \
            "mov r12,r3\n" \
            "pop {r3}\n" \
            "bx r12\n" \
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
    NAKED void name(Exe6Obj *source) \
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

Exe6Runtime *exe6_runtime(void)
{
    Exe6Runtime *runtime;
    __asm__("mov %0,r10" : "=l" (runtime));
    return runtime;
}

NATIVE_WRAPPER(exe6_sound_req, 0x080005CC, void, (uint32_t sound))
NATIVE_WRAPPER(exe6_get_cur_pet_navi, 0x080010B6, uint32_t, (void))
NATIVE_WRAPPER(exe6_mem_trans256, 0x08000950, void, (const void *source, void *destination, uint32_t size))
NATIVE_WRAPPER(exe6_mem_task_trans_set256, 0x08000AC8, void, (const void *source, void *destination, uint32_t size))
NATIVE_WRAPPER_R4(exe6_col_fade_set, 0x08002378, void, (uint32_t unused, uint32_t color, uint32_t count, uint32_t cache, uintptr_t destination))
NATIVE_WRAPPER(exe6_col_fade_kill, 0x0800239A, void, (uint32_t cache))
NATIVE_WRAPPER(exe6_obj_char_set, 0x080026A4, void, (void))
NATIVE_WRAPPER(exe6_obj_char_move, 0x080026C4, void, (void))
NATIVE_WRAPPER(exe6_obj_char_init, 0x080026E4, void, (uint32_t mode, uint32_t group, uint32_t index))

NATIVE_WRAPPER(exe6_obj_dma_seq_set, 0x08002DA4, void, (uint32_t animation))
NATIVE_WRAPPER(exe6_obj_clt_set, 0x08002D80, void, (uint32_t palette))
NATIVE_WRAPPER(exe6_obj_flash_set, 0x08002DB0, void, (void))
NATIVE_WRAPPER(exe6_obj_flash_reset, 0x08002DD8, void, (void))
NATIVE_WRAPPER(exe6_obj_clt_link_get, 0x08002D8C, uint32_t, (Exe6Obj *obj))
NATIVE_WRAPPER(exe6_obj_col_efc_set, 0x08002ED0, void, (uint32_t scale))
NATIVE_WRAPPER(exe6_obj_col_efc_link_get, 0x08002EDC, uint32_t, (Exe6Obj *obj))
NATIVE_WRAPPER(exe6_obj_mosaic_set, 0x08002EF6, void, (uint32_t low, uint32_t high))
NATIVE_WRAPPER(exe6_obj_prio_set, 0x08002E14, void, (uint32_t priority))
NATIVE_WRAPPER(exe6_obj_seq_info_get, 0x08002DEA, uint32_t, (void))
NATIVE_WRAPPER(exe6_obj_bld_set, 0x08002C7A, void, (uint32_t blend))
NATIVE_WRAPPER(exe6_obj_bld_reset, 0x08002CCE, void, (void))
NATIVE_WRAPPER(exe6_obj_shadow_all_set, 0x08003006, void, (void))
NATIVE_WRAPPER(exe6_obj_shadow_set, 0x08002E3C, void, (void))
NATIVE_WRAPPER(exe6_obj_flip_set, 0x08002F5C, void, (uint32_t flip))
NATIVE_WRAPPER(exe6_obj_no_trans_flag_num_set, 0x08002FBE, void, (uint32_t piece_index))
NATIVE_WRAPPER(exe6_obj_no_shadow, 0x08002F90, void, (void))
NATIVE_CHAIN(exe6_obj_bld_link_copy, 0x08002CE0, 0x08002C7E)
NATIVE_CHAIN(exe6_obj_flash_link_copy, 0x08002DC8, 0x08002DB4)
NATIVE_CHAIN(exe6_obj_mosaic_link_copy, 0x08002F3E, 0x08002F02)
NATIVE_WRAPPER(exe6_obj_move_delete, 0x08003458, void, (void))
NATIVE_WRAPPER(exe6_battle_event_busy_check, 0x0800A098, uint32_t, (void))
NATIVE_WRAPPER(exe6_battle_end_check, 0x0800A18E, uint32_t, (void))
NATIVE_WRAPPER(exe6_battle_pause_on, 0x0800A028, void, (void))
NATIVE_WRAPPER(exe6_battle_chip_set, 0x0800A318, void, (void))
NATIVE_WRAPPER(
    exe6_deck_shuffle_sub,
    0x0800A570,
    void,
    (Exe6ChipQueue *queue, uint32_t preserve_regular, uint32_t preserve_tag)
)
NATIVE_WRAPPER(exe6_real_operation_battle_check, 0x0800A8F8, uint32_t, (void))
NATIVE_WRAPPER(exe6_battle_select_chip_work_init, 0x0800A954, void, (void))
NATIVE_WRAPPER(exe6_battle_one_self_check, 0x0800A9EC, uint32_t, (uint32_t side))
NATIVE_WRAPPER(exe6_yazirushi_trans, 0x0800AE90, void, (uint32_t duration, uint32_t animation))
NATIVE_WRAPPER(exe6_em_set_flag_get, 0x0802D246, uint32_t, (void))
NATIVE_WRAPPER(exe6_block_in_screen_check_sub, 0x0800CC72, uint32_t, (uint32_t x, uint32_t y))
NATIVE_WRAPPER_R3(exe6_block_move_check, 0x0800CC86, uint32_t, (uint32_t x, uint32_t y, uint32_t required_flags, uint32_t excluded_flags))
NATIVE_WRAPPER(exe6_another_block_exist_check, 0x0800D5F0, uint32_t, (uint32_t x, uint32_t owner))
NATIVE_WRAPPER(exe6_block_at, 0x0800C90A, Exe6Block *, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(exe6_block_status_get, 0x0800C8F8, uint32_t, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(exe6_block_flash, 0x0800CBD8, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(exe6_block_crack_set, 0x0800C938, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(exe6_block_out_set3, 0x0800CA8C, void, (uint32_t x, uint32_t y))
NATIVE_WRAPPER(exe6_calc_degree, 0x0800117C, uint32_t, (int32_t y, int32_t x))
NATIVE_WRAPPER(exe6_calc_pl_em_spd, 0x0800E2C0, int32_t, (void))
NATIVE_WRAPPER(exe6_block_in_screen_check, 0x0800CC66, uint32_t, (void))
NATIVE_WRAPPER(exe6_block_to_pos, 0x0800E29C, void, (void))
NATIVE_WRAPPER(exe6_battle_obj_char_init, 0x0801BE2A, void, (uint32_t selector))
NATIVE_WRAPPER(exe6_pos_to_block, 0x0800E2AC, void, (void))
NATIVE_WRAPPER(exe6_enemy_life_sub, 0x0800E2D8, void, (uint32_t damage))
NATIVE_WRAPPER(exe6_enemy_flip_check, 0x0800E456, uint32_t, (void))
NATIVE_WRAPPER(exe6_cube_entry, 0x0800F614, void, (Exe6Obj *obj, uint32_t owner, uint32_t slot))
NATIVE_WRAPPER(exe6_cube_delete, 0x0800F656, void, (void))
NATIVE_WRAPPER(exe6_cube_life_span_check, 0x0800F672, void, (void))
NATIVE_WRAPPER(exe6_cube_erase2, 0x0800F8CE, uint32_t, (void))
NATIVE_WRAPPER(exe6_cube_set_dust_suikomi_efc, 0x0800F90E, void, (uint32_t kind))
NATIVE_WRAPPER(exe6_get_navi_adrs, 0x080103BC, Exe6Obj *, (uint32_t side))

/* The native Uninstall routine takes the target player through fixed r5. */
NAKED void exe6_navi_uninstall(Exe6Obj *player)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "adds r5,r0,#0\n"
        "ldr r4,=0x080140EF\n"
        "mov r12,r4\n"
        "mov lr,pc\n"
        "bx r12\n"
        "pop {r4-r7,pc}\n"
    );
}

NATIVE_WRAPPER(
    exe6_navi_select_chip_work_adrs_get,
    0x08010018,
    Exe6NaviSelectChipWork *,
    (uint32_t side)
)
NATIVE_WRAPPER(
    exe6_navi_status_work_adrs_get,
    0x08013682,
    Exe6NaviStatusWork *,
    (uint32_t side)
)
NATIVE_WRAPPER(exe6_navi_status_set, 0x080136B0, void, (uint32_t side, uint32_t property, uint32_t value))
NATIVE_WRAPPER(exe6_navi_status_get, 0x080136CC, uint32_t, (uint32_t side, uint32_t property))
NATIVE_WRAPPER(exe6_cur_pet_navi_stats_set, 0x0801379E, void, (uint32_t slot, uint32_t property, uint32_t value))
NATIVE_WRAPPER(exe6_cur_pet_navi_stats_get, 0x080137B6, uint32_t, (uint32_t slot, uint32_t property))

NATIVE_WRAPPER(
    exe6_cur_pet_navi_stats_adrs_get,
    0x0801401E,
    uint8_t *,
    (uint32_t navi)
)
NATIVE_WRAPPER(
    exe6_special_navi_stats_adrs_get,
    0x08013854,
    uint8_t *,
    (uint32_t index)
)
NATIVE_WRAPPER(exe6_battle_key_work_adrs_get, 0x0800A0F4, const uint8_t *, (uint32_t side))
NATIVE_WRAPPER(exe6_cockpit_set_custom_gauge_value, 0x0801DFA2, void, (uint32_t value))
NATIVE_WRAPPER(exe6_cockpit_get_custom_gauge_value, 0x0801DFE4, uint32_t, (void))
NATIVE_WRAPPER(exe6_cockpit_pause_set, 0x0801E15C, void, (void))
NATIVE_WRAPPER(exe6_operate_slot_in_gauge_sub, 0x0802E04E, void, (uint32_t side, uint32_t amount))
NATIVE_WRAPPER(exe6_camera_quake_set, 0x080302A8, void, (uint32_t intensity, uint32_t duration))
NATIVE_WRAPPER(exe6_battle_report_flag_on, 0x08001382, void, (uint32_t control_flags))
NATIVE_WRAPPER(exe6_battle_report_flag_off, 0x0800138E, void, (uint32_t control_flags))
NATIVE_WRAPPER(exe6_battle_hit_open, 0x08019892, Exe6Hit *, (void))
NATIVE_WRAPPER(exe6_battle_hit_close, 0x080198CE, void, (Exe6Hit *hit))
NATIVE_WRAPPER_R3(
    exe6_battle_hit_data_set,
    0x08019FB4,
    void,
    (
        Exe6Hit *hit,
        Exe6HitType self_hit_type,
        Exe6HitType target_hit_type,
        uint32_t hit_modifier
    )
)
NATIVE_WRAPPER(exe6_battle_hit_check, 0x0801A00E, void, (Exe6Hit *hit))
NATIVE_WRAPPER(exe6_battle_hit_set, 0x0801A018, void, (uint32_t unused, uint32_t region))
NATIVE_WRAPPER(exe6_battle_hit_block_pos_set, 0x0801A04C, void, (void))
NATIVE_WRAPPER(exe6_battle_hit_off, 0x0801A074, void, (Exe6Hit *hit))
NATIVE_WRAPPER(exe6_battle_hit_hit_mark_check, 0x0801A0D4, void, (void))
NATIVE_WRAPPER(exe6_battle_hit_hit_mark_set, 0x0801A140, void, (Exe6HitEffect effect))
NATIVE_WRAPPER(exe6_battle_hit_status_change_set, 0x0801A4D0, void, (uint32_t low, uint32_t high))
NATIVE_WRAPPER(exe6_battle_hit_req_flag_get, 0x0801A180, uint32_t, (void))
NATIVE_WRAPPER(exe6_cube_hit_check, 0x0801AD12, void, (void))
NATIVE_WRAPPER(exe6_cube_guard_mark_check, 0x0800EB9E, void, (void))
NATIVE_WRAPPER(exe6_battle_obj_char_move, 0x0801BBAC, void, (void))
NATIVE_WRAPPER(exe6_battle_obj_char_move2, 0x0801BBF4, void, (void))
NATIVE_WRAPPER(exe6_rand, 0x0800151C, uint32_t, (void))
NATIVE_WRAPPER(exe6_rand2, 0x08001532, uint32_t, (void))
NATIVE_WRAPPER(exe6_mem_clear8, 0x080008B4, void, (void *destination, uint32_t size))
NATIVE_WRAPPER(exe6_mem_trans8, 0x08000920, void, (const void *source, void *destination, uint32_t size))
NATIVE_WRAPPER(exe6_shuffle_sub, 0x08000C72, void, (void *bytes, uint32_t count, uint32_t range))
NATIVE_WRAPPER(exe6_cockpit_kokoro_navicus_bug_clear, 0x0801E658, void, (void))

NAKED void exe6_obj_invoke(Exe6Obj *obj, uintptr_t entry)
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
#define SET_EFC00_ADDRESS 0x080E05F6
#else
#define SET_EFC00_ADDRESS 0x080E1932
#endif

NATIVE_WRAPPER_R4(
    exe6_set_efc00,
    SET_EFC00_ADDRESS,
    Exe6Obj *,
    (uint32_t unused, int32_t x, int32_t y, int32_t z, uint32_t effect)
)

NAKED Exe6Obj *exe6_em_open(
    uint32_t type,
    Exe6ObjSpawnParameters spawn_parameters
)
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

NAKED void exe6_battle_hit_status_flag_off(
    Exe6Obj *player,
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
#define SET_SHL03_EV_ADDRESS 0x080C53A6
#else
#define SET_SHL03_EV_ADDRESS 0x080C6C16
#endif

NAKED Exe6Obj *exe6_set_shl03_ev(
    uint32_t block_x,
    uint32_t block_y,
    uint32_t parameter,
    uint32_t unused,
    Exe6BlockDamageProperties properties,
    uint32_t attack,
    uint32_t mode
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "ldr r4,=" STRINGIFY(SET_SHL03_EV_ADDRESS) "+1\n"
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
#define SET_EFC0C_ADDRESS 0x080E1332
#else
#define SET_EFC0C_ADDRESS 0x080E266E
#endif

NATIVE_WRAPPER(
    exe6_set_efc0c,
    SET_EFC0C_ADDRESS,
    void,
    (Exe6Obj *owner, uint32_t mode)
)

NAKED void exe6_saved_navi_dispatch(
    uintptr_t entry,
    Exe6Obj *owner,
    uint32_t block_x,
    uint32_t block_y,
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

NATIVE_WRAPPER(exe6_event_chip_state_reset, 0x0800B89C, void, (uint32_t side))
NATIVE_WRAPPER(exe6_event_chip_common_init, 0x0800B916, void, (void))
NATIVE_WRAPPER(exe6_event_chip_common_fade, 0x0800B94C, void, (void))
NATIVE_WRAPPER(exe6_event_chip_common_telop, 0x0800B9B0, void, (void))
NATIVE_WRAPPER(exe6_event_chip_common_end, 0x0800BC88, void, (void))
NATIVE_WRAPPER(exe6_event_chip_common_exit, 0x0800BD34, void, (void))

NAKED Exe6Obj *exe6_shl_open(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    Exe6ObjSpawnParameters spawn_parameters
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

NAKED Exe6Obj *exe6_efc_open(
    uint32_t type,
    Exe6ObjSpawnParameters spawn_parameters
)
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

NAKED Exe6Obj *exe6_efc_open_at(
    uint32_t type,
    int32_t x,
    int32_t y,
    int32_t z,
    Exe6ObjSpawnParameters spawn_parameters
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

NAKED uint32_t exe6_calc_pl_em_dir_spd_for(Exe6Obj *obj)
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

NATIVE_WRAPPER(exe6_get_block_pos, 0x0800E276, uint64_t, (uint32_t x, uint32_t y))
