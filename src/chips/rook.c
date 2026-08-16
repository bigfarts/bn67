#include "runtime.h"

BN67_SPRITE(rook_battle_sprite, "build/rook-battle-sprite.bin");
BN67_DUST_SPRITE(rook_battle_sprite);
BN67_FIELD_OBJECT(rook_battle_sprite, 4, 0, 1);
BN67_USE_SONG(signalred_spawn_song);
BN67_INCBIN(rook_icon, "build/rook-icon.bin");
BN67_INCBIN(rook_image, "build/rook-image.bin");
BN67_INCBIN(rook_palette, "build/rook-palette.bin");

BN67_CHIP_RECORD(0x0c0) {
    .codes = {
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_OBSTACLE,
    .chip_class = EXE6_CHIP_CLASS_STANDARD,
    .mb = 30,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x00,
        .family = BN67_ATTACK_FAMILY(rook_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(rook_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0xC4,
    .library_flags = 0x80,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x00C4,
    .library_gate_usage = 0x03,
    .dark_chip_id = UINT8_MAX,
    .icon = rook_icon,
    .image = rook_image,
    .palette = rook_palette,
};

static const uint16_t ROOK_HP = 500;
static const uint16_t ROOK_LIFETIME = 0x0708;
static const uint16_t STARTUP_TICKS = 3;
static const uint8_t ROOK_ANIMATION = 4;
static const Exe6HitType PASSIVE_HIT_TYPE =
    EXE6_HIT_TYPE_13;
static const Exe6HitType TARGET_HIT_TYPE =
    EXE6_HIT_TYPE_14;
static const uint8_t STARTUP_PENDING_FLAG = 0x01;
static const uint8_t HIT_DEFERRED_FLAG = 0x80;
static const uint8_t INITIAL_REMOVAL_FLAGS =
    STARTUP_PENDING_FLAG | HIT_DEFERRED_FLAG;
static const uint8_t SPAWNER_SIDE_WORK = 0;
static const uint8_t DEPLOYABLE_REGISTERED_WORK = 1;
static const uint8_t PUSH_ACTIVE_WORK = 2;
/* Native AirShot uses 0x61; WindRack and Tengu Racket share 0x49. */
static const uint8_t AIRSHOT_HIT_MODIFIER = 0x61;
static const uint8_t RACKET_HIT_MODIFIER = 0x49;
/* Native Navi AirShot knockback moves ten pixels per frame. */
static const int32_t AIRSHOT_PUSH_SPEED = 0x000A0000;
static const uint32_t PUSH_COLLISION_BLOCK_FLAGS =
    EXE6_BLOCK_FLAG_SUPPORT_OBJECT |
    EXE6_BLOCK_FLAG_SIDE_1_NAVI |
    EXE6_BLOCK_FLAG_SIDE_0_NAVI |
    EXE6_BLOCK_FLAG_SIDE_1_HIT |
    EXE6_BLOCK_FLAG_SIDE_0_HIT;
static const uint32_t SPAWN_BLOB_EFFECT = 0x15;
static const uint32_t DESTROY_EFFECT = 0x00;
static const int32_t DESTROY_EFFECT_HEIGHT = 0x00100000;
static const uint32_t FIELD_OBJECT_REMOVAL_EFFECT = 0x14;
static const int32_t FIELD_OBJECT_REMOVAL_EFFECT_HEIGHT = 0x000C0000;
static const uint32_t DESTROY_SFX = 0x70;

/*
 * WindRack and Tengu's wind object only checks the neutral support-object bit
 * before continuing through a panel. Keep Rook's owner-specific collision so
 * friendly attacks pass, but extend that check to the opposing side's support
 * bit. The relocated Custom-opening path provides an eight-byte relay slot.
 */
#if FALZAR
BN67_PATCH_SECTION(0x080CD418, 0x080E42D0, rook_windbreak_filter_main);
#define ROOK_WINDBREAK_FILTER_RETURN 0x080CD41F
#else
BN67_PATCH_SECTION(0x080CEC88, 0x080E42D0, rook_windbreak_filter_main);
#define ROOK_WINDBREAK_FILTER_RETURN 0x080CEC8F
#endif

NAKED void rook_windbreak_filter_main(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "ldr r0,[r7,#0x70]\n"
        "movs r1,#1\n"
        "lsls r1,r1,#23\n"
        "ldrb r2,[r5,#0x16]\n"
        "adds r2,#1\n"
        "adds r3,r1,#0\n"
        "lsls r3,r2\n"
        "orrs r1,r3\n"
        "tst r0,r1\n"
        "ldr r3,=" EXE6_STRINGIFY(ROOK_WINDBREAK_FILTER_RETURN) "\n"
        "bx r3\n"
    );
}

enum LaunchStep {
    LAUNCH_STEP_INIT,
    LAUNCH_STEP_ACTIVE = 4,
};

enum PushStartResult {
    PUSH_START_NONE,
    PUSH_START_ACTIVE,
    PUSH_START_BLOCKED,
};

static void obj_animate(Exe6Obj *obj)
{
    (void)obj;
    exe6_battle_hit_set(0, PASSIVE_HIT_TYPE);
    exe6_battle_obj_char_move();
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

static void obj_begin_placement_failure(Exe6Obj *obj)
{
    obj_show_spawn_blob(obj);
    obj_begin_destroy(obj);
}

static void obj_destroy(Exe6Obj *obj)
{
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

static void obj_normal_update(Exe6Obj *obj)
{
    if ((obj->removal_state & STARTUP_PENDING_FLAG) == 0) {
        obj_animate(obj);
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
    /* Do not replace an occupying Rook's deployable slot before rejecting. */
    exe6_cube_entry(obj, obj->owner, 0);
    obj->work[DEPLOYABLE_REGISTERED_WORK] = 1;
    exe6_sound_req(BN67_SONG_ID(signalred_spawn_song));
    obj_animate(obj);
}

static void obj_store_dust_ammo(Exe6Obj *obj)
{
    exe6_cube_set_dust_suikomi_efc(BN67_DUST_KIND(rook_battle_sprite));
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

static void obj_block_damage(Exe6Hit *hit)
{
    Exe6HitTypeFlag received = hit->received_hit_flags;
    if ((received & EXE6_HIT_TYPE_FLAG_GUARD_BLOCKED) == 0) {
        hit->received_hit_flags =
            received | EXE6_HIT_TYPE_FLAG_GUARD_BLOCKED;
        exe6_cube_guard_mark_check();
        hit->received_hit_flags = received;
    }
    hit->final_damage = 0;
    for (uint32_t i = 0; i < 5; i++) {
        hit->damage_buckets[i] = 0;
    }
}

static bool obj_received_push_attack(const Exe6Hit *hit)
{
    uint8_t modifier = hit->final_hit_modifier;
    return modifier == AIRSHOT_HIT_MODIFIER ||
           modifier == RACKET_HIT_MODIFIER;
}

static enum PushStartResult obj_begin_one_panel_push(Exe6Obj *obj)
{
    if (obj->work[PUSH_ACTIVE_WORK] != 0) {
        return PUSH_START_NONE;
    }

    int32_t direction = -(int32_t)exe6_calc_pl_em_dir_spd_for(obj);
    if (direction == 0) {
        return PUSH_START_NONE;
    }

    uint32_t block_x = (uint32_t)((int32_t)obj->block_x + direction);
    uint32_t block_y = obj->block_y;
    if (exe6_block_move_check(
            block_x,
            block_y,
            EXE6_BLOCK_FLAG_SOLID,
            0
        ) == 0) {
        return PUSH_START_NONE;
    }
    uint32_t block_flags = exe6_block_status_get(block_x, block_y);
    if ((block_flags & PUSH_COLLISION_BLOCK_FLAGS) != 0) {
        return PUSH_START_BLOCKED;
    }

    obj->target_block_x = (uint8_t)block_x;
    obj->velocity_x = direction * AIRSHOT_PUSH_SPEED;
    obj->work[PUSH_ACTIVE_WORK] = 1;
    return PUSH_START_ACTIVE;
}

static void obj_push_update(Exe6Obj *obj)
{
    if (obj->work[PUSH_ACTIVE_WORK] == 0) {
        return;
    }

    uint64_t coordinates = exe6_get_block_pos(
        obj->target_block_x,
        obj->block_y
    );
    int32_t target_x = (int32_t)(uint32_t)coordinates;
    int32_t next_x = obj->x + obj->velocity_x;
    bool reached = obj->velocity_x < 0 ?
        next_x <= target_x : next_x >= target_x;
    if (reached) {
        obj->x = target_x;
        obj->block_x = obj->target_block_x;
        obj->velocity_x = 0;
        obj->work[PUSH_ACTIVE_WORK] = 0;
    } else {
        obj->x = next_x;
        exe6_pos_to_block();
    }
    exe6_battle_hit_block_pos_set();
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
    if (obj_received_push_attack(hit)) {
        /* Push attacks deal no hit damage, even if the move is blocked. */
        obj_block_damage(hit);
        enum PushStartResult push = obj_begin_one_panel_push(obj);
        if (push == PUSH_START_BLOCKED) {
            obj_begin_damage_destroy(obj);
            return;
        }
    } else if (damage != 0) {
        if ((hit->received_hit_flags &
             EXE6_HIT_TYPE_FLAG_GUARD_PIERCING) != 0) {
            exe6_obj_flash_set();
            exe6_enemy_life_sub(damage);
        } else {
            obj_block_damage(hit);
        }
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

    obj_push_update(obj);
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
        BN67_SPRITE_GROUP(rook_battle_sprite),
        BN67_SPRITE_ID(rook_battle_sprite)
    );
    exe6_obj_shadow_set();
    obj->animation_word = (uint16_t)(
        ROOK_ANIMATION | (ROOK_ANIMATION << 8)
    );
    exe6_obj_dma_seq_set(ROOK_ANIMATION);
    exe6_obj_char_set();
    exe6_obj_clt_set(0);
    exe6_obj_char_move();
    exe6_obj_flip_set(obj->work[SPAWNER_SIDE_WORK] ^ 1u);
    exe6_block_to_pos();
    /* Animation 4 begins with BN3 Rook's own gray materialization frames. */
    obj->header_flags |= EXE6_OBJ_FLAG_VISIBLE;

    obj->hp = ROOK_HP;
    obj->max_hp = ROOK_HP;
    obj->name_id = (uint16_t)BN67_FIELD_OBJECT_ID(rook_battle_sprite);
    obj->aux_timer = STARTUP_TICKS;
    obj->timer = ROOK_LIFETIME;
    if (exe6_battle_hit_open() == NULL) {
        obj_destroy(obj);
        return;
    }

    obj->animation_state = 0;
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
        BN67_OBJ_ID(rook_obj_main),
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
    obj->work[SPAWNER_SIDE_WORK] = owner_side;
    obj->work[DEPLOYABLE_REGISTERED_WORK] = 0;
    obj->work[PUSH_ACTIVE_WORK] = 0;
    obj->parent = owner;
    obj->attack = controller->attack;
    obj->chip_data = controller->chip_data;
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

static void controller_update(Exe6Obj *controller)
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

BN67_EFFECT(rook_controller_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        exe6_event_chip_common_init();
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        controller_update(self);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_SHELL(rook_obj_main)
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

BN67_PERSISTENT_ATTACK(0x0c0, rook_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(rook_controller_main), spawn_parameters
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
