#include "runtime.h"


extern const uint8_t jealousy_effect_tiles[0x100];
extern const uint8_t jealousy_effect_palette[0x20];

BN6_INCBIN(jealousy_icon, "build/jealousy-icon.bin");
BN6_INCBIN(jealousy_image, "build/jealousy-image.bin");
BN6_INCBIN(jealousy_palette, "build/jealousy-palette.bin");
BN6_INCBIN(jealousy_effect_tiles, "build/jealousy-effect-tiles.bin");
BN6_INCBIN(jealousy_effect_palette, "build/jealousy-effect-palette.bin");

static const uintptr_t TILES_DESTINATION = 0x06017940u;
static const uint16_t PULSE_DELAY = 10;
static const uint16_t DELETE_FRAMES = 90;
static const uint16_t OVERLAY_LAST_FRAME = 20;
static const uint32_t GAUGE_FULL = 0x4000;
static const uint32_t DAMAGE_PROPERTIES = 0x1A050601;

struct JealousyWork {
    uint32_t remaining_attacks;          // +0x60
};

static uint32_t unit_loaded_chips(Object *unit)
{
    const uint8_t *runtime = (const uint8_t *)unit->runtime_data;
    if (runtime == NULL) {
        return 0;
    }
    if (runtime[0] != 2) {
        return unit->loaded_chip_count;
    }

    const uint8_t *list = bn6_chip_list(unit->owner);
    const uint16_t *entry =
        (const uint16_t *)(list + 2u + 2u * list[0]);
    uint32_t count = 0;
    while (*entry != UINT16_MAX) {
        ++count;
        ++entry;
    }
    return count;
}

static uint32_t max_loaded_chips(Object *controller)
{
    Object *const *units =
        bn6_battle_units_for_side(controller->owner ^ 1u);
    uint32_t maximum = 0;
    for (size_t index = 0; index < 4; ++index) {
        Object *unit = units[index];
        if (unit == NULL) {
            continue;
        }
        uint32_t count = unit_loaded_chips(unit);
        if (count >= maximum) {
            maximum = count;
        }
    }
    return maximum;
}

static uint32_t attack_field(Object *controller)
{
    uint32_t opposing_navi_flag = controller->owner == 0
        ? BN6_PANEL_FLAG_SIDE_1_NAVI
        : BN6_PANEL_FLAG_SIDE_0_NAVI;
    uint32_t hits = 0;
    for (uint32_t panel_x = 6; panel_x != 0; --panel_x) {
        for (uint32_t panel_y = 3; panel_y != 0; --panel_y) {
            if (bn6_panel_matches_flags(
                    panel_x,
                    panel_y,
                    opposing_navi_flag,
                    0
                ) == 0) {
                continue;
            }
            bn6_spawn_panel_damage(
                panel_x,
                panel_y,
                controller->parameter,
                0,
                DAMAGE_PROPERTIES,
                controller->attack + controller->attack_bonus,
                3
            );
            ++hits;
        }
    }
    return hits;
}

static void finish_delete(Object *controller)
{
    if (bn6_link_battle_active() != 0) {
        bn6_gauge_subtract(controller->owner ^ 1u, GAUGE_FULL);
    }
}

static void refresh_delete_overlay(Object *controller)
{
    if (
        bn6_link_battle_active() != 0
        && bn6_compare_local_side(controller->owner) != 0
    ) {
        bn6_draw_delete_overlay(120, 12);
    }
}

static void delete_phase(Object *controller)
{
    if (controller->substate == 0) {
        controller->timer = DELETE_FRAMES;
        controller->substate = 4;
    }
    if (controller->timer >= OVERLAY_LAST_FRAME) {
        refresh_delete_overlay(controller);
    }

    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if ((int16_t)timer >= 0) {
        return;
    }

    finish_delete(controller);
    controller->phase = 0x0C;
    controller->phase_timer = 0;
}

static void pulse_phase(Object *controller)
{
    struct JealousyWork *work =
        (struct JealousyWork *)controller->work;
    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if (timer != 0) {
        return;
    }

    controller->timer = PULSE_DELAY;
    if (attack_field(controller) == 0 || --work->remaining_attacks == 0) {
        controller->phase_timer = 8;
    }
}

static void effect_init(Object *controller)
{
    struct JealousyWork *work =
        (struct JealousyWork *)controller->work;
    bn6_display_setup(
        jealousy_effect_tiles,
        (void *)TILES_DESTINATION,
        sizeof(jealousy_effect_tiles)
    );
    bn6_display_setup(
        jealousy_effect_palette,
        (void *)BN6_PALETTE_BG_STAGING_0D,
        sizeof(jealousy_effect_palette)
    );

    work->remaining_attacks = max_loaded_chips(controller);
    if (work->remaining_attacks == 0) {
        controller->phase_timer = 8;
        return;
    }
    controller->timer = 1;
    controller->phase_timer = 4;
}

static void effect_update(Object *controller)
{
    switch (controller->phase_timer) {
    case 0:
        effect_init(controller);
        break;
    case 4:
        pulse_phase(controller);
        break;
    default:
        delete_phase(controller);
        break;
    }
}

static void update(Object *controller)
{
    switch (controller->phase) {
    case 0:
        bn6_self_type4_timestop_intro();
        break;
    case 4:
        bn6_self_type4_timestop_freeze();
        break;
    case 8:
        effect_update(controller);
        break;
    default:
        bn6_self_type4_timestop_outro();
        break;
    }
}

BN6_OBJECT4(jealousy_controller_main)
{
    switch (self->state) {
    case 0:
        bn6_self_type4_timestop_init();
        break;
    case 4:
        update(self);
        break;
    default:
        bn6_self_type4_timestop_free();
        break;
    }
}

BN6_ATTACK(0x0BF, jealousy_attack_main)
{
    Object *controller = bn6_spawn_type4(
        BN6_OBJECT_ID(jealousy_controller_main), spawn_argument
    );
    if (controller == NULL) {
        return;
    }
    controller->panel_x = (uint8_t)panel_x;
    controller->panel_y = (uint8_t)panel_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
}
