#include "runtime.h"

BN67_FIXED_COMPRESSED_SPRITE(
    0x0C,
    0x66,
    falzar_battle_sprite,
    "build/falzar-battle-sprite.bin"
);
BN67_INCBIN(falzar_icon, "build/falzar-icon.bin");
BN67_INCBIN(falzar_image, "build/falzar-image.bin");
BN67_ASM_RESOURCE(
    falzar_palette,
    ".byte 0x00,0x00,0xDE,0x7B,0x74,0x77,0xCC,0x49\n"
    ".byte 0xEB,0x44,0x07,0x77,0x43,0x6E,0x82,0x55\n"
    ".byte 0xC1,0x40,0x3E,0x1B,0x59,0x26,0xB4,0x09\n"
    ".byte 0xDD,0x76,0x7B,0x55,0x39,0x20,0x05,0x18\n"
);

BN67_FIXED_OBJECT(1, 0x31, falzar_child_main);
BN67_FIXED_OBJECT(4, 0x7B, falzar_controller_main);
BN67_FIXED_ATTACK(0x15, 0x23, falzar_attack_main);

/*
 * The original Strike Feather path forwards Falzar's base power in r6 but
 * drops the attack bonus cached in the controller's chip data. Add it before
 * tail-calling the edition-specific native Strike Feather constructor.
 */
NAKED void falzar_strike_feather_spawn_with_bonus(void)
{
    __asm__(
        ".syntax unified\n"
        "ldrh r3,[r5,#0x32]\n"
        "adds r6,r6,r3\n"
        "ldr r3,1f\n"
        "bx r3\n"
        ".balign 4\n"
        "1:\n"
#if FALZAR
        ".word 0x080DCC71\n"
#else
        ".word 0x080DE4E1\n"
#endif
    );
}

BN67_PATCH_LINKED_CALL(
    falzar_controller_main,
    0x302,
    falzar_strike_feather_spawn_with_bonus
);

BN67_CHIP_RECORD(0x139) {
    .codes = {
        EXE6_CHIP_CODE_X,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_GIGA,
    .mb = 99,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK,
        .counter_settings = 0x8A,
        .family = 0x15,
        .subfamily = 0x23,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0,
        .lock_on = 0,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x0D,
    .library_flags = 0x10,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 100,
    .library_sort_order = 0x0139,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = falzar_icon,
    .image = falzar_image,
    .palette = falzar_palette,
};
