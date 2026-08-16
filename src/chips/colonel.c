#include "runtime.h"

/* CrossDivide selects the alternate mode of Colonel's native summon. */
#define FAMILY 0x1B
#define SUBFAMILY 0x11
#define CROSSDIVIDE_ANIMATION_STATE 0x01

#if FALZAR
#define ICON ((const uint8_t *)0x0872BE14u)
#define IMAGE ((const uint8_t *)0x087200F4u)
#define PALETTE_BASE ((const uint8_t *)0x087254D4u)
#define PALETTE_EX ((const uint8_t *)0x087254F4u)
#define PALETTE_SP ((const uint8_t *)0x08725514u)
#else
#define ICON ((const uint8_t *)0x08729D50u)
#define IMAGE ((const uint8_t *)0x0871E030u)
#define PALETTE_BASE ((const uint8_t *)0x08723410u)
#define PALETTE_EX ((const uint8_t *)0x08723430u)
#define PALETTE_SP ((const uint8_t *)0x08723450u)
#endif

BN67_CHIP_RECORD(0x110) {
    .codes = {
        EXE6_CHIP_CODE_C,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 45,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = FAMILY,
        .subfamily = SUBFAMILY,
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = CROSSDIVIDE_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x25,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 160,
    .library_sort_order = 0x0110,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_BASE,
};

BN67_CHIP_RECORD(0x111) {
    .codes = {
        EXE6_CHIP_CODE_C,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 70,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = FAMILY,
        .subfamily = SUBFAMILY,
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = CROSSDIVIDE_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x26,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 180,
    .library_sort_order = 0x0111,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_EX,
};

BN67_CHIP_RECORD(0x112) {
    .codes = {
        EXE6_CHIP_CODE_C,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_SWORD,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 91,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = FAMILY,
        .subfamily = SUBFAMILY,
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = { .animation_state = CROSSDIVIDE_ANIMATION_STATE },
        .delay = 0,
    },
    .library_number = 0x27,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 300,
    .library_sort_order = 0x0112,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE_SP,
};
