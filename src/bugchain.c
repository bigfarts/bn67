#include "runtime.h"

EXE6_SPRITE(bugchain_battle_sprite, "build/bugchain-battle-sprite.bin");
EXE6_SONG(
    bugchain_aura_song,
    EXE6_PCM(
        bugchain_sound,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xD7,0x2E,0x7F,0x88,0xDB,0x39,0x8C,0xB1\n",
        "build/bugchain-sound-sample.bin"
    )
);
EXE6_INCBIN(bugchain_icon, "build/bugchain-icon.bin");
EXE6_INCBIN(bugchain_image, "build/bugchain-image.bin");
EXE6_INCBIN(bugchain_palette, "build/bugchain-palette.bin");

static const uint16_t EFFECT_FRAMES = 60;
static const uint16_t VISUAL_FRAMES = 50;
static const uint16_t SOUND_FRAME = 42;

enum BugPropertyOffset {
    MOVEMENT_BUG_PROPERTY_OFFSET = 0x31,
    BLOCK_BUG_PROPERTY_OFFSET = 0x13,
    BUSTER_BUG_PROPERTY_OFFSET = 0x14,
    DAMAGE_HP_BUG_PROPERTY_OFFSET = 0x16,
    CUSTOM_DAMAGE_BUG_PROPERTY_OFFSET = 0x54,
    EMOTION_BUG_PROPERTY_OFFSET = 0x24,
    CUSTOM_HP_BUG_PROPERTY_OFFSET = 0x19,
    BATTLE_HP_BUG_PROPERTY_OFFSET = 0x18,
    COLOR_BUG_PROPERTY_OFFSET = 0x1A,
    CUSTOM_BUG_PROPERTY_OFFSET = 0x63,
};

static const uint8_t BYTE_PROPERTIES[] = {
    MOVEMENT_BUG_PROPERTY_OFFSET,
    BLOCK_BUG_PROPERTY_OFFSET,
    BUSTER_BUG_PROPERTY_OFFSET,
    DAMAGE_HP_BUG_PROPERTY_OFFSET,
    EMOTION_BUG_PROPERTY_OFFSET,
    CUSTOM_HP_BUG_PROPERTY_OFFSET,
    BATTLE_HP_BUG_PROPERTY_OFFSET,
    COLOR_BUG_PROPERTY_OFFSET,
    CUSTOM_BUG_PROPERTY_OFFSET,
};

static void transfer_bugs(Exe6Obj *controller)
{
    uint32_t source_side = controller->owner;
    uint8_t *source = exe6_navi_status_work_adrs_get(source_side);
    uint8_t *target = exe6_navi_status_work_adrs_get(source_side ^ 1u);

    for (size_t index = 0;
         index < sizeof(BYTE_PROPERTIES) / sizeof(BYTE_PROPERTIES[0]);
         ++index) {
        size_t offset = BYTE_PROPERTIES[index];
        uint8_t value = source[offset];
        if (value != 0 && value > target[offset]) {
            target[offset] = value;
        }
    }

    /*
     * Custom Damage stores an HP-loss amount; the other bugs store byte
     * severities.
     */
    uint16_t source_value =
        *(uint16_t *)(source + CUSTOM_DAMAGE_BUG_PROPERTY_OFFSET);
    uint16_t *target_value =
        (uint16_t *)(target + CUSTOM_DAMAGE_BUG_PROPERTY_OFFSET);
    if (source_value != 0 && source_value > *target_value) {
        *target_value = source_value;
    }
}

static void spawn_visual(
    Exe6Obj *player,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *visual = exe6_efc_open(
        EXE6_OBJ_ID(bugchain_visual_main), spawn_parameters
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
    if (controller->substate == 0) {
        if ((exe6_em_set_flag_get() & EXE6_BATTLE_CONFIG_FLAG_LINK) == 0) {
            controller->phase = 0x0C;
            controller->phase_timer = 0;
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
        controller->substate = 4;
    }

    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if ((int16_t)timer < 0) {
        transfer_bugs(controller);
        controller->phase = 0x0C;
        controller->phase_timer = 0;
    }
}

static void update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    switch (controller->phase) {
    case 0:
        exe6_event_chip_common_fade();
        break;
    case 4:
        exe6_event_chip_common_telop();
        break;
    case 8:
        effect_update(controller, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

EXE6_EFFECT(bugchain_controller_main)
{
    switch (self->state) {
    case 0:
        exe6_event_chip_common_init();
        break;
    case 4:
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
        exe6_sound_req(EXE6_SONG_ID(bugchain_aura_song));
    }

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        visual->state_word = 8;
    }
    copy_coords(visual);
}

static void visual_init(Exe6Obj *visual)
{
    exe6_obj_char_init(
        0x80,
        EXE6_SPRITE_GROUP(bugchain_battle_sprite),
        EXE6_SPRITE_ID(bugchain_battle_sprite)
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
    visual->state_word = 4;
    visual_update(visual);
}

EXE6_EFFECT(bugchain_visual_main)
{
    switch (self->state) {
    case 0:
        visual_init(self);
        break;
    case 4:
        visual_update(self);
        break;
    default:
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

EXE6_PERSISTENT_ATTACK(0x0BE, bugchain_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        EXE6_OBJ_ID(bugchain_controller_main), spawn_parameters
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
