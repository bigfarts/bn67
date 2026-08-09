#include "runtime.h"

BN6_INCLUDE(common);
BN6_USE_SONG(common_navi_summon_song);
BN6_SPRITE(laserman_battle_sprite, "build/laserman-battle-sprite.bin");

BN6_INCBIN(laserman_icon, "build/laserman-icon.bin");
BN6_INCBIN(laserman_image, "build/laserman-image.bin");
BN6_INCBIN(laserman_palette_base, "build/laserman-pal-base.bin");
BN6_ASM_RESOURCE(
    laserman_palette_ex,
    ".incbin \"build/laserman-pal-base.bin\",0,0x02\n"
    ".short 0x00C0,0x0180,0x0280,0x03E0,0x0060\n"
    ".incbin \"build/laserman-pal-base.bin\",0x0C,0x14\n"
);
BN6_INCBIN(laserman_palette_sp, "build/laserman-pal-sp.bin");
BN6_SONG(
    laserman_fire_song,
    BN6_PCM(
        laserman_fire,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xF6,0x2F,0x7F,0xA2,0x81,0xBE\n"
        ".byte 0x60,0x84,0x40,0x84,0x20,0x84,0x10,0x84,0xB1\n",
        "build/laserman-fire-sample.bin"
    )
);

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t DESTROY_STATE = 8;
static const uint8_t HIT_VISUAL = 25;
static const uint16_t WAIT_FRAMES = 20;
static const uint16_t RAISE_FRAMES = 30;
static const uint16_t LASER_FRAMES = 80;
static const uint16_t BEAM_FRAMES = 60;
static const Bn6CollisionType COLLISION_SELECTOR =
    BN6_COLLISION_TYPE_STANDARD_TARGET;

struct LasermanHitWork {
    uint32_t reserved[5];
    uint32_t command_stream;             // +0x74
};

_Static_assert(
    offsetof(struct LasermanHitWork, command_stream) == 0x14,
    "LaserMan hit work layout"
);

enum CommandEffect {
    COMMAND_EFFECT_DISABLE_SUPER_ARMOR = 1,
    COMMAND_EFFECT_DISABLE_FLOAT_SHOES = 2,
    COMMAND_EFFECT_DISABLE_AIR_SHOES = 3,
    COMMAND_EFFECT_DISABLE_UNDERSHIRT = 4,
    COMMAND_EFFECT_RESET_ATTACK = 5,
    COMMAND_EFFECT_RESET_RAPID = 6,
    COMMAND_EFFECT_RESET_CHARGE = 7,
    COMMAND_EFFECT_RESTORE_CHARGE_SHOT = 0x0A,
    COMMAND_EFFECT_DISABLE_B_LEFT = 0x0C,
    COMMAND_MARKER = 0xFD,
    COMMAND_EFFECT_REDUCE_CUSTOM = 0xFE,
    COMMAND_END = 0xFF,
};

static const uint16_t COMMAND_NONE[] = {COMMAND_MARKER, COMMAND_END};
static const uint16_t COMMAND_UP[] = {
    COMMAND_EFFECT_RESET_ATTACK,
    COMMAND_EFFECT_RESET_RAPID,
    COMMAND_EFFECT_RESET_CHARGE,
    COMMAND_MARKER,
    COMMAND_END,
};
static const uint16_t COMMAND_DOWN[] = {
    COMMAND_EFFECT_DISABLE_SUPER_ARMOR,
    COMMAND_EFFECT_DISABLE_FLOAT_SHOES,
    COMMAND_EFFECT_DISABLE_AIR_SHOES,
    COMMAND_EFFECT_DISABLE_UNDERSHIRT,
    COMMAND_EFFECT_DISABLE_B_LEFT,
    COMMAND_MARKER,
    COMMAND_END,
};
static const uint16_t COMMAND_RIGHT[] = {
    COMMAND_EFFECT_RESTORE_CHARGE_SHOT,
    COMMAND_MARKER,
    COMMAND_END,
};
static const uint16_t COMMAND_LEFT[] = {
    COMMAND_EFFECT_REDUCE_CUSTOM,
    COMMAND_MARKER,
    COMMAND_END,
};

static const uint16_t *const COMMAND_STREAMS[] = {
    COMMAND_NONE,
    COMMAND_UP,
    COMMAND_DOWN,
    COMMAND_RIGHT,
    COMMAND_LEFT,
};

static const uint32_t BEAM_SCALES[] = {
    0x00000000,
    0x0000B060,
    0x0000A80A,
    0x00000000,
    0x0000B9C0,
};

static bool timer_expired(Object *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer < 0;
}

static void set_animation(Object *self, uint8_t animation)
{
    self->animation = animation;
    self->palette = UINT8_MAX;
    bn6_self_sprite_set_animation(animation);
    bn6_self_sprite_load_animation_data();
}

static void read_command(Object *actor)
{
    const uint8_t *input = bn6_input_state_for_side(actor->owner);
    uint16_t keys = *(const uint16_t *)(input + 2);
    if ((keys & BN6_KEY_UP) != 0) {
        actor->subvariant = 1;
    } else if ((keys & BN6_KEY_DOWN) != 0) {
        actor->subvariant = 2;
    } else if ((keys & BN6_KEY_RIGHT) != 0) {
        actor->subvariant = 3;
    } else if ((keys & BN6_KEY_LEFT) != 0) {
        actor->subvariant = 4;
    }
}

static void actor_destroy(Object *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    bn6_self_object_free();
}

static void spawn_laser(Object *actor)
{
    Object *beam = bn6_spawn_type1(
        BN6_OBJECT_ID(laserman_beam_main), actor->subvariant
    );
    if (beam == NULL) {
        return;
    }
    beam->panel_x = (uint8_t)(
        (int32_t)actor->panel_x + bn6_self_object_side_direction()
    );
    beam->panel_y = actor->panel_y;
    beam->parameter = actor->parameter;
    beam->owner_word = actor->owner_word;
    beam->attack = actor->attack;
    beam->parent = actor;
    beam->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_TIME_STOP;
}

static void actor_attack(Object *self)
{
    if (self->substate == 0) {
        set_animation(self, 2);
        self->timer = RAISE_FRAMES;
        self->substate = 4;
        return;
    }
    if (self->substate == 4) {
        read_command(self);
        if (timer_expired(self)) {
            set_animation(self, 3);
            spawn_laser(self);
            bn6_play_sound(BN6_SONG_ID(laserman_fire_song));
            self->timer = LASER_FRAMES;
            self->substate = 8;
        }
        return;
    }
    if (self->substate == 8) {
        if (timer_expired(self)) {
            set_animation(self, 4);
            self->substate = 12;
        }
        return;
    }
    if ((bn6_self_sprite_get_frame_flags()
            & BN6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = DESTROY_STATE;
    }
}

static void actor_update(Object *self)
{
    if (self->phase == 0) {
        if (!timer_expired(self)) {
            return;
        }
        if (bn6_panel_matches_flags(
                self->panel_x,
                self->panel_y,
                BN6_PANEL_FLAG_SOLID,
                0
            ) == 0) {
            self->state_word = DESTROY_STATE;
            return;
        }
        self->phase = 4;
        self->substate = 0;
        return;
    }
    actor_attack(self);
}

static void actor_init(Object *self)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(laserman_battle_sprite),
        BN6_SPRITE_ID(laserman_battle_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_property_2e3c();
    self->animation = 0;
    self->palette = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_object_set_coords();
    self->z = 0;
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    bn6_self_sprite_set_palette(0);
    bn6_self_sprite_set_priority(1);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    self->phase = 0;
    self->substate = 0;
    self->timer = WAIT_FRAMES;
    bn6_play_sound(BN6_SONG_ID(common_navi_summon_song));
}

static void spawn_hit(
    Object *beam,
    uint32_t panel_x,
    uint32_t panel_y,
    uint16_t command
)
{
    Object *hit = bn6_spawn_type3(
        BN6_OBJECT_ID(laserman_hit_main), 0, 0, 0, HIT_VISUAL
    );
    if (hit == NULL) {
        return;
    }
    struct LasermanHitWork *work =
        (struct LasermanHitWork *)hit->work;
    hit->panel_x = (uint8_t)panel_x;
    hit->panel_y = (uint8_t)panel_y;
    hit->animation_word = command;
    work->command_stream = beam->variant;
    hit->owner_word = beam->owner_word;
    hit->attack = command == COMMAND_MARKER ? beam->attack : 0;
    if (command != COMMAND_MARKER) {
        hit->phase_timer_low = 0;
    }
    hit->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_TIME_STOP;
}

static void spawn_row_event(Object *beam, uint16_t command)
{
    for (uint32_t panel_x = 1; panel_x <= 6; ++panel_x) {
        spawn_hit(beam, panel_x, beam->panel_y, command);
    }
}

static void beam_command_tick(Object *self)
{
    if (self->removal_state != 0) {
        --self->removal_state;
        return;
    }
    const uint16_t *stream = COMMAND_STREAMS[self->variant];
    uint16_t command = stream[self->animation_state];
    if ((uint8_t)command == COMMAND_END) {
        return;
    }
    ++self->animation_state;
    self->removal_state = 5;
    if ((uint8_t)command == COMMAND_MARKER) {
        spawn_row_event(self, command);
    }
}

static void beam_update(Object *self)
{
    if (self->phase == 0) {
        if ((bn6_self_sprite_get_frame_flags()
                & BN6_ANIMATION_FRAME_FLAG_END) != 0) {
            set_animation(self, 18);
            self->animation_state = 0;
            self->removal_state = 0;
            self->timer = BEAM_FRAMES;
            self->phase = 4;
        }
        return;
    }
    if (self->phase == 4) {
        beam_command_tick(self);
        if (timer_expired(self)) {
            set_animation(self, 19);
            self->phase = 8;
        }
        return;
    }
    if ((bn6_self_sprite_get_frame_flags()
            & BN6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = DESTROY_STATE;
    }
}

static void beam_init(Object *self)
{
    bn6_self_object_set_coords();
    self->z = 0;
    self->x += bn6_self_object_side_direction() * (0x40 << 16);
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(laserman_battle_sprite),
        BN6_SPRITE_ID(laserman_battle_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    set_animation(self, 17);
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    bn6_self_sprite_set_palette(
        self->variant == 0 || self->variant == 2 ? 0 : 10
    );
    bn6_self_sprite_set_priority(1);
    bn6_self_sprite_set_scale(BEAM_SCALES[self->variant]);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    self->phase = 0;
}

static void apply_command_effect(Object *hit, uint16_t command)
{
    uint32_t target_side = hit->owner ^ 1u;
    enum CommandEffect effect = (enum CommandEffect)(uint8_t)command;
    uint32_t property; // ID of the player property affected by the command.
    uint32_t value; // Raw value written to that player property.

    switch (effect) {
    case COMMAND_EFFECT_DISABLE_SUPER_ARMOR:
        property = 0x23;
        value = 0;
        break;
    case COMMAND_EFFECT_DISABLE_FLOAT_SHOES:
        property = 0x1B;
        value = 0;
        break;
    case COMMAND_EFFECT_DISABLE_AIR_SHOES:
        property = 0x1C;
        value = 0;
        break;
    case COMMAND_EFFECT_DISABLE_UNDERSHIRT:
        property = 0x1D;
        value = 0;
        break;
    case COMMAND_EFFECT_RESET_ATTACK:
        property = 1;
        value = 0;
        break;
    case COMMAND_EFFECT_RESET_RAPID:
        property = 2;
        value = 0;
        break;
    case COMMAND_EFFECT_RESET_CHARGE:
        property = 3;
        value = 0;
        break;
    case COMMAND_EFFECT_DISABLE_B_LEFT:
        property = 7;
        value = UINT8_MAX;
        break;
    case COMMAND_EFFECT_RESTORE_CHARGE_SHOT: {
        uint32_t current = bn6_player_property_for_side(target_side, 5);
        Object *player = bn6_player_object_for_side(target_side);
        if (player != NULL) {
            PlayerRuntime *runtime = player->runtime_data;
            if (runtime->active_power_attack == current) {
                hit->phase_timer_low = 1;
            }
        }
        bn6_player_property_set_for_side(target_side, 4, 0);
        property = 5;
        value = 1;
        break;
    }
    case COMMAND_EFFECT_REDUCE_CUSTOM:
        value = bn6_player_property_for_side(target_side, 0x0A);
        if (value > 2) {
            --value;
        }
        property = 0x0A;
        break;
    default:
        return;
    }
    bn6_player_property_set_for_side(target_side, property, value);
}

static void refresh_target_player(Object *hit, uint16_t command)
{
    uint32_t target_side = hit->owner ^ 1u;
    Object *player = bn6_player_object_for_side(target_side);
    if (player == NULL) {
        return;
    }
    PlayerRuntime *runtime = player->runtime_data;
    enum CommandEffect effect = (enum CommandEffect)(uint8_t)command;
    if (effect == COMMAND_EFFECT_RESTORE_CHARGE_SHOT) {
        if (hit->phase_timer_low != 0) {
            runtime->active_power_attack =
                (uint8_t)bn6_player_property_for_side(target_side, 5);
        }
    } else if (effect == COMMAND_EFFECT_DISABLE_B_LEFT) {
        runtime->b_left =
            (uint8_t)bn6_player_property_for_side(target_side, 7);
    }

    if (command < COMMAND_EFFECT_DISABLE_SUPER_ARMOR
        || command > COMMAND_EFFECT_DISABLE_UNDERSHIRT) {
        return;
    }
    static const uint8_t PROPERTIES[] = {0x1B, 0x1C, 0x1D, 0x23};
    static const uint32_t STATUS_FLAGS[] = {
        BN6_COLLISION_STATUS_FLAG_FLOAT_SHOES,
        BN6_COLLISION_STATUS_FLAG_AIR_SHOES,
        BN6_COLLISION_STATUS_FLAG_UNDERSHIRT,
        BN6_COLLISION_STATUS_FLAG_SUPER_ARMOR,
    };
    for (size_t index = 0; index < 4; ++index) {
        if (bn6_player_property_for_side(target_side, PROPERTIES[index]) == 0) {
            bn6_player_clear_collision_status_flags(
                player,
                STATUS_FLAGS[index]
            );
        }
    }
}

static void apply_selected_command(Object *hit)
{
    struct LasermanHitWork *work =
        (struct LasermanHitWork *)hit->work;
    const uint16_t *stream = COMMAND_STREAMS[work->command_stream];
    for (size_t index = 0;; ++index) {
        uint16_t command = stream[index];
        uint8_t effect = (uint8_t)command;
        if (effect == COMMAND_END || effect == COMMAND_MARKER) {
            return;
        }
        hit->animation_word = command;
        apply_command_effect(hit, command);
        refresh_target_player(hit, command);
    }
}

static bool hit_init(Object *self)
{
    if (bn6_battle_is_over() != 0 || bn6_self_panel_is_valid_object() == 0) {
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
        (Bn6CollisionType)self->variant,
        COLLISION_SELECTOR,
        3
    );
    bn6_self_collision_set_hit_effect(BN6_HIT_EFFECT_NORMAL);
    bn6_self_collision_present(0, self->variant);
    self->state_word = ACTIVE_STATE;
    return true;
}

static void hit_update(Object *self)
{
    Collision *collision = self->collision;
    bn6_collision_remove(collision);
    bn6_self_collision_spawn_effect();
    if (collision->received_collision_flags != 0) {
        Object *target = bn6_player_object_for_side(self->owner ^ 1u);
        if (target != NULL
            && target->panel_x == self->panel_x
            && target->panel_y == self->panel_y
            && (uint8_t)self->animation_word == COMMAND_MARKER) {
            apply_selected_command(self);
        }
    }
    bn6_collision_clear_region(collision);
    bn6_collision_free(self->collision);
    bn6_self_object_free();
}

BN6_OBJECT3(laserman_hit_main)
{
    if (self->state == 0) {
        (void)hit_init(self);
    } else if (self->state == ACTIVE_STATE) {
        hit_update(self);
    } else {
        bn6_self_object_free();
    }
}

BN6_OBJECT1(laserman_beam_main)
{
    if (self->state == 0) {
        beam_init(self);
    } else if (self->state == ACTIVE_STATE) {
        beam_update(self);
    } else {
        bn6_self_object_free();
        return;
    }
    bn6_self_sprite_update();
}

BN6_OBJECT1(laserman_actor_main)
{
    if (self->state == 0) {
        actor_init(self);
    } else if (self->state == ACTIVE_STATE) {
        actor_update(self);
    } else {
        actor_destroy(self);
        return;
    }
    bn6_self_sprite_update();
}

BN6_SUMMON_ATTACK(0x0E3, laserman_attack_main)
{
    Object *actor = bn6_spawn_type1(
        BN6_OBJECT_ID(laserman_actor_main), spawn_argument
    );
    if (actor == NULL) {
        return;
    }
    actor->panel_x = (uint8_t)panel_x;
    actor->panel_y = (uint8_t)panel_y;
    actor->parameter = (uint8_t)parameter;
    actor->owner_word = owner->owner_word;
    actor->subvariant = 0;
    read_command(actor);
    actor->parent = owner;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
