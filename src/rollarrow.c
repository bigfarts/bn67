#include "abi.h"
#include "runtime.h"

BN6_INCLUDE(common);
BN6_USE_SONG(common_navi_summon_song);
BN6_SPRITE(rollarrow_actor_sprite, "build/rollarrow-actor-sprite.bin");
BN6_SPRITE(rollarrow_projectile_sprite, "build/rollarrow-projectile-sprite.bin");

BN6_INCBIN(rollarrow_icon_1, "build/rollarrow-icon-1.bin");
BN6_INCBIN(rollarrow_icon_2, "build/rollarrow-icon-2.bin");
BN6_INCBIN(rollarrow_icon_3, "build/rollarrow-icon-3.bin");
BN6_INCBIN(rollarrow_image, "build/rollarrow-image.bin");
BN6_INCBIN(rollarrow_palette_1, "build/rollarrow-pal-1.bin");
BN6_INCBIN(rollarrow_palette_2, "build/rollarrow-pal-2.bin");
BN6_INCBIN(rollarrow_palette_3, "build/rollarrow-pal-3.bin");
BN6_SONG(
    rollarrow_fire_song,
    BN6_PCM(
        rollarrow_fire,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xE1,0x3C,0x7F,0x92,0xB1\n",
        "build/rollarrow-fire-sample.bin"
    )
);

static const uint16_t WAIT_FRAMES = 20;
static const uint16_t FIRE_HOLD_FRAMES = 6;
static const uint16_t POST_SHOT_FRAMES = 30;
static const uint16_t EXIT_FRAMES = 5;
static const int32_t PROJECTILE_SPEED = 0x00070000;
static const uint8_t PROJECTILE_PANELS = 8;
static const uint32_t PROJECTILE_COLLISION_REGION = 8;

static bool timer_expired(Object *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer < 0;
}

static void set_animation(Object *self, uint32_t animation)
{
    self->animation = (uint8_t)animation;
    self->palette = UINT8_MAX;
    bn6_self_sprite_set_animation(animation);
    bn6_self_sprite_load_animation_data();
}

static void spawn_projectile(Object *actor)
{
    Object *projectile = bn6_spawn_type3(
        BN6_OBJECT_ID(rollarrow_arrow_main),
        actor->panel_y,
        actor->parameter,
        0,
        PROJECTILE_COLLISION_REGION
    );
    if (projectile == NULL) {
        return;
    }

    projectile->panel_x = actor->panel_x;
    projectile->panel_y = actor->panel_y;
    projectile->parameter = actor->parameter;
    projectile->owner_word = actor->owner_word;
    projectile->attack = actor->attack;
    projectile->parent = actor;

    int32_t direction = (int32_t)bn6_object_front_direction_for(actor);
    projectile->x = actor->x + direction * (8 << 16);
    projectile->y = ((actor->y >> 16) - 1) << 16;
    projectile->z = 0x24 << 16;
    projectile->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_TIME_STOP;
}

static void actor_wait(Object *self)
{
    if (self->substate == 0) {
        set_animation(self, 0);
        self->timer = WAIT_FRAMES;
        self->substate = 4;
    }
    if (timer_expired(self)) {
        self->phase = 4;
        self->substate = 0;
    }
}

static void actor_fire(Object *self)
{
    if (self->substate == 0) {
        set_animation(self, 7);
        bn6_play_sound(BN6_SONG_ID(rollarrow_fire_song));
        spawn_projectile(self);
        self->timer = FIRE_HOLD_FRAMES;
        self->substate = 4;
    }
    if (timer_expired(self)) {
        self->phase = 8;
        self->substate = 0;
    }
}

static void actor_exit(Object *self)
{
    if (self->substate == 0) {
        set_animation(self, 0);
        self->timer = POST_SHOT_FRAMES;
        self->substate = 4;
        return;
    }
    if (self->substate == 4) {
        if (timer_expired(self)) {
            set_animation(self, 4);
            self->timer = EXIT_FRAMES;
            self->substate = 8;
        }
        return;
    }
    if (timer_expired(self)) {
        self->header_flags &= (uint8_t)~BN6_OBJECT_FLAG_VISIBLE;
        self->state_word = 8;
    }
}

static void actor_update(Object *self)
{
    switch (self->phase) {
    case 0:
        actor_wait(self);
        break;
    case 4:
        actor_fire(self);
        break;
    default:
        actor_exit(self);
        break;
    }
}

static void actor_init(Object *self)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(rollarrow_actor_sprite),
        BN6_SPRITE_ID(rollarrow_actor_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_property_2e3c();
    self->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_object_set_coords();
    self->z = 0;
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    bn6_self_sprite_set_palette(0);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    self->state_word = 4;
    self->substate = 0;
    bn6_play_sound(BN6_SONG_ID(common_navi_summon_song));
}

static void actor_destroy(Object *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    bn6_self_object_free();
}

BN6_OBJECT1(rollarrow_actor_main)
{
    switch (self->state) {
    case 0:
        actor_init(self);
        break;
    case 4:
        actor_update(self);
        break;
    default:
        actor_destroy(self);
        return;
    }
    bn6_self_object_update_timestop();
}

static void projectile_free(Object *self)
{
    Collision *collision = self->collision;
    bn6_collision_clear_region(collision);
    bn6_collision_free(collision);
    bn6_self_object_free();
}

static bool projectile_init(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        bn6_self_object_free();
        return false;
    }

    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(rollarrow_projectile_sprite),
        BN6_SPRITE_ID(rollarrow_projectile_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    self->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;

    Collision *collision = bn6_self_collision_create();
    if (collision == NULL) {
        bn6_self_object_free();
        return false;
    }
    bn6_collision_setup(collision, PROJECTILE_COLLISION_REGION, 5, 3);
    bn6_self_collision_set_hit_effect(BN6_HIT_EFFECT_CHIP_DELETE);
    bn6_self_collision_present(0, PROJECTILE_COLLISION_REGION);
    self->animation_state = PROJECTILE_PANELS;
    self->target_panel_x = (uint8_t)(
        self->panel_x + (int32_t)bn6_object_front_direction_for(self)
    );
    self->state_word = 4;
    return true;
}

static bool projectile_update(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        projectile_free(self);
        return false;
    }

    Collision *collision = self->collision;
    bn6_collision_remove(collision);
    bn6_self_collision_spawn_effect();
    if (collision->received_collision_flags != 0) {
        projectile_free(self);
        return false;
    }

    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    uint32_t panel_x = self->target_panel_x;
    uint32_t panel_y = self->panel_y;
    if (bn6_panel_is_valid_xy(panel_x, panel_y) == 0) {
        projectile_free(self);
        return false;
    }

    int32_t target_x = (int32_t)(uint32_t)bn6_panel_to_coords(panel_x, panel_y);
    int32_t next_x = self->x + direction * PROJECTILE_SPEED;
    self->x = next_x;
    bool entered = direction < 0 ? next_x <= target_x : next_x >= target_x;
    if (entered) {
        self->x = target_x;
        self->panel_x = self->target_panel_x;
        bn6_self_object_update_panel();
        bn6_self_collision_update_panel();
        if (self->animation_state <= 1) {
            self->animation_state = 0;
            projectile_free(self);
            return false;
        }
        --self->animation_state;
        self->target_panel_x = (uint8_t)(self->target_panel_x + direction);
    }
    bn6_self_collision_present(0, PROJECTILE_COLLISION_REGION);
    return true;
}

BN6_OBJECT3(rollarrow_arrow_main)
{
    switch (self->state) {
    case 0:
        if (!projectile_init(self)) {
            return;
        }
        break;
    case 4:
        if (!projectile_update(self)) {
            return;
        }
        break;
    default:
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_timestop();
}

BN6_SUMMON_ATTACK(0x018, rollarrow_attack_main)
{
    Object *actor = bn6_spawn_type1(
        BN6_OBJECT_ID(rollarrow_actor_main), spawn_argument
    );
    if (actor == NULL) {
        return;
    }
    actor->panel_x = (uint8_t)panel_x;
    actor->panel_y = (uint8_t)panel_y;
    actor->parameter = (uint8_t)parameter;
    actor->owner_word = owner->owner_word;
    actor->parent = owner;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
