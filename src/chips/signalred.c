#include "runtime.h"

BN67_SPRITE(signalred_battle_sprite, "build/signalred-battle-sprite.bin");
BN67_DUST_SPRITE(signalred_battle_sprite);
BN67_FIELD_OBJECT(signalred_battle_sprite, 0, 0, 1);
BN67_SONG(
    signalred_spawn_song,
    BN67_PCM(
        signalred_spawn,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xDC,0x3C,0x7F,0x8D,0xB1,0x00\n",
        "build/signalred-spawn-sample.bin"
    )
);
BN67_INCBIN(signalred_icon, "build/signalred-icon.bin");
BN67_INCBIN(signalred_image, "build/signalred-image.bin");
BN67_INCBIN(signalred_palette, "build/signalred-palette.bin");

BN67_CHIP_RECORD(0x0c1) {
    .codes = {
        EXE6_CHIP_CODE_S,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_OBSTACLE,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 80,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x00,
        .family = BN67_ATTACK_FAMILY(signalred_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(signalred_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x14,
    .library_flags = 0xC4,
    .library_lock_on_type = 0x40,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00C6,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = signalred_icon,
    .image = signalred_image,
    .palette = signalred_palette,
};

static const uint32_t GREEN_SFX = 0x00D1;
static const uint16_t STARTUP_TICKS = 3;
static const uint16_t RED_TICKS = 420;
static const uint16_t GREEN_TICKS = 50;
static const uint16_t OBJ_HP = 100;
static const Exe6HitType PASSIVE_HIT_TYPE =
    EXE6_HIT_TYPE_13;
static const Exe6HitType TARGET_HIT_TYPE =
    EXE6_HIT_TYPE_14;
static const uint8_t STARTUP_PENDING_FLAG = 0x01;
static const uint8_t HIT_DEFERRED_FLAG = 0x80;
static const uint8_t INITIAL_REMOVAL_FLAGS =
    STARTUP_PENDING_FLAG | HIT_DEFERRED_FLAG;
static const uint8_t DEPLOYABLE_REGISTERED_WORK = 0;
static const uint8_t OPPONENT_CHIPS_DISABLED_WORK = 1;
static const uint32_t SPAWN_BLOB_EFFECT = 0x15;
static const uint32_t DESTROY_EFFECT = 0x00;
static const int32_t DESTROY_EFFECT_HEIGHT = 0x00100000;
static const uint32_t FIELD_OBJECT_REMOVAL_EFFECT = 0x14;
static const int32_t FIELD_OBJECT_REMOVAL_EFFECT_HEIGHT = 0x000C0000;
static const uint32_t DESTROY_SFX = 0x70;

enum LightState {
    LIGHT_STATE_RED,
    LIGHT_STATE_GREEN = 4,
};

enum LaunchStep {
    LAUNCH_STEP_INIT,
    LAUNCH_STEP_ACTIVE = 4,
};

static uint32_t opponent_chip_enable_flag(const Exe6Obj *obj)
{
    return obj->owner == 0
        ? EXE6_BATTLE_CONTROL_FLAG_SIDE_1_CHIPS_ENABLED
        : EXE6_BATTLE_CONTROL_FLAG_SIDE_0_CHIPS_ENABLED;
}

static void enable_opponent_chips(Exe6Obj *obj)
{
    if (obj->work[OPPONENT_CHIPS_DISABLED_WORK] == 0) {
        return;
    }
    exe6_battle_report_flag_on(opponent_chip_enable_flag(obj));
    obj->work[OPPONENT_CHIPS_DISABLED_WORK] = 0;
}

static void disable_opponent_chips(Exe6Obj *obj)
{
    exe6_battle_report_flag_off(opponent_chip_enable_flag(obj));
    obj->work[OPPONENT_CHIPS_DISABLED_WORK] = 1;
}

static void obj_animate(Exe6Obj *obj)
{
    (void)obj;
    exe6_battle_hit_set(0, PASSIVE_HIT_TYPE);
    exe6_battle_obj_char_move();
}

static void obj_begin_destroy(Exe6Obj *obj)
{
    obj->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    obj->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static void obj_begin_damage_destroy(Exe6Obj *obj)
{
    (void)exe6_set_efc00(
        0,
        obj->x,
        obj->y,
        obj->z + DESTROY_EFFECT_HEIGHT,
        DESTROY_EFFECT
    );
    exe6_sound_req(DESTROY_SFX);
    obj_begin_destroy(obj);
}

static void obj_show_spawn_blob(Exe6Obj *obj)
{
    (void)exe6_set_efc00(
        0,
        obj->x,
        obj->y,
        obj->z + DESTROY_EFFECT_HEIGHT,
        SPAWN_BLOB_EFFECT
    );
}

static void obj_begin_placement_failure(Exe6Obj *obj)
{
    obj_show_spawn_blob(obj);
    obj_begin_destroy(obj);
}

static void obj_destroy(Exe6Obj *obj)
{
    enable_opponent_chips(obj);
    if (obj->work[DEPLOYABLE_REGISTERED_WORK] != 0) {
        exe6_cube_delete();
        obj->work[DEPLOYABLE_REGISTERED_WORK] = 0;
    }
    Exe6Hit *hit = obj->hit;
    if (hit != NULL) {
        exe6_battle_hit_check(hit);
        exe6_battle_hit_off(hit);
        hit = obj->hit;
        exe6_battle_hit_close(hit);
    }
    exe6_obj_move_delete();
}

static void play_spawn_sound(Exe6Obj *obj)
{
    exe6_sound_req(BN67_SONG_ID(signalred_spawn_song));
    obj->aux_timer = RED_TICKS;
    disable_opponent_chips(obj);
}

static void play_green_sound(Exe6Obj *obj)
{
    (void)obj;
    exe6_sound_req(GREEN_SFX);
}

static void obj_turn_green(Exe6Obj *obj)
{
    obj->animation_state = LIGHT_STATE_GREEN;
    obj->animation = 1;
    exe6_obj_dma_seq_set(1);
    exe6_obj_char_set();
    obj->aux_timer = GREEN_TICKS;
    enable_opponent_chips(obj);
    play_green_sound(obj);
}

static void obj_cycle_update(Exe6Obj *obj)
{
    uint16_t timer = (uint16_t)(obj->aux_timer - 1u);
    obj->aux_timer = timer;
    if ((int16_t)timer >= 0) {
        obj_animate(obj);
        return;
    }
    if (obj->animation_state == LIGHT_STATE_RED) {
        obj_turn_green(obj);
        obj_animate(obj);
        return;
    }

    obj->animation_state = LIGHT_STATE_RED;
    obj->animation = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    obj->aux_timer = RED_TICKS;
    disable_opponent_chips(obj);
    obj_animate(obj);
}

static void obj_normal_update(Exe6Obj *obj)
{
    if ((obj->removal_state & STARTUP_PENDING_FLAG) == 0) {
        obj_cycle_update(obj);
        return;
    }

    uint16_t timer = (uint16_t)(obj->aux_timer - 1u);
    obj->aux_timer = timer;
    if (timer != 0) {
        obj_animate(obj);
        return;
    }
    enum Bn67DeployablePlacementResult placement =
        bn67_deployable_placement_check(obj->block_x, obj->block_y);
    if (placement == BN67_DEPLOYABLE_PLACEMENT_INVALID) {
        obj_begin_placement_failure(obj);
        return;
    }
    if (placement == BN67_DEPLOYABLE_PLACEMENT_OCCUPIED) {
        obj_begin_damage_destroy(obj);
        return;
    }
    obj->removal_state &= (uint8_t)~STARTUP_PENDING_FLAG;
    /* Do not replace an occupying SignalRed's slot before rejecting. */
    exe6_cube_entry(obj, obj->owner, 1);
    obj->work[DEPLOYABLE_REGISTERED_WORK] = 1;
    play_spawn_sound(obj);
    obj_animate(obj);
}

static void obj_store_dust_ammo(Exe6Obj *obj)
{
    exe6_cube_set_dust_suikomi_efc(BN67_DUST_KIND(signalred_battle_sprite));
    obj_destroy(obj);
}

static bool obj_handle_removal_request(Exe6Obj *obj)
{
    uint32_t secondary_flags = exe6_battle_hit_req_flag_get();
    if ((secondary_flags &
         (EXE6_HIT_SECONDARY_FLAG_DUST_SUCTION_SIDE_0 |
          EXE6_HIT_SECONDARY_FLAG_DUST_SUCTION_SIDE_1)) != 0) {
        obj_store_dust_ammo(obj);
        return true;
    }

    if ((secondary_flags &
         (EXE6_HIT_SECONDARY_FLAG_TIMED_BLINK_REMOVAL |
          EXE6_HIT_SECONDARY_FLAG_FIELD_OBJECT_REMOVAL)) == 0) {
        return false;
    }

    uint32_t erase_result = exe6_cube_erase2();
    if (erase_result == 0) {
        return true;
    }
    if (erase_result == 2) {
        (void)exe6_set_efc00(
            0,
            obj->x,
            obj->y,
            obj->z + FIELD_OBJECT_REMOVAL_EFFECT_HEIGHT,
            FIELD_OBJECT_REMOVAL_EFFECT
        );
    }
    obj_begin_destroy(obj);
    return true;
}

static void obj_update(Exe6Obj *obj)
{
    obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;

    uint8_t removal = obj->removal_state;
    if ((removal & HIT_DEFERRED_FLAG) != 0) {
        if (exe6_battle_event_busy_check() != 0) {
            if ((removal & STARTUP_PENDING_FLAG) != 0) {
                obj_normal_update(obj);
            } else {
                obj_animate(obj);
            }
            return;
        }
        Exe6Hit *hit = obj->hit;
        exe6_battle_hit_data_set(
            hit,
            PASSIVE_HIT_TYPE,
            TARGET_HIT_TYPE,
            3
        );
        exe6_battle_hit_set(0, PASSIVE_HIT_TYPE);
        obj->removal_state = (uint8_t)(
            removal & (uint8_t)~HIT_DEFERRED_FLAG
        );
    }

    exe6_cube_hit_check();
    exe6_cube_life_span_check();
    Exe6Hit *hit = obj->hit;
    uint32_t damage = hit->final_damage;
    if (damage != 0) {
        exe6_obj_flash_set();
        exe6_enemy_life_sub(damage);
    }
    if (exe6_battle_end_check() != 0) {
        obj_begin_destroy(obj);
        return;
    }
    if (obj->hp == 0) {
        obj_begin_damage_destroy(obj);
        return;
    }
    if (obj_handle_removal_request(obj)) {
        return;
    }
    if (exe6_battle_event_busy_check() != 0) {
        obj_animate(obj);
        return;
    }
    obj_normal_update(obj);
}

static void obj_init(Exe6Obj *obj)
{
    if (exe6_block_in_screen_check() == 0) {
        obj_destroy(obj);
        return;
    }

    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(signalred_battle_sprite),
        BN67_SPRITE_ID(signalred_battle_sprite)
    );
    exe6_obj_shadow_set();
    obj->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_flip_set(obj->owner);
    exe6_block_to_pos();
    /* Animation 0 includes SignalRed's native materialization frames. */
    obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;

    obj->hp = OBJ_HP;
    obj->max_hp = OBJ_HP;
    obj->name_id = (uint16_t)BN67_FIELD_OBJECT_ID(signalred_battle_sprite);
    obj->aux_timer = STARTUP_TICKS;
    obj->timer = 0x0960;
    if (exe6_battle_hit_open() == NULL) {
        obj_destroy(obj);
        return;
    }

    obj->animation_state = LIGHT_STATE_RED;
    obj->removal_state = INITIAL_REMOVAL_FLAGS;
    obj->state_word = EXE6_OBJECT_STATE_ACTIVE;
    exe6_battle_obj_char_move();
}

static Exe6Obj *spawn_persistent(Exe6Obj *controller)
{
    Exe6Obj *owner = controller->parent;
    uint32_t block_x = owner->block_x;
    uint32_t block_y = owner->block_y;
    block_x += exe6_calc_pl_em_dir_spd_for(owner);
    if (exe6_block_in_screen_check_sub(block_x, block_y) == 0) {
        return NULL;
    }

    uint64_t coordinates = exe6_get_block_pos(block_x, block_y);
    Exe6Obj *obj = exe6_shl_open(
        BN67_OBJ_ID(signalred_obj_main),
        (int32_t)(uint32_t)coordinates,
        (int32_t)(uint32_t)(coordinates >> 32),
        0,
        exe6_obj_spawn_empty()
    );
    if (obj == NULL) {
        return NULL;
    }

    obj->block_x = (uint8_t)block_x;
    obj->block_y = (uint8_t)block_y;
    uint8_t owner_side = owner->owner;
    obj->owner = owner_side;
    obj->work[DEPLOYABLE_REGISTERED_WORK] = 0;
    obj->work[OPPONENT_CHIPS_DISABLED_WORK] = 0;
    obj->parent = owner;
    obj->attack = controller->attack;
    obj->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    return obj;
}

static void launch_effect(Exe6Obj *controller)
{
    if (controller->substate == LAUNCH_STEP_INIT) {
        (void)spawn_persistent(controller);
        controller->timer = 30;
        controller->substate = LAUNCH_STEP_ACTIVE;
    }

    uint16_t timer = (uint16_t)(controller->timer - 1u);
    controller->timer = timer;
    if (timer == 0) {
        controller->phase = EXE6_EVENT_CHIP_PHASE_OUTRO;
        controller->phase_timer = LAUNCH_STEP_INIT;
    }
}

static void update(Exe6Obj *controller)
{
    switch (controller->phase) {
    case EXE6_EVENT_CHIP_PHASE_FADE:
        exe6_event_chip_common_fade();
        break;
    case EXE6_EVENT_CHIP_PHASE_TELOP:
        exe6_event_chip_common_telop();
        break;
    case EXE6_EVENT_CHIP_PHASE_EFFECT:
        launch_effect(controller);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

BN67_EFFECT(signalred_controller_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        exe6_event_chip_common_init();
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        update(self);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_SHELL(signalred_obj_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        obj_init(self);
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        obj_update(self);
        break;
    default:
        obj_destroy(self);
        break;
    }
}

BN67_PERSISTENT_ATTACK(0x0C1, signalred_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(signalred_controller_main), spawn_parameters
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
