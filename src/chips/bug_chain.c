#include "runtime.h"

BN67_SPRITE(bug_chain_battle_sprite, "build/bug_chain_battle_sprite.bin");
BN67_SONG(
    bug_chain_aura_song,
    BN67_PCM(
        bug_chain_sound,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xD7,0x2E,0x7F,0x88,0xDB,0x39,0x8C,0xB1\n",
        "build/bug_chain_sound_sample.bin"
    )
);
BN67_INCBIN(bug_chain_icon, "build/bug_chain_icon.bin");
BN67_INCBIN(bug_chain_image, "build/bug_chain_image.bin");
BN67_INCBIN(bug_chain_palette, "build/bug_chain_palette.bin");

BN67_CHIP_RECORD(0x0be) {
    .codes = {
        EXE6_CHIP_CODE_C,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 59,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x00,
        .family = BN67_ATTACK_FAMILY(bug_chain_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(bug_chain_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0xC2,
    .library_flags = 0x80,
    .library_lock_on_type = 0x01,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00C2,
    .library_gate_usage = 0x03,
    .dark_chip_id = UINT8_MAX,
    .icon = bug_chain_icon,
    .image = bug_chain_image,
    .palette = bug_chain_palette,
};

static const uint16_t EFFECT_FRAMES = 60;
static const uint16_t VISUAL_FRAMES = 50;
static const uint16_t SOUND_FRAME = 42;

enum EffectStep {
    EFFECT_STEP_INIT,
    EFFECT_STEP_ACTIVE = 4,
};

static bool transfer_bug(uint8_t source, uint8_t *target)
{
    if (source == 0 || source <= *target) {
        return false;
    }
    *target = source;
    return true;
}

static void transfer_bugs(Exe6Obj *controller)
{
    uint32_t source_side = controller->owner;
    Exe6NaviStatusWork *source =
        exe6_navi_status_work_adrs_get(source_side);
    Exe6NaviStatusWork *target =
        exe6_navi_status_work_adrs_get(source_side ^ 1u);

    transfer_bug(source->movement_bug, &target->movement_bug);
    transfer_bug(source->damage_hp_bug, &target->damage_hp_bug);
    transfer_bug(source->emotion_bug, &target->emotion_bug);
    transfer_bug(source->custom_hp_bug, &target->custom_hp_bug);
    transfer_bug(source->battle_hp_bug, &target->battle_hp_bug);
    transfer_bug(source->color_bug, &target->color_bug);
    transfer_bug(source->custom_bug, &target->custom_bug);

    /*
     * Block and Buster bugs each have a companion byte that describes the
     * effect selected by their severity byte.  Copy the pair together when
     * BugChain replaces the target's weaker bug; copying only the severity
     * makes a CrackStep bug use action zero (an empty panel), and can leave a
     * Buster bug with a level belonging to the target's old severity.
     */
    if (transfer_bug(
        source->block_bug_severity,
        &target->block_bug_severity
    )) {
        target->block_bug_action = source->block_bug_action;
    }

    if (transfer_bug(
        source->buster_bug_severity,
        &target->buster_bug_severity
    )) {
        target->buster_bug_level = source->buster_bug_level;
    }

    /*
     * Custom Damage stores an HP-loss amount; the other bugs store byte
     * severities.
     */
    uint16_t source_damage = source->custom_damage_bug;
    if (
        source_damage != 0
        && source_damage > target->custom_damage_bug
    ) {
        target->custom_damage_bug = source_damage;
    }
}

static void spawn_visual(
    Exe6Obj *player,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *visual = exe6_efc_open(
        BN67_OBJ_ID(bug_chain_visual_main), spawn_parameters
    );
    if (visual == NULL) {
        return;
    }
    visual->parent = player;
}

static void effect_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    if (controller->substate == EFFECT_STEP_INIT) {
        if ((exe6_em_set_flag_get() & EXE6_BATTLE_CONFIG_FLAG_LINK) == 0) {
            controller->phase = EXE6_EVENT_CHIP_PHASE_OUTRO;
            controller->phase_timer = EFFECT_STEP_INIT;
            return;
        }

        Exe6Obj *player = exe6_get_navi_adrs(0);
        if (player != NULL) {
            spawn_visual(player, spawn_parameters);
        }
        player = exe6_get_navi_adrs(1);
        if (player != NULL) {
            spawn_visual(player, spawn_parameters);
        }
        controller->timer = EFFECT_FRAMES;
        controller->substate = EFFECT_STEP_ACTIVE;
    }

    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if ((int16_t)timer < 0) {
        transfer_bugs(controller);
        controller->phase = EXE6_EVENT_CHIP_PHASE_OUTRO;
        controller->phase_timer = EFFECT_STEP_INIT;
    }
}

static void update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    switch (controller->phase) {
    case EXE6_EVENT_CHIP_PHASE_FADE:
        exe6_event_chip_common_fade();
        break;
    case EXE6_EVENT_CHIP_PHASE_TELOP:
        exe6_event_chip_common_telop();
        break;
    case EXE6_EVENT_CHIP_PHASE_EFFECT:
        effect_update(controller, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

BN67_EFFECT(bug_chain_controller_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        exe6_event_chip_common_init();
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        update(self, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

static void copy_coords(Exe6Obj *visual)
{
    Exe6Obj *player = visual->parent;
    if (player == NULL) {
        return;
    }
    visual->x = player->x;
    visual->y = player->y;
    visual->z = player->z;
}

static void visual_update(Exe6Obj *visual)
{
    if (visual->timer == SOUND_FRAME) {
        exe6_sound_req(BN67_SONG_ID(bug_chain_aura_song));
    }

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        visual->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
    copy_coords(visual);
}

static void visual_init(Exe6Obj *visual)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(bug_chain_battle_sprite),
        BN67_SPRITE_ID(bug_chain_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    visual->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_char_move();
    visual->owner = visual->parent->owner;
    exe6_obj_flip_set(visual->owner);
    visual->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    visual->timer = VISUAL_FRAMES;
    visual->state_word = EXE6_OBJECT_STATE_ACTIVE;
    visual_update(visual);
}

BN67_EFFECT(bug_chain_visual_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        visual_init(self);
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        visual_update(self);
        break;
    default:
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

BN67_PERSISTENT_ATTACK(0x0BE, bug_chain_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(bug_chain_controller_main), spawn_parameters
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
