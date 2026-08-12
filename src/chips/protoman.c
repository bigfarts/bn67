#include "runtime.h"

/* DeltaRay is the final native family-0x1B summon in both BN6 editions. */
#define DELTARAY_FAMILY 0x1B
#define DELTARAY_SUBFAMILY 0x1C
#define DELTARAY_ANIMATION_STATE 0x0C

#if FALZAR
#define ICON ((const uint8_t *)0x0872BE14u)
#define IMAGE ((const uint8_t *)0x0871ACF4u)
#define PALETTE_BASE ((const uint8_t *)0x08724ED4u)
#define PALETTE_EX ((const uint8_t *)0x08724EF4u)
#define PALETTE_SP ((const uint8_t *)0x08724F14u)
#else
#define ICON ((const uint8_t *)0x08729D50u)
#define IMAGE ((const uint8_t *)0x08718C30u)
#define PALETTE_BASE ((const uint8_t *)0x08722E10u)
#define PALETTE_EX ((const uint8_t *)0x08722E30u)
#define PALETTE_SP ((const uint8_t *)0x08722E50u)
#endif

BN67_CHIP_RECORD(0x0e0) {
    .codes = {
        EXE6_CHIP_CODE_B,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 41,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = DELTARAY_FAMILY,
        .subfamily = DELTARAY_SUBFAMILY,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = DELTARAY_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x04,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 80,
    .library_sort_order = 0x00E0,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_BASE,
};

BN67_CHIP_RECORD(0x0e1) {
    .codes = {
        EXE6_CHIP_CODE_B,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 53,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = DELTARAY_FAMILY,
        .subfamily = DELTARAY_SUBFAMILY,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = DELTARAY_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x05,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 100,
    .library_sort_order = 0x00E1,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_EX,
};

BN67_CHIP_RECORD(0x0e2) {
    .codes = {
        EXE6_CHIP_CODE_B,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 68,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = DELTARAY_FAMILY,
        .subfamily = DELTARAY_SUBFAMILY,
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = DELTARAY_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x06,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 200,
    .library_sort_order = 0x00E2,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_SP,
};
