#include "runtime.h"

BN67_INCBIN(gundelsol_ex_icon, "build/gundelsol-ex-icon.bin");
BN67_INCBIN(gundelsol_ex_image, "build/gundelsol-ex-image.bin");
BN67_INCBIN(gundelsol_ex_palette, "build/gundelsol-ex-palette.bin");

/* Original BR5J chip 0x012. Its family-0x37 attack remains native in BN6. */
BN67_CHIP_RECORD(0x012) {
    .codes = {
        EXE6_CHIP_CODE_G,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 80,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0,
        .family = 0x37,
        .subfamily = 0x03,
        .dark_soul_usage = 0,
        .unknown_0e = 0x04,
        .lock_on = 0x01,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x01,
    .library_flags = 0x01,
    .library_lock_on_type = 0x10,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00C9,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = gundelsol_ex_icon,
    .image = gundelsol_ex_image,
    .palette = gundelsol_ex_palette,
};
