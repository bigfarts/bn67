#include "runtime.h"

BN6_SPRITE(signalred_battle_sprite, "build/signalred-battle-sprite.bin");
BN6_ASM_RESOURCE(
    signalred_dust_sprite_table,
    ".short 0x0904,0x240C,0x230C,0x300C,0x0010\n"
    ".short 0x0810,0x1804,0x0A04,0x340C,0x350C\n"
    ".short 0x0D04,0x0010,0x410C,0x0504,0x410C\n"
    ".byte __bn6_sprite_group_signalred_battle_sprite\n"
    ".byte __bn6_sprite_id_signalred_battle_sprite\n"
);
BN6_SONG(
    signalred_spawn_song,
    BN6_PCM(
        signalred_spawn,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xDC,0x3C,0x7F,0x8D,0xB1,0x00\n",
        "build/signalred-spawn-sample.bin"
    )
);
BN6_POINTER_PATCH(0x08012010, signalred_dust_sprite_table);
BN6_POINTER_PATCH(0x08012014, signalred_dust_sprite_table);
#if FALZAR
BN6_POINTER_PATCH(0x080E9990, signalred_dust_sprite_table);
#else
BN6_POINTER_PATCH(0x080EACD0, signalred_dust_sprite_table);
#endif
BN6_INCBIN(signalred_icon, "build/signalred-icon.bin");
BN6_INCBIN(signalred_image, "build/signalred-image.bin");
BN6_INCBIN(signalred_palette, "build/signalred-palette.bin");

static const uint32_t GREEN_SFX = 0x00D1;
static const uint16_t STARTUP_TICKS = 3;
static const uint16_t RED_TICKS = 420;
static const uint16_t GREEN_TICKS = 50;
static const uint16_t OBJECT_HP = 100;
static const Bn6CollisionType PASSIVE_COLLISION_TYPE =
    BN6_COLLISION_TYPE_13;
static const Bn6CollisionType TARGET_COLLISION_TYPE =
    BN6_COLLISION_TYPE_14;
static const uint32_t DUST_AMMO_KIND = 15;
static const uint8_t STARTUP_PENDING_FLAG = 0x01;
static const uint8_t COLLISION_DEFERRED_FLAG = 0x80;
static const uint8_t INITIAL_REMOVAL_FLAGS =
    STARTUP_PENDING_FLAG | COLLISION_DEFERRED_FLAG;
static const uint32_t DESTROY_EFFECT = 0x00;
static const int32_t DESTROY_EFFECT_HEIGHT = 0x00100000;
static const uint32_t DESTROY_SFX = 0x70;

static uint32_t opponent_chip_enable_flag(const Object *object)
{
    return object->owner == 0
        ? BN6_BATTLE_CONTROL_FLAG_SIDE_1_CHIPS_ENABLED
        : BN6_BATTLE_CONTROL_FLAG_SIDE_0_CHIPS_ENABLED;
}

static void enable_opponent_chips(Object *object)
{
    bn6_battle_set_control_flags(opponent_chip_enable_flag(object));
}

static void disable_opponent_chips(Object *object)
{
    bn6_battle_clear_control_flags(opponent_chip_enable_flag(object));
}

static void object_animate(Object *object)
{
    (void)object;
    bn6_self_collision_present(0, PASSIVE_COLLISION_TYPE);
    bn6_self_object_update();
}

static void object_begin_destroy(Object *object)
{
    object->header_flags &= (uint8_t)~BN6_OBJECT_FLAG_VISIBLE;
    object->state_word = 8;
}

static void object_begin_damage_destroy(Object *object)
{
    (void)bn6_spawn_battle_effect(
        0,
        object->x,
        object->y,
        object->z + DESTROY_EFFECT_HEIGHT,
        DESTROY_EFFECT
    );
    bn6_play_sound(DESTROY_SFX);
    object_begin_destroy(object);
}

static void object_destroy(Object *object)
{
    enable_opponent_chips(object);
    bn6_self_object_unregister_deployable();
    Collision *collision = object->collision;
    if (collision != NULL) {
        bn6_collision_remove(collision);
        bn6_collision_clear_region(collision);
        collision = object->collision;
        bn6_collision_free(collision);
    }
    bn6_self_object_free();
}

static void play_spawn_sound(Object *object)
{
    bn6_play_sound(BN6_SONG_ID(signalred_spawn_song));
    object->aux_timer = RED_TICKS;
    disable_opponent_chips(object);
}

static void play_green_sound(Object *object)
{
    (void)object;
    bn6_play_sound(GREEN_SFX);
}

static void object_turn_green(Object *object)
{
    object->animation_state = 4;
    object->animation = 1;
    bn6_self_sprite_set_animation(1);
    bn6_self_sprite_load_animation_data();
    object->aux_timer = GREEN_TICKS;
    enable_opponent_chips(object);
    play_green_sound(object);
}

static void object_cycle_update(Object *object)
{
    uint16_t timer = (uint16_t)(object->aux_timer - 1u);
    object->aux_timer = timer;
    if ((int16_t)timer >= 0) {
        object_animate(object);
        return;
    }
    if (object->animation_state == 0) {
        object_turn_green(object);
        object_animate(object);
        return;
    }

    object->animation_state = 0;
    object->animation = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    object->aux_timer = RED_TICKS;
    disable_opponent_chips(object);
    object_animate(object);
}

static void object_normal_update(Object *object)
{
    if ((object->removal_state & STARTUP_PENDING_FLAG) == 0) {
        object_cycle_update(object);
        return;
    }

    uint16_t timer = (uint16_t)(object->aux_timer - 1u);
    object->aux_timer = timer;
    if (timer != 0) {
        object_animate(object);
        return;
    }
    object->removal_state &= (uint8_t)~STARTUP_PENDING_FLAG;
    play_spawn_sound(object);
    object_animate(object);
}

static void object_store_dust_ammo(Object *object)
{
    bn6_self_object_store_dust_ammo(DUST_AMMO_KIND);
    object_destroy(object);
}

static void object_update(Object *object)
{
    uint8_t removal = object->removal_state;
    if ((removal & COLLISION_DEFERRED_FLAG) != 0) {
        if (bn6_battle_is_time_stopped() != 0) {
            if ((removal & STARTUP_PENDING_FLAG) != 0) {
                object_normal_update(object);
            } else {
                object_animate(object);
            }
            return;
        }
        Collision *collision = object->collision;
        bn6_collision_setup(
            collision,
            PASSIVE_COLLISION_TYPE,
            TARGET_COLLISION_TYPE,
            3
        );
        bn6_self_collision_present(0, PASSIVE_COLLISION_TYPE);
        object->removal_state = (uint8_t)(
            removal & (uint8_t)~COLLISION_DEFERRED_FLAG
        );
    }

    bn6_self_field_collision_update();
    bn6_self_deployable_lifetime_update();
    Collision *collision = object->collision;
    uint32_t damage = collision->final_damage;
    if (damage != 0) {
        bn6_self_sprite_flash_white();
        bn6_self_object_apply_damage(damage);
    }
    if (bn6_battle_is_over() != 0) {
        object_begin_destroy(object);
        return;
    }
    if (object->hp == 0) {
        object_begin_damage_destroy(object);
        return;
    }
    if (bn6_battle_is_time_stopped() != 0) {
        object_animate(object);
        return;
    }

    uint32_t secondary_flags = bn6_self_collision_get_secondary_flags();
    if ((secondary_flags & (BN6_COLLISION_SECONDARY_FLAG_DUST_SUCTION_SIDE_0 | BN6_COLLISION_SECONDARY_FLAG_DUST_SUCTION_SIDE_1)) != 0) {
        object_store_dust_ammo(object);
        return;
    }
    if ((secondary_flags & BN6_COLLISION_SECONDARY_FLAG_TIMED_BLINK_REMOVAL) != 0) {
        if (bn6_self_object_update_timed_removal() == 1) {
            object_begin_destroy(object);
        }
        return;
    }
    object_normal_update(object);
}

static void object_init(Object *object)
{
    if (bn6_self_panel_is_valid_object() == 0) {
        object_destroy(object);
        return;
    }

    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(signalred_battle_sprite),
        BN6_SPRITE_ID(signalred_battle_sprite)
    );
    bn6_self_sprite_property_2e3c();
    object->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    bn6_self_sprite_set_flip(object->owner);
    bn6_self_object_set_coords();
    object->header_flags |= BN6_OBJECT_FLAG_VISIBLE;

    object->hp = OBJECT_HP;
    object->max_hp = OBJECT_HP;
    object->aux_timer = STARTUP_TICKS;
    object->timer = 0x0960;
    if (bn6_self_collision_create() == NULL) {
        object_destroy(object);
        return;
    }

    object->animation_state = 0;
    object->substate = 0;
    object->removal_state = INITIAL_REMOVAL_FLAGS;
    object->state_word = 4;
    bn6_self_object_update();
}

static Object *spawn_persistent(Object *controller)
{
    Object *owner = controller->parent;
    uint32_t panel_x = owner->panel_x;
    uint32_t panel_y = owner->panel_y;
    panel_x += bn6_object_front_direction_for(owner);
    if (bn6_panel_is_valid_xy(panel_x, panel_y) == 0) {
        return NULL;
    }

    uint64_t coordinates = bn6_panel_to_coords(panel_x, panel_y);
    Object *object = bn6_spawn_type3(
        BN6_OBJECT_ID(signalred_object_main),
        (int32_t)(uint32_t)coordinates,
        (int32_t)(uint32_t)(coordinates >> 32),
        0,
        0
    );
    if (object == NULL) {
        return NULL;
    }

    object->panel_x = (uint8_t)panel_x;
    object->panel_y = (uint8_t)panel_y;
    uint8_t owner_side = owner->owner;
    object->owner = owner_side;
    object->parent = owner;
    object->attack = controller->attack;
    object->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_TIME_STOP;
    bn6_object_register_deployable(object, owner_side, 1);
    return object;
}

static void launch_effect(Object *controller)
{
    if (controller->substate == 0) {
        (void)spawn_persistent(controller);
        controller->timer = 30;
        controller->substate = 4;
    }

    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if (timer == 0) {
        controller->phase = 0x0C;
        controller->phase_timer = 0;
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
        launch_effect(controller);
        break;
    default:
        bn6_self_type4_timestop_outro();
        break;
    }
}

BN6_OBJECT4(signalred_controller_main)
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

BN6_OBJECT3(signalred_object_main)
{
    switch (self->state) {
    case 0:
        object_init(self);
        break;
    case 4:
        object_update(self);
        break;
    default:
        object_destroy(self);
        break;
    }
}

BN6_ATTACK(0x0C1, signalred_attack_main)
{
    Object *controller = bn6_spawn_type4(
        BN6_OBJECT_ID(signalred_controller_main), spawn_argument
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
