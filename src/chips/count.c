#include "runtime.h"

BN67_FIXED_COMPRESSED_SPRITE(
    0x08,
    0x16,
    count_battle_sprite,
    "build/count-battle-sprite.bin"
);
BN67_INCBIN(count_icon, "build/count-icon.bin");
BN67_INCBIN(count_image, "build/count-image.bin");
BN67_INCBIN(count_palette_base, "build/count-pal-base.bin");
BN67_INCBIN(count_palette_ex, "build/count-pal-ex.bin");
BN67_INCBIN(count_palette_sp, "build/count-pal-sp.bin");

/* These are the original BR5J Count object and attack slots. */
BN67_FIXED_OBJECT(1, 0x11, count_native_main);
BN67_FIXED_OBJECT(1, 0x12, count_native_aux_main);
BN67_FIXED_OBJECT(3, 0x0D, count_lance_main);
BN67_FIXED_ATTACK(0x1B, 0x12, count_attack_main);

#define COUNT_RECORD(chip_id, second_code, rarity_value, mb_value, spawn_variant, \
                     power_value, sort_value, palette_value)                     \
    BN67_CHIP_RECORD(chip_id) {                                                   \
        .codes = {                                                               \
            EXE6_CHIP_CODE_H, second_code, EXE6_CHIP_CODE_NONE,                  \
            EXE6_CHIP_CODE_NONE,                                                 \
        },                                                                       \
        .attack_element = 0,                                                     \
        .rarity = rarity_value,                                                  \
        .element = EXE6_CHIP_ELEMENT_NULL,                                       \
        .chip_class = EXE6_CHIP_CLASS_MEGA,                                      \
        .mb = mb_value,                                                          \
        .behavior = {                                                            \
            .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |                      \
                            EXE6_CHIP_EFFECT_FLAG_ATTACK |                        \
                            EXE6_CHIP_EFFECT_FLAG_NAVI |                          \
                            EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,             \
            .counter_settings = 0x8A,                                            \
            .family = 0x1B,                                                      \
            .subfamily = 0x12,                                                   \
            .dark_soul_usage = 0x0A,                                             \
            .unknown_0e = 0,                                                     \
            .lock_on = 0,                                                        \
            .object_spawn = { .variant = spawn_variant },                        \
            .delay = 0,                                                          \
        },                                                                       \
        .library_number = 0x28 + ((chip_id) - 0x113),                            \
        .library_flags = 0,                                                      \
        .library_lock_on_type = 0,                                               \
        .alphabetical_sort = 0,                                                  \
        .power = power_value,                                                    \
        .library_sort_order = sort_value,                                        \
        .library_gate_usage = 0x01,                                              \
        .dark_chip_id = UINT8_MAX,                                               \
        .icon = count_icon,                                                      \
        .image = count_image,                                                    \
        .palette = palette_value,                                                \
    }

COUNT_RECORD(
    0x113, EXE6_CHIP_CODE_ASTERISK, 2, 60, 0x32, 20, 0x0113,
    count_palette_base
);
COUNT_RECORD(
    0x114, EXE6_CHIP_CODE_NONE, 3, 75, 0x46, 25, 0x0114,
    count_palette_ex
);
COUNT_RECORD(
    0x115, EXE6_CHIP_CODE_NONE, 4, 89, 0x64, 0x03F9, 0x0115,
    count_palette_sp
);
