#include "runtime.h"

BN6_SPRITE(bugcharge_gospel_sprite, "build/bugcharge-gospel-sprite.bin");

BN6_INCBIN(bugcharge_icon, "build/bugcharge-icon.bin");
BN6_INCBIN(bugcharge_image, "build/bugcharge-image.bin");
BN6_INCBIN(bugcharge_palette, "build/bugcharge-palette.bin");
BN6_SONG(
    bugcharge_charge_song,
    BN6_PCM(
        bugcharge_charge,
        0x80,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x70,0xD9,0x3C,0x7F,0x8A,0xB1\n",
        "build/bugcharge-charge-sample.bin"
    )
);
BN6_SONG(
    bugcharge_fire_song,
    ".byte 1,0,0x80,0\n"
    ".long bugcharge_fire_voicegroup\n"
    ".long bugcharge_fire_track\n"
    ".global bugcharge_fire_voicegroup\n"
    "bugcharge_fire_voicegroup:\n"
    ".byte 0x0C,0x3C,0,0\n"
    ".long 0\n"
    ".byte 0,3,0,0\n"
    ".global bugcharge_fire_track\n"
    "bugcharge_fire_track:\n"
    ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
    ".byte 0xBE,0x60,0xD2,0x3C,0x7F,0x83,0xEA,0x48\n"
    ".byte 0x9B,0x81,0xB1\n"
);

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t EFFECT_PHASE = 8;
static const uint8_t OUTRO_PHASE = 0x0C;
static const uint8_t FIRE_SUBSTATE = 4;
static const uint8_t COOLDOWN_SUBSTATE = 8;
static const uint8_t GOSPEL_VISUAL = 25;
static const uint16_t CHARGE_FRAMES = 39;
static const uint16_t SHOT_INTERVAL = 14;
static const uint16_t FINAL_COOLDOWN = 29;
static const Bn6CollisionType COLLISION_SELECTOR =
    BN6_COLLISION_TYPE_STANDARD_TARGET;
static const uint32_t PRESENT_COLLISION_VALUE = COLLISION_SELECTOR << 3;
static const uint32_t EXTENDED_COLLISION_VALUE =
    PRESENT_COLLISION_VALUE << 8;

static const uint8_t BYTE_PROPERTIES[] = {
    0x13, 0x14, 0x16, 0x19, 0x18, 0x1A, 0x63,
};

static uint16_t count_and_clear_bugs(Object *controller)
{
    uint8_t *properties = bn6_player_properties_for_side(controller->owner);
    uint16_t count = 1;

    if (properties[0x31] != 0) {
        ++count;
        properties[0x31] = 0;
    }
    for (size_t index = 0;
         index < sizeof(BYTE_PROPERTIES) / sizeof(BYTE_PROPERTIES[0]);
         ++index) {
        uint8_t offset = BYTE_PROPERTIES[index];
        if (properties[offset] != 0) {
            ++count;
            properties[offset] = 0;
        }
    }

    uint16_t *halfword = (uint16_t *)(properties + 0x54);
    if (*halfword != 0) {
        ++count;
        *halfword = 0;
    }
    if (properties[0x24] != 0) {
        ++count;
        properties[0x24] = 0;
    }
    bn6_bugfix_clear_runtime_state();
    return count;
}

static void head_set_position(Object *self)
{
    Object *player = self->parent;
    self->x = player->x;
    self->y = player->y;
    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    self->x += direction * (24 << 16);
    self->z = 0x17 << 16;
}

static void head_init(Object *self)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(bugcharge_gospel_sprite),
        BN6_SPRITE_ID(bugcharge_gospel_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    self->animation = 1;
    self->palette = 1;
    bn6_self_sprite_set_animation(1);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    head_set_position(self);
}

static bool head_update(Object *self)
{
    if (bn6_battle_is_over() != 0 || self->parent == NULL) {
        bn6_self_object_free();
        return false;
    }
    int32_t timer = (int32_t)self->aux_timer - 1;
    self->aux_timer = (uint16_t)timer;
    if (timer < 0) {
        bn6_self_object_free();
        return false;
    }
    head_set_position(self);
    return true;
}

static void spawn_charge_head(
    Object *controller,
    uint32_t spawn_argument
)
{
    Object *player = bn6_player_object_for_side(controller->owner);
    if (player == NULL) {
        return;
    }
    Object *head = bn6_spawn_type4(
        BN6_OBJECT_ID(bugcharge_head_main), spawn_argument
    );
    if (head == NULL) {
        return;
    }
    head->aux_timer = (uint16_t)(controller->timer * 15u + 55u);
    head->owner_word = controller->owner_word;
    head->parent = player;
}

static void spawn_gospel(Object *controller)
{
    int32_t direction =
        (int32_t)bn6_object_front_direction_for(controller);
    Object *gospel = bn6_spawn_type3(
        BN6_OBJECT_ID(bugcharge_gospel_main),
        0,
        0,
        0,
        GOSPEL_VISUAL
    );
    if (gospel == NULL) {
        return;
    }
    gospel->panel_x = (uint8_t)((int32_t)controller->panel_x + direction);
    gospel->panel_y = controller->panel_y;
    gospel->owner_word = controller->owner_word;
    gospel->attack = controller->attack;
    gospel->parent = controller;
    gospel->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING;
}

static void effect_update(Object *self, uint32_t spawn_argument)
{
    if (self->substate == 0) {
        self->timer = count_and_clear_bugs(self);
        spawn_charge_head(self, spawn_argument);
        bn6_play_sound(BN6_SONG_ID(bugcharge_charge_song));
        self->aux_timer = CHARGE_FRAMES;
        self->substate = FIRE_SUBSTATE;
        return;
    }

    int32_t delay = (int32_t)self->aux_timer - 1;
    self->aux_timer = (uint16_t)delay;
    if (delay >= 0) {
        return;
    }

    if (self->substate == FIRE_SUBSTATE) {
        bn6_play_sound(BN6_SONG_ID(bugcharge_fire_song));
        spawn_gospel(self);
        bn6_screen_shake_set(2, 20);
        if (--self->timer != 0) {
            self->aux_timer = SHOT_INTERVAL;
        } else {
            self->aux_timer = FINAL_COOLDOWN;
            self->substate = COOLDOWN_SUBSTATE;
        }
        return;
    }

    self->phase = OUTRO_PHASE;
    self->phase_timer = 0;
}

static void controller_update(Object *self, uint32_t spawn_argument)
{
    if (self->phase == 0) {
        bn6_self_type4_dimming_intro();
    } else if (self->phase == 4) {
        bn6_self_type4_dimming_freeze();
    } else if (self->phase == EFFECT_PHASE) {
        effect_update(self, spawn_argument);
    } else {
        bn6_self_type4_dimming_outro();
    }
}

static void free_collision(Object *self)
{
    Collision *collision = self->collision;
    if (collision != NULL) {
        bn6_collision_clear_region(collision);
        bn6_collision_free(self->collision);
    }
    bn6_self_object_free();
}

static bool hit_init(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        bn6_self_object_free();
        return false;
    }
    bn6_self_object_set_coords();
    self->z = 0x10 << 16;
    Collision *collision = bn6_self_collision_create();
    if (collision == NULL) {
        bn6_self_object_free();
        return false;
    }
    bn6_collision_setup(
        collision,
        BN6_COLLISION_TYPE_0A,
        COLLISION_SELECTOR,
        3
    );
    collision->region = 1;
    bn6_self_collision_set_hit_effect(BN6_HIT_EFFECT_FIRE);
    bn6_self_collision_present(0, PRESENT_COLLISION_VALUE);
    self->state_word = ACTIVE_STATE;
    return true;
}

static void hit_update(Object *self)
{
    Collision *collision = self->collision;
    bn6_collision_remove(collision);
    bn6_self_collision_spawn_effect();
    bn6_collision_clear_region(collision);
    bn6_collision_free(self->collision);
    bn6_self_object_free();
}

static void spawn_hit(Object *source, uint32_t panel_x, uint32_t panel_y)
{
    Object *hit = bn6_spawn_type3(
        BN6_OBJECT_ID(bugcharge_hit_main),
        0,
        0,
        0,
        GOSPEL_VISUAL
    );
    if (hit == NULL) {
        return;
    }
    hit->panel_x = (uint8_t)panel_x;
    hit->panel_y = (uint8_t)panel_y;
    hit->owner_word = source->owner_word;
    hit->attack = source->attack;
    hit->parent = source;
    hit->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING;
}

static USED void attack_row(Object *source)
{
    uint32_t opposing_navi_flag = source->owner == 0
        ? BN6_PANEL_FLAG_SIDE_1_NAVI
        : BN6_PANEL_FLAG_SIDE_0_NAVI;
    for (uint32_t panel_x = 6; panel_x != 0; --panel_x) {
        if (bn6_panel_matches_flags(
                panel_x,
                source->panel_y,
                opposing_navi_flag,
                0
            ) != 0) {
            spawn_hit(source, panel_x, source->panel_y);
        }
    }
}

static bool gospel_init(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        bn6_self_object_free();
        return false;
    }
    bn6_self_object_set_coords();
    self->z = 0x14 << 16;
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(bugcharge_gospel_sprite),
        BN6_SPRITE_ID(bugcharge_gospel_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    self->animation = 0;
    self->palette = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;

    Collision *collision = bn6_self_collision_create();
    if (collision == NULL) {
        bn6_self_object_free();
        return false;
    }
    bn6_collision_setup(
        collision,
        BN6_COLLISION_TYPE_STANDARD_ATTACK,
        COLLISION_SELECTOR,
        3
    );
    bn6_self_collision_set_hit_effect(BN6_HIT_EFFECT_SMALL_IMPACT);
    bn6_self_collision_set_extended_effect(0, PRESENT_COLLISION_VALUE);
    bn6_self_collision_present(0, EXTENDED_COLLISION_VALUE);
    self->animation_state = 8;
    self->state_word = ACTIVE_STATE;
    return true;
}

static bool gospel_update(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        free_collision(self);
        return false;
    }

    Collision *collision = self->collision;
    bn6_collision_remove(collision);
    bn6_self_collision_spawn_effect();
    if (collision->received_collision_flags != 0) {
        free_collision(self);
        return false;
    }

    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    self->x += direction * (10 << 16);
    bn6_self_object_update_panel();
    bn6_self_collision_update_panel();
    if (bn6_self_panel_is_valid_object() == 0) {
        free_collision(self);
        return false;
    }

    uint64_t coordinates = bn6_panel_to_coords(self->panel_x, self->panel_y);
    int32_t panel_x = (int32_t)(uint32_t)coordinates;
    uint32_t panel_y = (uint32_t)(coordinates >> 32);
    if (self->x == panel_x && --self->animation_state == 0) {
        free_collision(self);
        return false;
    }
    bn6_self_collision_present(0, panel_y);
    return true;
}

BN6_OBJECT3(bugcharge_hit_main)
{
    if (self->state == 0) {
        if (!hit_init(self)) {
            return;
        }
    } else if (self->state == ACTIVE_STATE) {
        hit_update(self);
        return;
    } else {
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_dimming();
}

BN6_OBJECT3(bugcharge_gospel_main)
{
    if (self->state == 0) {
        if (!gospel_init(self) || !gospel_update(self)) {
            return;
        }
    } else if (self->state == ACTIVE_STATE) {
        if (!gospel_update(self)) {
            return;
        }
    } else {
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_dimming();
}

BN6_OBJECT4(bugcharge_head_main)
{
    if (self->state == 0) {
        head_init(self);
    } else if (self->state == ACTIVE_STATE) {
        if (!head_update(self)) {
            return;
        }
    } else {
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_dimming();
}

BN6_OBJECT4(bugcharge_controller_main)
{
    if (self->state == 0) {
        bn6_self_type4_dimming_init();
    } else if (self->state == ACTIVE_STATE) {
        controller_update(self, spawn_argument);
    } else {
        bn6_self_type4_dimming_free();
    }
}

BN6_PERSISTENT_ATTACK(0x131, bugcharge_attack_main)
{
    Object *controller = bn6_spawn_type4(
        BN6_OBJECT_ID(bugcharge_controller_main), spawn_argument
    );
    if (controller == NULL) {
        return NULL;
    }
    controller->panel_x = (uint8_t)panel_x;
    controller->panel_y = (uint8_t)panel_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
    return controller;
}
