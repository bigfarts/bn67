#include "runtime.h"

BN67_FIXED_SPRITE(
    0x0C,
    0x49,
    otenko_battle_sprite,
    "build/otenko-battle-sprite.bin"
);
BN67_INCBIN(otenko_icon, "build/otenko-icon.bin");
BN67_INCBIN(otenko_image, "build/otenko-image.bin");
BN67_INCBIN(otenko_palette, "build/otenko-palette.bin");

/* Original BR5J chip 0x099. */
BN67_CHIP_RECORD(0x099) {
    .codes = {
        EXE6_CHIP_CODE_O,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_OBSTACLE,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 66,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0,
        .family = 0x15,
        .subfamily = 0x12,
        .dark_soul_usage = 0x03,
        .unknown_0e = 0,
        .lock_on = 0,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x02,
    .library_flags = 0x01,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 1,
    .library_sort_order = 0x00CA,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = otenko_icon,
    .image = otenko_image,
    .palette = otenko_palette,
};
