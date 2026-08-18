#include "runtime.h"

BN67_SPRITE(dark_aura_battle_sprite, "build/dark_aura_battle_sprite.bin");
BN67_INCBIN(dark_aura_icon, "build/dark_aura_icon.bin");
BN67_INCBIN(dark_aura_image, "build/dark_aura_image.bin");
BN67_INCBIN(dark_aura_palette, "build/dark_aura_palette.bin");

/*
 * Barrier type 0x0F normally uses an enemy-positioned BN6 aura sprite. The
 * visual hook below redirects only this chip's marked instance to BN3's
 * centered DarkAura archive and battle palette. Extraction removes the
 * archive's numeric OAM pieces, leaving only the aura. Normal enemy users
 * retain the native sprite and 100-damage threshold. The hit hook expands DarkAura's 0xFF
 * sentinel to the otherwise-unrepresentable 300-damage threshold (the native
 * collision field is only one byte wide).
 */
BN67_PATCH_SECTION(
    0x0801A948,
    0x0802CD48,
    dark_aura_threshold_dispatch
);

#if FALZAR
BN67_PATCH_SECTION(
    0x080E0B16,
    0x080E979C,
    dark_aura_visual_sprite_dispatch
);
BN67_PATCH_SECTION(
    0x080E3B10,
    0x0802CD50,
    dark_aura_activation_dispatch
);
#define EFFECT_FLAGS                                                    \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING |                                    \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#else
BN67_PATCH_SECTION(
    0x080E1E52,
    0x080EAADC,
    dark_aura_visual_sprite_dispatch
);
BN67_PATCH_SECTION(
    0x080E4E50,
    0x0802CD50,
    dark_aura_activation_dispatch
);
#define EFFECT_FLAGS EXE6_CHIP_EFFECT_FLAG_DIMMING
#endif

/* BN3 Blue chip 0x135, installed over BN6 Falzar's BugDthTh 0x136. */
BN67_CHIP_RECORD(0x136) {
    .codes = {
        EXE6_CHIP_CODE_A,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_GIGA,
    .mb = 89,
    .behavior = {
        .effect_flags = EFFECT_FLAGS,
        .counter_settings = 0x00,
        .family = 0x15,
        .subfamily = 0x04,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .variant = 0x0F },
        .delay = 0,
    },
    /* Preserve BugDthTh's fifth Falzar Giga-library position. */
    .library_number = 0x05,
    .library_flags = 0x18,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x0136,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = dark_aura_icon,
    .image = dark_aura_image,
    .palette = dark_aura_palette,
};

NAKED void dark_aura_visual_sprite_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        /* Discard the section patch's saved r1. */
        "pop {r1}\n"
        "ldrb r0,[r5,#4]\n"
        "cmp r0,#15\n"
        "bne 1f\n"
        /* Activation marks only DarkAura's owner with threshold 0xFF. */
        "ldr r0,[r5,#0x4C]\n"
        "ldr r0,[r0,#0x54]\n"
        "ldrb r0,[r0,#0x17]\n"
        "cmp r0,#255\n"
        "bne 1f\n"
        "ldr r1,=__bn67_sprite_group_dark_aura_battle_sprite\n"
        "ldr r2,=__bn67_sprite_id_dark_aura_battle_sprite\n"
        "b 2f\n"
        "1:\n"
        /* Reproduce the native sprite group/index loads for every other aura. */
        "ldrb r1,[r4,#0]\n"
        "ldrb r2,[r4,#1]\n"
        "2:\n"
        "movs r0,#128\n"
        "bx lr\n"
        ".pool\n"
    );
}

NAKED void dark_aura_activation_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        /* Discard the section patch's saved r1. */
        "pop {r1}\n"
        /* The native barrier setter has already initialized type 0x0F. */
        "cmp r4,#15\n"
        "bne 1f\n"
        /* The controller saved immediately below us owns the activating chip. */
        "ldr r0,[sp,#0]\n"
        "ldrh r0,[r0,#0x30]\n"
        "ldr r1,=0x136\n"
        "cmp r0,r1\n"
        "bne 1f\n"
        "ldr r0,[r5,#0x54]\n"
        "movs r1,#255\n"
        "strb r1,[r0,#0x17]\n"
        "ldr r1,=3000\n"
        "strh r1,[r0,#0x1A]\n"
        "1:\n"
        /* Reproduce the three native instructions displaced by the hook. */
        "ldr r6,[r5,#0x58]\n"
        "ldr r0,[r6,#0x60]\n"
        "tst r0,r0\n"
        "bx lr\n"
        ".pool\n"
    );
}

NAKED void dark_aura_threshold_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        /* Discard the section patch's saved r1. */
        "pop {r1}\n"
        "ldrb r1,[r4,#0x17]\n"
        "cmp r1,#255\n"
        "bne 1f\n"
        "ldrb r2,[r4,#6]\n"
        "cmp r2,#15\n"
        "bne 1f\n"
        "movs r1,#75\n"
        "lsls r1,r1,#2\n"
        "1:\n"
        "cmp r0,r1\n"
        "blt 2f\n"
        /* Damage at or above the threshold continues down the break path. */
        "bx lr\n"
        "2:\n"
        /* Lower damage is rejected by the native aura cleanup path. */
        "ldr r2,=0x0801A96F\n"
        "bx r2\n"
        ".pool\n"
    );
}
