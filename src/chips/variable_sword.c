#include "runtime.h"

/*
 * BN6 keeps separate VarSword command tables for the two facing directions.
 * Entries include the held A bit (0x01); B is 0x02, and the directional bits
 * are Right 0x10, Left 0x20, Up 0x40, and Down 0x80. ElementSonic is first so
 * its command-progress byte lives at action-work offset +0x30. The remaining
 * five entries preserve BN6's native command order.
 */
BN67_ASM_RESOURCE(
    variable_sword_command_tables,
    /* Side 0 command pointers. */
    ".long variable_sword_element_sonic_side_0\n"
    ".long variable_sword_long_sword_side_0\n"
    ".long variable_sword_fighter_sword_side_0\n"
    ".long variable_sword_wide_sword_side_0\n"
    ".long variable_sword_life_sword_side_0\n"
    ".long variable_sword_sonic_boom_side_0\n"
    /* Side 1 command pointers, with horizontal inputs mirrored. */
    ".long variable_sword_element_sonic_side_1\n"
    ".long variable_sword_long_sword_side_1\n"
    ".long variable_sword_fighter_sword_side_1\n"
    ".long variable_sword_wide_sword_side_1\n"
    ".long variable_sword_life_sword_side_1\n"
    ".long variable_sword_sonic_boom_side_1\n"
    /* B, B, Back, Down, Up. */
    "variable_sword_element_sonic_side_0:\n"
    ".hword 0x0003,0x0003,0x0021,0x0081,0x0041,0\n"
    /* Down, Down-Forward, Forward. */
    "variable_sword_long_sword_side_0:\n"
    ".hword 0x0081,0x0091,0x0011,0\n"
    /* Back, Down-Back, Down, Down-Forward, Forward. */
    "variable_sword_fighter_sword_side_0:\n"
    ".hword 0x0021,0x00A1,0x0081,0x0091,0x0011,0\n"
    /* Up, Forward, Down. */
    "variable_sword_wide_sword_side_0:\n"
    ".hword 0x0041,0x0011,0x0081,0\n"
    /* Down, Back, Up, Forward, Down. */
    "variable_sword_life_sword_side_0:\n"
    ".hword 0x0081,0x0021,0x0041,0x0011,0x0081,0\n"
    /* Back, B, Forward, B. */
    "variable_sword_sonic_boom_side_0:\n"
    ".hword 0x0021,0x0003,0x0011,0x0003,0\n"
    "variable_sword_element_sonic_side_1:\n"
    ".hword 0x0003,0x0003,0x0011,0x0081,0x0041,0\n"
    "variable_sword_long_sword_side_1:\n"
    ".hword 0x0081,0x00A1,0x0021,0\n"
    "variable_sword_fighter_sword_side_1:\n"
    ".hword 0x0011,0x0091,0x0081,0x00A1,0x0021,0\n"
    "variable_sword_wide_sword_side_1:\n"
    ".hword 0x0041,0x0021,0x0081,0\n"
    "variable_sword_life_sword_side_1:\n"
    ".hword 0x0081,0x0011,0x0041,0x0021,0x0081,0\n"
    "variable_sword_sonic_boom_side_1:\n"
    ".hword 0x0011,0x0003,0x0021,0x0003,0\n"
);

/* Hidden attack records used by the native VarSword action state machine. */
BN67_ASM_RESOURCE(
    variable_sword_command_results,
    ".hword 0x0173\n" /* ElementSonic -> SonicBom. */
    ".hword 0x0049\n" /* LongSword. */
    ".hword 0x0172\n" /* FighterSword. */
    ".hword 0x0048\n" /* WideSword. */
    ".hword 0x0153\n" /* LifeSword. */
    ".hword 0x0173\n" /* BN6's ordinary SonicBoom. */
);

#if FALZAR
BN67_PATCH_POINTER(0x080EF66C, variable_sword_command_tables);
BN67_PATCH_POINTER(0x080EF7D4, variable_sword_command_results);
BN67_PATCH_THUMB_POINTER(0x080EF984, variable_sword_sonic_boom_init);
BN67_PATCH_SECTION(0x080EF650, 0x080EF670, variable_sword_command_table_setup);
BN67_PATCH_SECTION(0x080EF760, 0x080EF678, variable_sword_command_matcher_setup);
BN67_PATCH_SECTION(0x080EF9F8, 0x080EF680, variable_sword_element_wave_dispatch);
BN67_PATCH_SECTION(0x080EFA02, 0x080EF688, variable_sword_sonic_shell_finalize);
BN67_PATCH_SECTION(0x080CF87E, 0x080EF690, variable_sword_sonic_hit_modifier);
#else
BN67_PATCH_POINTER(0x080F09AC, variable_sword_command_tables);
BN67_PATCH_POINTER(0x080F0B14, variable_sword_command_results);
BN67_PATCH_THUMB_POINTER(0x080F0CC4, variable_sword_sonic_boom_init);
BN67_PATCH_SECTION(0x080F0990, 0x080F09B0, variable_sword_command_table_setup);
BN67_PATCH_SECTION(0x080F0AA0, 0x080F09B8, variable_sword_command_matcher_setup);
BN67_PATCH_SECTION(0x080F0D38, 0x080F09C0, variable_sword_element_wave_dispatch);
BN67_PATCH_SECTION(0x080F0D42, 0x080F09C8, variable_sword_sonic_shell_finalize);
BN67_PATCH_SECTION(0x080D10EE, 0x080F09D0, variable_sword_sonic_hit_modifier);
#endif

/*
 * The section-patch ABI saves r1 before entering each expanded-ROM helper.
 * These two helpers reproduce the native instructions displaced while growing
 * VarSword's per-side table from five pointers to six.
 */
NAKED void variable_sword_command_table_setup(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "movs r1,#24\n"
        "muls r0,r1\n"
        "ldr r1,=variable_sword_command_tables\n"
        "bx lr\n"
    );
}

NAKED void variable_sword_command_matcher_setup(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "movs r0,#24\n"
        "ldr r2,[r7,#0x28]\n"
        "lsrs r3,r0,#2\n"
        "bx lr\n"
    );
}

/*
 * The native state dispatcher enters through `mov lr,pc; bx r1`, leaving an
 * even return address. `mov pc,lr` preserves Thumb state where `bx lr` would
 * switch to ARM and reset the game.
 */
NAKED void variable_sword_sonic_boom_init(void)
{
    __asm__(
        ".syntax unified\n"
        "movs r1,#1\n"
        "movs r0,#0x30\n"
        "ldrb r0,[r7,r0]\n"
        "cmp r0,#10\n"
        "bne 1f\n"
        "adds r1,#3\n"
        "1:\n"
        "strh r1,[r7,#0x12]\n"
        "movs r0,#4\n"
        "strh r0,[r7,#0]\n"
        "mov pc,lr\n"
    );
}

/*
 * A completed ElementSonic command leaves 10 at +0x30. Counts 4, 3, 2, 1
 * become the Fire, Elec, Wood, and Aqua parameters accepted by shell 0x58.
 */
NAKED void variable_sword_element_wave_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "movs r3,#0x30\n"
        "ldrb r3,[r7,r3]\n"
        "cmp r3,#10\n"
        "bne 1f\n"
        "movs r2,#5\n"
        "ldrh r3,[r7,#0x12]\n"
        "subs r2,r2,r3\n"
        "cmp r2,#1\n"
        "beq 1f\n"
        "adds r2,#1\n"
        "cmp r2,#5\n"
        "bne 1f\n"
        "movs r2,#2\n"
        "1:\n"
        "ldrh r3,[r7,#6]\n"
        "adds r6,r6,r3\n"
        "ldr r4,[r7,#0x0c]\n"
        "bx lr\n"
    );
}

/*
 * Preserve SonicBoom's velocity and tag only ElementSonic waves one through
 * three in shell byte +0x0F. The native instructions after the six-byte patch
 * are skipped because this helper owns the complete null-check/velocity block.
 */
NAKED void variable_sword_sonic_shell_finalize(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "tst r0,r0\n"
        "beq 1f\n"
        "movs r1,#8\n"
        "lsls r1,r1,#16\n"
        "str r1,[r0,#0x40]\n"
        "movs r1,#0x30\n"
        "ldrb r1,[r7,r1]\n"
        "cmp r1,#10\n"
        "bne 1f\n"
        "ldrh r1,[r7,#0x12]\n"
        "cmp r1,#1\n"
        "beq 1f\n"
        "movs r1,#1\n"
        "strb r1,[r0,#0x0f]\n"
        "1:\n"
#if FALZAR
        "ldr r3,=0x080EFA0D\n"
#else
        "ldr r3,=0x080F0D4D\n"
#endif
        "bx r3\n"
    );
}

/*
 * Native shell 0x58 uses hit modifier 3 (stagger plus fixed flashing
 * invulnerability). Tagged waves use modifier 1 (stagger only); the untagged
 * Aqua wave and all native shell callers keep modifier 3.
 */
NAKED void variable_sword_sonic_hit_modifier(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "movs r1,#7\n"
        "movs r2,#5\n"
        "ldrb r3,[r5,#0x0f]\n"
        "cmp r3,#0\n"
        "beq 1f\n"
        "movs r3,#1\n"
        "bx lr\n"
        "1:\n"
        "ldrb r3,[r5,#4]\n"
        "bx lr\n"
    );
}
