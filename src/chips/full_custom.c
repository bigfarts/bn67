#include "runtime.h"

#if FALZAR
#define ICON ((const uint8_t *)0x0872AF94u)
#define IMAGE ((const uint8_t *)0x08712EF4u)
#define PALETTE ((const uint8_t *)0x08724AD4u)
#else
#define ICON ((const uint8_t *)0x08728ED0u)
#define IMAGE ((const uint8_t *)0x08710E30u)
#define PALETTE ((const uint8_t *)0x08722A10u)
#endif

BN67_CHIP_RECORD(0x0ae) {
    .codes = {
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 51,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_CHIP_TRADER |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x00,
        .family = 0x1C,
        .subfamily = 0x05,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0x14,
    },
    .library_number = 0xB0,
    .library_flags = 0x80,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00B0,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE,
};
