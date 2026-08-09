#include "runtime.h"

BN6_SPRITE(bugchain_battle_sprite, "build/bugchain-battle-sprite.bin");
BN6_SONG(
    bugchain_aura_song,
    BN6_PCM(
        bugchain_sound,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xD7,0x2E,0x7F,0x88,0xDB,0x39,0x8C,0xB1\n",
        "build/bugchain-sound-sample.bin"
    )
);
BN6_INCBIN(bugchain_icon, "build/bugchain-icon.bin");
BN6_INCBIN(bugchain_image, "build/bugchain-image.bin");
BN6_INCBIN(bugchain_palette, "build/bugchain-palette.bin");

static const uint16_t EFFECT_FRAMES = 60;
static const uint16_t VISUAL_FRAMES = 50;
static const uint16_t SOUND_FRAME = 42;
static const size_t HALFWORD_PROPERTY_OFFSET = 0x54;

static const uint8_t BYTE_PROPERTIES[] = {
    0x31, 0x13, 0x14, 0x16, 0x24, 0x19, 0x18, 0x1A, 0x63,
};

static void transfer_bugs(Object *controller)
{
    uint32_t source_side = controller->owner;
    uint8_t *source = bn6_player_properties_for_side(source_side);
    uint8_t *target = bn6_player_properties_for_side(source_side ^ 1u);

    for (size_t index = 0;
         index < sizeof(BYTE_PROPERTIES) / sizeof(BYTE_PROPERTIES[0]);
         ++index) {
        size_t offset = BYTE_PROPERTIES[index];
        uint8_t value = source[offset];
        if (value != 0 && value > target[offset]) {
            target[offset] = value;
        }
    }

    uint16_t source_value = *(uint16_t *)(source + HALFWORD_PROPERTY_OFFSET);
    uint16_t *target_value =
        (uint16_t *)(target + HALFWORD_PROPERTY_OFFSET);
    if (source_value != 0 && source_value > *target_value) {
        *target_value = source_value;
    }
}

static void spawn_visual(Object *player, uint32_t spawn_argument)
{
    Object *visual = bn6_spawn_type4(
        BN6_OBJECT_ID(bugchain_visual_main), spawn_argument
    );
    if (visual == NULL) {
        return;
    }
    visual->parent = player;
}

static void effect_update(Object *controller, uint32_t spawn_argument)
{
    if (controller->substate == 0) {
        if ((bn6_battle_get_config_flags() & BN6_BATTLE_CONFIG_FLAG_LINK) == 0) {
            controller->phase = 0x0C;
            controller->phase_timer = 0;
            return;
        }

        Object *player = bn6_player_object_for_side(0);
        if (player != NULL) {
            spawn_visual(player, spawn_argument);
        }
        player = bn6_player_object_for_side(1);
        if (player != NULL) {
            spawn_visual(player, spawn_argument);
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

static void update(Object *controller, uint32_t spawn_argument)
{
    switch (controller->phase) {
    case 0:
        bn6_self_type4_timestop_intro();
        break;
    case 4:
        bn6_self_type4_timestop_freeze();
        break;
    case 8:
        effect_update(controller, spawn_argument);
        break;
    default:
        bn6_self_type4_timestop_outro();
        break;
    }
}

BN6_OBJECT4(bugchain_controller_main)
{
    switch (self->state) {
    case 0:
        bn6_self_type4_timestop_init();
        break;
    case 4:
        update(self, spawn_argument);
        break;
    default:
        bn6_self_type4_timestop_free();
        break;
    }
}

static void copy_coords(Object *visual)
{
    Object *player = visual->parent;
    if (player == NULL) {
        return;
    }
    visual->x = player->x;
    visual->y = player->y;
    visual->z = player->z;
}

static void visual_update(Object *visual)
{
    if (visual->timer == SOUND_FRAME) {
        bn6_play_sound(BN6_SONG_ID(bugchain_aura_song));
    }

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        visual->state_word = 8;
    }
    copy_coords(visual);
}

static void visual_init(Object *visual)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(bugchain_battle_sprite),
        BN6_SPRITE_ID(bugchain_battle_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    visual->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    visual->owner = visual->parent->owner;
    bn6_self_sprite_set_flip(visual->owner);
    visual->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    visual->timer = VISUAL_FRAMES;
    visual->state_word = 4;
    visual_update(visual);
}

BN6_OBJECT4(bugchain_visual_main)
{
    switch (self->state) {
    case 0:
        visual_init(self);
        break;
    case 4:
        visual_update(self);
        break;
    default:
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_timestop();
}

BN6_ATTACK(0x0BE, bugchain_attack_main)
{
    Object *controller = bn6_spawn_type4(
        BN6_OBJECT_ID(bugchain_controller_main), spawn_argument
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
