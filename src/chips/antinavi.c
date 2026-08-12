#include "runtime.h"

#if FALZAR
#define ICON ((const uint8_t *)0x0872B594u)
#define IMAGE ((const uint8_t *)0x08716374u)
#define PALETTE ((const uint8_t *)0x08724C54u)
#else
#define ICON ((const uint8_t *)0x087294D0u)
#define IMAGE ((const uint8_t *)0x087142B0u)
#define PALETTE ((const uint8_t *)0x08722B90u)
#endif

BN67_CHIP_RECORD(0x0ba) {
    .codes = {
        EXE6_CHIP_CODE_F,
        EXE6_CHIP_CODE_L,
        EXE6_CHIP_CODE_T,
        EXE6_CHIP_CODE_ASTERISK,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 33,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_CHIP_TRADER |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x00,
        .family = 0x15,
        .subfamily = 0x14,
        .dark_soul_usage = 0x17,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .variant = 2 },
        .delay = 0,
    },
    .library_number = 0xBC,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00BC,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE,
};
