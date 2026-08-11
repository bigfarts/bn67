#include "abi.h"
#include "runtime.h"

BN67_SPRITE(searchman_battle_sprite, "build/searchman-battle-sprite.bin");
BN67_SPRITE(searchman_reticle_alt_sprite, "build/searchman-reticle-alt.bin");
BN67_SPRITE(searchman_reticle_sprite, "build/searchman-reticle.bin");

BN67_INCBIN(searchman_image, "build/searchman-image.bin");
BN67_INCBIN(searchman_palette_base, "build/searchman-pal-base.bin");
BN67_ASM_RESOURCE(
    searchman_palette_ex,
    ".incbin \"build/searchman-pal-base.bin\",0,0x1A\n"
    ".short 0x03FF,0x0299,0x0190\n"
);
BN67_INCBIN(searchman_palette_sp, "build/searchman-pal-sp.bin");

#if FALZAR
#define SEARCHMAN_ICON ((const uint8_t *)0x0872BE14u)
#else
#define SEARCHMAN_ICON ((const uint8_t *)0x08729D50u)
#endif

BN67_CHIP_RECORD(0x107) {
    .codes = {
        EXE6_CHIP_CODE_S,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_CURSOR,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 52,
    .behavior = {
        .effect_flags = 0x47,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(searchman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(searchman_attack_main),
        .dark_soul_usage = 0x1F,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x1C,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 20,
    .library_sort_order = 0x0107,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = SEARCHMAN_ICON,
    .image = searchman_image,
    .palette = searchman_palette_base,
};

BN67_CHIP_RECORD(0x108) {
    .codes = {
        EXE6_CHIP_CODE_S,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_CURSOR,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 64,
    .behavior = {
        .effect_flags = 0x47,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(searchman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(searchman_attack_main),
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = { .variant = 3 },
        .delay = 0,
    },
    .library_number = 0x1D,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 40,
    .library_sort_order = 0x0108,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = SEARCHMAN_ICON,
    .image = searchman_image,
    .palette = searchman_palette_ex,
};

BN67_CHIP_RECORD(0x109) {
    .codes = {
        EXE6_CHIP_CODE_S,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_CURSOR,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 86,
    .behavior = {
        .effect_flags = 0x47,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(searchman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(searchman_attack_main),
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = { .variant = 4 },
        .delay = 0,
    },
    .library_number = 0x1E,
    .library_flags = 0x00,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 75,
    .library_sort_order = 0x0109,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = SEARCHMAN_ICON,
    .image = searchman_image,
    .palette = searchman_palette_sp,
};

static const uint8_t ACTOR_ACTIVE_STATE = 4;
static const uint8_t ACTOR_DESTROY_STATE = 8;
static const uint8_t APPEAR_PHASE = 0;
static const uint8_t ATTACK_PHASE = 4;
static const uint8_t EXIT_PHASE = 8;
static const uint8_t RETICLE_SCAN_PHASE = 4;
static const uint8_t RETICLE_LOCKED_PHASE = 8;
static const Exe6HitType NORMAL_HIT_TYPE =
    EXE6_HIT_TYPE_INVIS_PIERCING_OBJECT_HITTING_ATTACK;
static const Exe6HitType DELETE_HIT_TYPE =
    EXE6_HIT_TYPE_INVIS_PIERCING_OBJECT_HITTING_DELETE_ACTIVE_CHIP_ATTACK;
static const uint8_t SHOT_COUNT = 5;
static const uint16_t BLOCK_WAIT_FRAMES = 20;
static const uint16_t SHOT_FRAMES = 10;
static const uint16_t SHOT_COOLDOWN_FRAMES = 30;
static const uint16_t EXIT_FRAMES = 5;
static const uint16_t RETICLE_LIFETIME = 300;
static const uint16_t RETICLE_LOCK_FRAMES = 50;
static const uint8_t RETICLE_SCAN_FRAMES_BASE = 6;
static const uint8_t RETICLE_SCAN_FRAMES_EX = 5;
static const uint8_t RETICLE_SCAN_FRAMES_SP = 4;
static const uint8_t SEARCHMAN_VARIANT_EX = 3;
static const uint8_t SEARCHMAN_VARIANT_SP = 4;
static const uint32_t IMPACT_RANDOM_MASK = 0x0F;
static const Exe6HitType HIT_SELECTOR =
    EXE6_HIT_TYPE_STANDARD_TARGET;
static const uint32_t PRESENT_HIT_REGION = HIT_SELECTOR << 3;

struct Exe6SearchmanActorWork {
    uint32_t scale;                      // +0x60
};

struct Exe6SearchmanReticleWork {
    Exe6Obj *player;                      // +0x60
    uint32_t alternate;                  // +0x64
};

static bool timer_positive_after_decrement(Exe6Obj *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer > 0;
}

static void set_animation(Exe6Obj *self, uint8_t animation)
{
    self->animation = animation;
    self->palette = 0;
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
}

static void pulse_scale(Exe6Obj *self)
{
    struct Exe6SearchmanActorWork *work =
        (struct Exe6SearchmanActorWork *)self->work;
    exe6_obj_col_efc_set(work->scale * 1057u);
}

static void actor_animate(Exe6Obj *self)
{
    (void)self;
    exe6_battle_obj_char_move2();
    pulse_scale(self);
}

static void actor_destroy(Exe6Obj *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    exe6_obj_move_delete();
}

static void appear(Exe6Obj *self)
{
    struct Exe6SearchmanActorWork *work =
        (struct Exe6SearchmanActorWork *)self->work;
    if (self->substate == 0) {
        work->scale = 0x1F;
        exe6_sound_req(0x94);
        self->timer = 0;
        self->substate = 4;
        return;
    }

    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    ++self->timer;
    if ((self->timer & 2u) != 0) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    }

    int32_t scale = (int32_t)work->scale - 2;
    work->scale = (uint32_t)scale;
    if (scale > 0) {
        return;
    }

    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    work->scale = 0;
    self->phase_timer = 4;
}

static void wait_for_block(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->timer = BLOCK_WAIT_FRAMES;
        self->substate = 4;
        return;
    }
    if (timer_positive_after_decrement(self)) {
        return;
    }
    if (exe6_block_move_check(
            self->block_x,
            self->block_y,
            EXE6_BLOCK_FLAG_SOLID,
            0
        ) == 0) {
        self->state_word = ACTOR_DESTROY_STATE;
        return;
    }
    self->phase = ATTACK_PHASE;
    self->phase_timer = 0;
}

static void appear_phase(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        appear(self);
    } else {
        wait_for_block(self);
    }
}

static void spawn_reticle(Exe6Obj *actor, Exe6Obj *player, uint32_t alternate)
{
    Exe6Obj *reticle = exe6_efc_open(
        BN67_OBJ_ID(searchman_reticle_main),
        exe6_obj_spawn_with_variant(actor->variant)
    );
    if (reticle == NULL) {
        return;
    }
    struct Exe6SearchmanReticleWork *work =
        (struct Exe6SearchmanReticleWork *)reticle->work;
    work->player = player;
    reticle->owner_word = actor->owner_word;
    work->alternate = alternate;
    reticle->parent = actor;
}

static void start_reticle(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->animation = 0x10;
        self->subvariant = 0;
        self->target_block_x = 0;
        self->target_block_y = 0;
        spawn_reticle(self, self->parent, 0);
        self->substate = 4;
        return;
    }
    if (self->target_block_x != 0) {
        self->phase_timer = 4;
    }
}

static void spawn_hit(Exe6Obj *actor, bool delete_shot)
{
    Exe6HitType hit_type = delete_shot
        ? DELETE_HIT_TYPE
        : NORMAL_HIT_TYPE;
    Exe6Obj *hit = exe6_shl_open(
        BN67_OBJ_ID(searchman_hit_main),
        0,
        0,
        0,
        exe6_obj_spawn_with_variant((uint8_t)hit_type)
    );
    if (hit == NULL) {
        return;
    }
    hit->block_x = actor->target_block_x;
    hit->block_y = actor->target_block_y;
    hit->parameter = actor->parameter;
    hit->attack = actor->attack;
    hit->owner_word = actor->owner_word;
    hit->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void next_shot(Exe6Obj *self)
{
    self->palette = 0x12;
    self->timer = SHOT_FRAMES;
    exe6_sound_req(0xB9);
    self->removal_state = 0;
    self->substate = 4;
}

static void fire_tick(Exe6Obj *self)
{
    if (self->timer == 7 && self->removal_state == 0) {
        self->removal_state = 1;
        bool delete_shot = self->subvariant != 0
            && self->animation_state == 1;
        spawn_hit(self, delete_shot);
    }

    if (timer_positive_after_decrement(self)) {
        return;
    }
    --self->animation_state;
    if ((int8_t)self->animation_state > 0) {
        next_shot(self);
    } else {
        self->phase_timer = 8;
    }
}

static void fire_shots(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->animation_state = SHOT_COUNT;
        self->animation = 0x11;
        next_shot(self);
    } else {
        fire_tick(self);
    }
}

static void shot_cooldown(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->animation = 0;
        self->timer = SHOT_COOLDOWN_FRAMES;
        self->substate = 4;
        return;
    }
    if (!timer_positive_after_decrement(self)) {
        self->phase = EXIT_PHASE;
        self->phase_timer = 0;
    }
}

static void attack_phase(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        start_reticle(self);
    } else if (self->phase_timer_low == 4) {
        fire_shots(self);
    } else {
        shot_cooldown(self);
    }
}

static void exit_phase(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        self->animation_word = 4;
        self->timer = EXIT_FRAMES;
        self->phase_timer_low = 4;
        return;
    }
    if (!timer_positive_after_decrement(self)) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = ACTOR_DESTROY_STATE;
    }
}

static void actor_update(Exe6Obj *self)
{
    if (self->phase == APPEAR_PHASE) {
        appear_phase(self);
    } else if (self->phase == ATTACK_PHASE) {
        attack_phase(self);
    } else {
        exit_phase(self);
    }
    actor_animate(self);
}

static void actor_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(searchman_battle_sprite),
        BN67_SPRITE_ID(searchman_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    set_animation(self, 0);
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    exe6_obj_col_efc_set(0x7FFF);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = ACTOR_ACTIVE_STATE;
    actor_update(self);
}

static bool reticle_key_pressed(const Exe6Obj *self, uint16_t mask)
{
    const struct Exe6SearchmanReticleWork *work =
        (const struct Exe6SearchmanReticleWork *)self->work;
    return (work->player->runtime_data->input & mask) == mask;
}

static void reticle_commit_target(Exe6Obj *self)
{
    Exe6Obj *actor = self->parent;
    actor->target_block_x = self->block_x;
    actor->target_block_y = self->block_y;
}

static void reticle_find_bounds(Exe6Obj *self, int32_t *enemy, int32_t *own)
{
    int32_t direction = -(int32_t)exe6_calc_pl_em_dir_spd_for(self);
    int32_t block = 6 - 5 * (int32_t)(self->owner ^ self->owner_aux);

    while (block >= 1 && block <= 6) {
        if (exe6_another_block_exist_check((uint32_t)block, self->owner) != 0) {
            break;
        }
        block += direction;
    }
    *own = block;

    while (block >= 1 && block <= 6) {
        if (exe6_another_block_exist_check((uint32_t)block, self->owner ^ 1u) == 3) {
            break;
        }
        block += direction;
    }
    *enemy = block - direction;
}

static void reticle_set_initial_block(Exe6Obj *self)
{
    int32_t enemy;
    int32_t own;
    reticle_find_bounds(self, &enemy, &own);
    self->block_x = (uint8_t)enemy;

    int32_t front = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->target_block_y = (uint8_t)(own + front);
    self->target_block_x = (uint8_t)(enemy - front);
    self->block_y = 1;
}

static bool reticle_change_row(Exe6Obj *self)
{
    int32_t row = self->block_y;
    if (self->subvariant != 0) {
        ++row;
        if (row <= 3) {
            self->block_y = (uint8_t)row;
            return false;
        }
        self->subvariant = 0;
    } else {
        --row;
        if (row >= 1) {
            self->block_y = (uint8_t)row;
            return false;
        }
        self->subvariant = 1;
    }
    return true;
}

static void reticle_change_column(Exe6Obj *self)
{
    for (uint32_t attempt = 0; attempt < 2; ++attempt) {
        int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
        if (self->removal_state == 0) {
            direction = -direction;
        }
        int32_t column = (int32_t)self->block_x + direction;
        if (column != self->target_block_x && column != self->target_block_y) {
            self->block_x = (uint8_t)column;
            return;
        }
        self->removal_state ^= 1u;
    }
}

static void reticle_scan(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        uint8_t frames = RETICLE_SCAN_FRAMES_BASE;
        if (self->variant == SEARCHMAN_VARIANT_EX) {
            frames = RETICLE_SCAN_FRAMES_EX;
        } else if (self->variant == SEARCHMAN_VARIANT_SP) {
            frames = RETICLE_SCAN_FRAMES_SP;
        }
        self->timer = frames;
        self->phase_timer_low = frames;
    }

    if (reticle_key_pressed(self, 1)) {
        if (!reticle_key_pressed(self, 2)) {
            exe6_sound_req(0x8B);
            self->parent->subvariant = 1;
        }
        self->animation_state = 5;
        self->phase = RETICLE_LOCKED_PHASE;
        self->phase_timer_low = 0;
        return;
    }

    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    if (timer < 0) {
        self->phase = self->aux_timer == 0 ? RETICLE_LOCKED_PHASE : 0;
        self->phase_timer_low = 0;
    }
}

static void reticle_step(Exe6Obj *self)
{
    if (reticle_change_row(self)) {
        reticle_change_column(self);
    }
    exe6_block_to_pos();
    exe6_sound_req(0x10E);
    self->phase = RETICLE_SCAN_PHASE;
    reticle_scan(self);
}

static void reticle_locked(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        self->animation = 1;
        exe6_sound_req(0xBD);
        reticle_commit_target(self);
        self->timer = RETICLE_LOCK_FRAMES;
        self->phase_timer_low = RETICLE_LOCK_FRAMES;
    }

    if (self->animation_state != 0) {
        --self->animation_state;
        if (!reticle_key_pressed(self, 2)) {
            self->parent->subvariant = 1;
        }
    }
    if (!timer_positive_after_decrement(self)) {
        self->state = ACTOR_DESTROY_STATE;
    }
}

static void reticle_update(Exe6Obj *self)
{
    if (self->phase == 0) {
        reticle_step(self);
    } else if (self->phase == RETICLE_SCAN_PHASE) {
        reticle_scan(self);
    } else {
        reticle_locked(self);
    }
    if (self->aux_timer != 0) {
        --self->aux_timer;
    }
    exe6_battle_obj_char_move2();
}

static void reticle_init(Exe6Obj *self)
{
    struct Exe6SearchmanReticleWork *work =
        (struct Exe6SearchmanReticleWork *)self->work;
    uint32_t group = BN67_SPRITE_GROUP(searchman_reticle_sprite);
    uint32_t sprite = BN67_SPRITE_ID(searchman_reticle_sprite);
    if (work->alternate != 0) {
        group = BN67_SPRITE_GROUP(searchman_reticle_alt_sprite);
        sprite = BN67_SPRITE_ID(searchman_reticle_alt_sprite);
    }
    exe6_obj_char_init(0x80, group, sprite);
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    set_animation(self, 0);
    exe6_obj_clt_set(0);
    reticle_set_initial_block(self);
    exe6_block_to_pos();
    self->subvariant = 1;
    self->removal_state = 1;
    self->aux_timer = RETICLE_LIFETIME;

    self->animation_state = 0;
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->z = 0;
    self->state_word = ACTOR_ACTIVE_STATE;
    self->phase = RETICLE_SCAN_PHASE;
    reticle_update(self);
}

static void randomize_impact(int32_t *x, int32_t *z)
{
    uint32_t random = exe6_rand();
    int32_t x_offset = (int32_t)(random & IMPACT_RANDOM_MASK) - 7;
    int32_t z_offset = (int32_t)((random >> 16) & IMPACT_RANDOM_MASK) - 7;
    *x += x_offset << 16;
    *z += z_offset << 16;
}

static void hit_update(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();

    if (hit->received_hit_flags == 0) {
        uint32_t random = exe6_rand();
        Exe6ObjSpawnParameters effect_parameters = {
            .variant = 7,
            .subvariant = (uint8_t)(random & 2u),
        };
        int32_t x = self->x;
        int32_t y = self->y;
        int32_t z = self->z;
        randomize_impact(&x, &z);
        (void)exe6_efc_open_at(0, x, y, z, effect_parameters);
    }

    exe6_battle_hit_off(hit);
    exe6_battle_hit_close(self->hit);
    exe6_obj_move_delete();
}

static void hit_init(Exe6Obj *self)
{
    if (exe6_block_in_screen_check() == 0) {
        exe6_obj_move_delete();
        return;
    }
    exe6_block_to_pos();
    self->z = 0x00100000;
    Exe6Hit *hit = exe6_battle_hit_open();
    if (hit == NULL) {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_hit_data_set(
        hit,
        (Exe6HitType)self->variant,
        HIT_SELECTOR,
        3
    );
    /* Type 29 carries the delete property; effect 9 supplies its contact VFX. */
    Exe6HitEffect hit_effect = self->variant == NORMAL_HIT_TYPE
        ? EXE6_HIT_EFFECT_NORMAL
        : EXE6_HIT_EFFECT_CHIP_DELETE;
    exe6_battle_hit_hit_mark_set(hit_effect);
    exe6_battle_hit_set(0, PRESENT_HIT_REGION);
    self->state_word = ACTOR_ACTIVE_STATE;
}

BN67_SHELL(searchman_hit_main)
{
    if (self->state == 0) {
        hit_init(self);
    } else if (self->state == ACTOR_ACTIVE_STATE) {
        hit_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_EFFECT(searchman_reticle_main)
{
    if (self->state == 0) {
        reticle_init(self);
    } else if (self->state == ACTOR_ACTIVE_STATE) {
        reticle_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_ENEMY(searchman_actor_main)
{
    if (self->state == 0) {
        actor_init(self);
    } else if (self->state == ACTOR_ACTIVE_STATE) {
        actor_update(self);
    } else {
        actor_destroy(self);
    }
}

BN67_SUMMON_ATTACK(0x107, searchman_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(searchman_actor_main), spawn_parameters
    );
    if (actor == NULL) {
        return;
    }
    actor->block_x = (uint8_t)block_x;
    actor->block_y = (uint8_t)block_y;
    actor->parameter = (uint8_t)parameter;
    actor->owner_word = owner->owner_word;
    actor->parent = owner;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
