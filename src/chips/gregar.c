#include "runtime.h"

BN67_FIXED_COMPRESSED_SPRITE(
    0x0C,
    0x68,
    gregar_battle_sprite,
    "build/gregar_battle_sprite.bin"
);
BN67_INCBIN(gregar_icon, "build/gregar_icon.bin");
BN67_INCBIN(gregar_image, "build/gregar_image.bin");
BN67_ASM_RESOURCE(
    gregar_palette,
    ".byte 0x00,0x00,0xFF,0x77,0x9E,0x47,0x3F,0x1F\n"
    ".byte 0x7D,0x0A,0x77,0x0D,0xF4,0x04,0x51,0x00\n"
    ".byte 0x89,0x10,0xA3,0x18,0x5F,0x4D,0x87,0x37\n"
    ".byte 0x90,0x7F,0xCC,0x5A,0x09,0x36,0x26,0x21\n"
);

BN67_FIXED_OBJECT(1, 0x30, gregar_child_main);
BN67_FIXED_OBJECT(4, 0x7A, gregar_controller_main);
BN67_FIXED_OBJECT(4, 0x7D, gregar_shared_aux_main);
BN67_FIXED_ATTACK(0x15, 0x22, gregar_attack_main);

BN67_CHIP_RECORD(0x138) {
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
        .subfamily = 0x22,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0,
        .lock_on = 0,
        .object_spawn = { .variant = 0x64 },
        .delay = 0,
    },
    .library_number = 0x0C,
    .library_flags = 0x10,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 300,
    .library_sort_order = 0x0138,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = gregar_icon,
    .image = gregar_image,
    .palette = gregar_palette,
};
