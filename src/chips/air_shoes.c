#include "runtime.h"

BN67_INCBIN(air_shoes_icon, "build/air_shoes_icon.bin");
BN67_INCBIN(air_shoes_image, "build/air_shoes_image.bin");
BN67_INCBIN(air_shoes_palette, "build/air_shoes_palette.bin");
BN67_SPRITE(air_shoes_battle_sprite, "build/air_shoes_battle_sprite.bin");
BN67_SONG(
    air_shoes_activate_song,
    BN67_PCM(
        air_shoes_activate,
        0x60,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBE,0x7F\n"
        ".byte 0xBF,0x40,0xE0,0x48,0x7F,0x91,0xB1\n",
        "build/air_shoes_activate_sample.bin"
    )
);

/* BN3 chip 0xA8, retaining AirShot's wildcard and Standard-library slot. */
BN67_CHIP_RECORD(0x004) {
    .codes = {
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 26,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_CHIP_TRADER |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0,
        .family = BN67_ATTACK_FAMILY(air_shoes_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(air_shoes_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x04,
    .library_flags = 0x80,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x0004,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = air_shoes_icon,
    .image = air_shoes_image,
    .palette = air_shoes_palette,
};

static const uint32_t AIR_SHOES_PROPERTY = 0x1C;
static const uint16_t ACTIVATION_FRAMES = 10;
static const uint16_t HOLD_FRAMES = 30;

enum EffectStep {
    EFFECT_STEP_INIT,
    EFFECT_STEP_ACTIVATE = 4,
    EFFECT_STEP_HOLD = 8,
};

static void effect_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *player = controller->parent;
    switch (controller->substate) {
    case EFFECT_STEP_INIT: {
        Exe6Obj *visual = exe6_efc_open(
            BN67_OBJ_ID(air_shoes_visual_main), spawn_parameters
        );
        if (visual != NULL) {
            visual->parent = player;
            visual->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
        }
        exe6_sound_req(BN67_SONG_ID(air_shoes_activate_song));
        controller->timer = ACTIVATION_FRAMES;
        controller->substate = EFFECT_STEP_ACTIVATE;
        break;
    }
    case EFFECT_STEP_ACTIVATE:
        if (--controller->timer == 0) {
            /* Persist through Custom and form changes, and activate now.
             * Native Uninstall clears both this property and the live flag. */
            exe6_navi_status_set(player->owner, AIR_SHOES_PROPERTY, 1);
            exe6_battle_hit_status_flag_on(
                player, EXE6_HIT_STATUS_FLAG_AIR_SHOES
            );
            controller->timer = HOLD_FRAMES;
            controller->substate = EFFECT_STEP_HOLD;
        }
        break;
    default:
        if (--controller->timer == 0) {
            controller->phase = EXE6_EVENT_CHIP_PHASE_OUTRO;
            controller->phase_timer = 0;
        }
        break;
    }
}

BN67_EFFECT(air_shoes_controller_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        exe6_event_chip_common_init();
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        switch (self->phase) {
        case EXE6_EVENT_CHIP_PHASE_FADE:
            exe6_event_chip_common_fade();
            break;
        case EXE6_EVENT_CHIP_PHASE_TELOP:
            exe6_event_chip_common_telop();
            break;
        case EXE6_EVENT_CHIP_PHASE_EFFECT:
            effect_update(self, spawn_parameters);
            break;
        default:
            exe6_event_chip_common_end();
            break;
        }
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_EFFECT(air_shoes_visual_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        exe6_obj_char_init(
            0x80,
            BN67_SPRITE_GROUP(air_shoes_battle_sprite),
            BN67_SPRITE_ID(air_shoes_battle_sprite)
        );
        exe6_obj_no_shadow();
        self->animation_word = 0;
        exe6_obj_dma_seq_set(0);
        exe6_obj_char_set();
        exe6_obj_flip_set(self->parent->owner);
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    } else if (self->state != EXE6_OBJECT_STATE_ACTIVE ||
               (exe6_obj_seq_info_get() & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        exe6_obj_move_delete();
        return;
    }
    self->x = self->parent->x;
    self->y = self->parent->y;
    self->z = self->parent->z;
    exe6_battle_obj_char_move2();
}

BN67_PERSISTENT_ATTACK(0x004, air_shoes_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(air_shoes_controller_main), spawn_parameters
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
