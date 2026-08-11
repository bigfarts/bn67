#include "runtime.h"

BN67_INCBIN(jealousy_icon, "build/jealousy-icon.bin");
BN67_INCBIN(jealousy_image, "build/jealousy-image.bin");
BN67_INCBIN(jealousy_palette, "build/jealousy-palette.bin");

extern const uint8_t jealousy_effect_tiles[0x100];
BN67_INCBIN(jealousy_effect_tiles, "build/jealousy-effect-tiles.bin");

extern const uint8_t jealousy_effect_palette[0x20];
BN67_INCBIN(jealousy_effect_palette, "build/jealousy-effect-palette.bin");

BN67_CHIP_RECORD(0x0bf) {
    .codes = {
        EXE6_CHIP_CODE_J,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 60,
    .behavior = {
        .effect_flags = 0x43,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(jealousy_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(jealousy_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x00,
    .library_flags = 0x80,
    .library_lock_on_type = 0x10,
    .alphabetical_sort = 0,
    .power = 80,
    .library_sort_order = 0x00C3,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = jealousy_icon,
    .image = jealousy_image,
    .palette = jealousy_palette,
};

static const uintptr_t TILES_DESTINATION = 0x06017940u;
static const uint16_t PULSE_DELAY = 10;
static const uint16_t DELETE_FRAMES = 90;
static const uint16_t OVERLAY_LAST_FRAME = 20;
static const uint32_t GAUGE_FULL = 0x4000;

static const Exe6BlockDamageProperties DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CURRENT_BLOCK,
    .hit_effect = EXE6_HIT_EFFECT_SMALL_IMPACT,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_1A,
};

struct JealousyWork {
    uint32_t remaining_attacks;          // +0x60
};

static uint32_t unit_loaded_chips(Exe6Obj *unit)
{
    const Exe6PlayerRuntime *runtime = unit->runtime_data;
    if (runtime == NULL) {
        return 0;
    }
    if (runtime->type != 2) {
        return unit->loaded_chip_count;
    }

    const Exe6NaviSelectChipWork *selection =
        exe6_navi_select_chip_work_adrs_get(unit->owner);
    const uint16_t *entry =
        &selection->chip_ids[selection->active_chip_index];
    uint32_t count = 0;
    while (*entry != UINT16_MAX) {
        ++count;
        ++entry;
    }
    return count;
}

static uint32_t max_loaded_chips(Exe6Obj *controller)
{
    Exe6Obj *const *units =
        exe6_runtime()->battle_context->battle_units[controller->owner ^ 1u];
    uint32_t maximum = 0;
    for (size_t index = 0; index < 4; ++index) {
        Exe6Obj *unit = units[index];
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

static uint32_t attack_field(Exe6Obj *controller)
{
    uint32_t opposing_navi_flag = controller->owner == 0
        ? EXE6_BLOCK_FLAG_SIDE_1_NAVI
        : EXE6_BLOCK_FLAG_SIDE_0_NAVI;
    uint32_t hits = 0;
    for (uint32_t block_x = 6; block_x != 0; --block_x) {
        for (uint32_t block_y = 3; block_y != 0; --block_y) {
            if (exe6_block_move_check(
                    block_x,
                    block_y,
                    opposing_navi_flag,
                    0
                ) == 0) {
                continue;
            }
            exe6_set_shl03_ev(
                block_x,
                block_y,
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

static void finish_delete(Exe6Obj *controller)
{
    if (exe6_real_operation_battle_check() != 0) {
        exe6_operate_slot_in_gauge_sub(controller->owner ^ 1u, GAUGE_FULL);
    }
}

static void refresh_delete_overlay(Exe6Obj *controller)
{
    if (
        exe6_real_operation_battle_check() != 0
        && exe6_battle_one_self_check(controller->owner) != 0
    ) {
        exe6_yazirushi_trans(120, 12);
    }
}

static void delete_phase(Exe6Obj *controller)
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

static void pulse_phase(Exe6Obj *controller)
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

static void effect_init(Exe6Obj *controller)
{
    struct JealousyWork *work =
        (struct JealousyWork *)controller->work;
    exe6_mem_task_trans_set256(
        jealousy_effect_tiles,
        (void *)TILES_DESTINATION,
        sizeof(jealousy_effect_tiles)
    );
    exe6_mem_task_trans_set256(
        jealousy_effect_palette,
        (void *)EXE6_PALETTE_BG_STAGING_0D,
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

static void effect_update(Exe6Obj *controller)
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

static void update(Exe6Obj *controller)
{
    switch (controller->phase) {
    case 0:
        exe6_event_chip_common_fade();
        break;
    case 4:
        exe6_event_chip_common_telop();
        break;
    case 8:
        effect_update(controller);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

BN67_EFFECT(jealousy_controller_main)
{
    switch (self->state) {
    case 0:
        exe6_event_chip_common_init();
        break;
    case 4:
        update(self);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_PERSISTENT_ATTACK(0x0BF, jealousy_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(jealousy_controller_main), spawn_parameters
    );
    if (controller == NULL) {
        return NULL;
    }
    controller->block_x = (uint8_t)block_x;
    controller->block_y = (uint8_t)block_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
    return controller;
}
