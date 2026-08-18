#include "common.h"
#include "runtime.h"

BN67_SPRITE(numberman_battle_sprite, "build/numberman_battle_sprite.bin");
BN67_SPRITE(numberman_die_sprite, "build/numberman_die_sprite.bin");

BN67_INCBIN(numberman_image, "build/numberman_image.bin");
BN67_INCBIN(numberman_palette_base, "build/numberman_pal_base.bin");
BN67_ASM_RESOURCE(
    numberman_palette_ex,
    ".incbin \"build/numberman_pal_base.bin\",0,0x1A\n"
    ".short 0x7EF4,0x75E9,0x44C1\n"
);
BN67_INCBIN(numberman_palette_sp, "build/numberman_pal_sp.bin");
BN67_SONG(
    numberman_explosion_song,
    ".byte 1,0,0x80,0\n"
    ".long numberman_explosion_voicegroup\n"
    ".long numberman_explosion_track\n"
    ".global numberman_explosion_voicegroup\n"
    "numberman_explosion_voicegroup:\n"
    ".byte 0x0C,0x3C,0,0,1,0,0,0,0,2,0,0\n"
    ".global numberman_explosion_track\n"
    "numberman_explosion_track:\n"
    ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBE,0x7F\n"
    ".byte 0xBF,0x40,0xD0,0x30,0x7F,0x82,0xED,0x33,0x9E,0xB1\n"
);

#if FALZAR
#define ICON ((const uint8_t *)0x0872BE14u)
#define LIBRARY_FLAGS 0x01
#define LIBRARY_NUMBER_BASE 0x0F
#define LIBRARY_SORT_BASE 0x00FE
#else
#define ICON ((const uint8_t *)0x08729D50u)
#define LIBRARY_FLAGS 0x00
#define LIBRARY_NUMBER_BASE 0x13
#define LIBRARY_SORT_BASE 0x00EF
#endif

BN67_CHIP_RECORD(0x0ef) {
    .codes = {
        EXE6_CHIP_CODE_N,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 33,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(numberman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(numberman_attack_main),
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = LIBRARY_NUMBER_BASE,
    .library_flags = LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 30,
    .library_sort_order = LIBRARY_SORT_BASE,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = numberman_image,
    .palette = numberman_palette_base,
};

BN67_CHIP_RECORD(0x0f0) {
    .codes = {
        EXE6_CHIP_CODE_N,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 50,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(numberman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(numberman_attack_main),
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .variant = 3 },
        .delay = 0,
    },
    .library_number = LIBRARY_NUMBER_BASE + 1,
    .library_flags = LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 40,
    .library_sort_order = LIBRARY_SORT_BASE + 1,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = numberman_image,
    .palette = numberman_palette_ex,
};

BN67_CHIP_RECORD(0x0f1) {
    .codes = {
        EXE6_CHIP_CODE_N,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 66,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(numberman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(numberman_attack_main),
        .dark_soul_usage = 0x00,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = { .variant = 4 },
        .delay = 0,
    },
    .library_number = LIBRARY_NUMBER_BASE + 2,
    .library_flags = LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 90,
    .library_sort_order = LIBRARY_SORT_BASE + 2,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = numberman_image,
    .palette = numberman_palette_sp,
};

enum ActorPhase {
    ACTOR_PHASE_APPEAR,
    ACTOR_PHASE_WAIT = 4,
    ACTOR_PHASE_THROW = 8,
    ACTOR_PHASE_WAIT_FOR_DIE = 12,
    ACTOR_PHASE_EXIT = 16,
};

enum DiePhase {
    DIE_PHASE_FLIGHT,
    DIE_PHASE_BOUNCE = 4,
    DIE_PHASE_ROLL = 8,
    DIE_PHASE_EXPLODE = 12,
    DIE_PHASE_CONTACT = 16,
};

enum UpdateStep {
    UPDATE_STEP_INIT,
    UPDATE_STEP_ACTIVE = 4,
};
static const uint16_t APPEAR_FRAMES = 4;
static const uint16_t WAIT_FRAMES = 40;
static const uint16_t THROW_FRAMES = 10;
static const uint16_t FLIGHT_FRAMES = 43;
static const uint16_t ROLL_FRAMES = 60;
static const uint16_t ROLL_FLASH_FRAMES = 50;
static const uint16_t AFTER_ROLL_FRAMES = 30;
static const uint16_t EXIT_FRAMES = 4;
static const int32_t DIE_INITIAL_HEIGHT = 24;
static const int32_t DIE_INITIAL_Z_VELOCITY = 4 << 16;
static const int32_t DIE_BOUNCE_Z_VELOCITY = 3 << 16;
static const int32_t DIE_GRAVITY = -0x3000;
static const int32_t DIE_CONTACT_HEIGHT = 25 << 16;
static const uint32_t DIE_DISTANCE = 3;
static const uint32_t EXPLOSION_EFFECT = 0;
static const uint32_t DIE_PRIORITY = 0;
static const Exe6BlockDamageProperties DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CENTERED_3X3,
    .hit_effect = EXE6_HIT_EFFECT_NONE,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_STANDARD_ATTACK,
};
static const Exe6BlockDamageProperties CONTACT_DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CURRENT_BLOCK,
    .hit_effect = EXE6_HIT_EFFECT_NONE,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_STANDARD_ATTACK,
};

/* This is BN5's exact 16-entry mapping from the battle RNG to die faces. */
static const uint8_t DIE_ROLLS[16] = {
    1, 1, 2, 2, 3, 3, 4, 3,
    4, 4, 5, 4, 5, 5, 6, 6,
};

struct ActorWork {
    uint8_t die_active;
};

static void actor_appear(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 3);
        exe6_sound_req(0x94);
        self->timer = APPEAR_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }

    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    if (decrement_timer(&self->timer) <= 0) {
        set_phase(self, ACTOR_PHASE_WAIT);
    }
}

static void actor_wait(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 0);
        self->timer = WAIT_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        if (exe6_block_move_check(
                self->block_x,
                self->block_y,
                EXE6_BLOCK_FLAG_SOLID,
                0
            ) == 0) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
        set_phase(self, ACTOR_PHASE_THROW);
    }
}

static uint8_t roll_die(void)
{
    return DIE_ROLLS[exe6_rand() & 0x0Fu];
}

static uint32_t multiply_attack(uint32_t attack, uint32_t multiplier)
{
    uint32_t base = attack & 0x7FFFu;
    uint32_t scaled = (base * multiplier) & 0x7FFFu;
    return (attack & 0xFFFF8000u) | scaled;
}

static void spawn_die(Exe6Obj *actor)
{
    struct ActorWork *work = (struct ActorWork *)actor->work;
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(actor);
    uint32_t target_x = (uint32_t)(
        (int32_t)actor->block_x + direction * (int32_t)DIE_DISTANCE
    );
    uint32_t target_y = actor->block_y;
    uint8_t roll = roll_die();
    Exe6ObjSpawnParameters parameters = {
        .variant = roll,
        .subvariant = 1,
    };
    Exe6Obj *die = exe6_shl_open(
        BN67_OBJ_ID(numberman_die_main),
        actor->x,
        actor->y,
        actor->z + (DIE_INITIAL_HEIGHT << 16),
        parameters
    );
    if (die == NULL) {
        work->die_active = 0;
        return;
    }

    uint64_t target_position = exe6_get_block_pos(target_x, target_y);
    int32_t target_position_x = (int32_t)(uint32_t)target_position;
    int32_t target_position_y = (int32_t)(uint32_t)(target_position >> 32);
    die->block_x = actor->block_x;
    die->block_y = actor->block_y;
    die->target_block_x = (uint8_t)target_x;
    die->target_block_y = (uint8_t)target_y;
    die->velocity_x = (target_position_x - actor->x) / (int32_t)FLIGHT_FRAMES;
    die->velocity_y = (target_position_y - actor->y) / (int32_t)FLIGHT_FRAMES;
    die->velocity_z = DIE_INITIAL_Z_VELOCITY;
    die->owner_word = actor->owner_word;
    die->attack = actor->attack;
    die->parent = actor;
    die->completion = &work->die_active;
    die->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    work->die_active = 1;
}

static void actor_throw(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 6);
        exe6_sound_req(0xB2);
        self->timer = THROW_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }

    spawn_die(self);
    set_phase(self, ACTOR_PHASE_WAIT_FOR_DIE);
}

static void actor_wait_for_die(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    if (work->die_active != 0) {
        return;
    }
    if (self->substate == UPDATE_STEP_INIT) {
        self->timer = AFTER_ROLL_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
        return;
    }
    if (decrement_timer(&self->timer) < 0) {
        set_phase(self, ACTOR_PHASE_EXIT);
    }
}

static void actor_exit(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 4);
        self->timer = EXIT_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void actor_update(Exe6Obj *self)
{
    if (self->phase == ACTOR_PHASE_APPEAR) {
        actor_appear(self);
    } else if (self->phase == ACTOR_PHASE_WAIT) {
        actor_wait(self);
    } else if (self->phase == ACTOR_PHASE_THROW) {
        actor_throw(self);
    } else if (self->phase == ACTOR_PHASE_WAIT_FOR_DIE) {
        actor_wait_for_die(self);
    } else {
        actor_exit(self);
    }
    exe6_battle_obj_char_move2();
}

static void actor_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(numberman_battle_sprite),
        BN67_SPRITE_ID(numberman_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    set_animation(self, 0);
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    set_phase(self, ACTOR_PHASE_APPEAR);
}

static void die_finish(Exe6Obj *self)
{
    actor_destroy(self);
}

static void die_begin_bounce(Exe6Obj *self)
{
    self->block_x = self->target_block_x;
    self->block_y = self->target_block_y;
    exe6_block_to_pos();
    self->z = 0;
    self->phase = DIE_PHASE_BOUNCE;
    self->substate = UPDATE_STEP_INIT;
}

static void die_leave_field(Exe6Obj *self)
{
    self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    self->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static bool object_is_on_die_panel(Exe6Obj *self, Exe6Obj *target)
{
    return target != NULL
        && (target->header_flags & EXE6_OBJ_FLAG_ACTIVE) != 0
        && target->block_x == self->block_x
        && target->block_y == self->block_y;
}

static bool die_lands_on_target(Exe6Obj *self)
{
    Exe6Runtime *runtime = exe6_runtime();
    if (runtime == NULL || runtime->battle_context == NULL) {
        return false;
    }
    Exe6BattleContext *battle = runtime->battle_context;
    uint32_t opposing_side = self->owner ^ 1u;
    for (size_t index = 0; index < 4; ++index) {
        if (object_is_on_die_panel(
                self,
                battle->active_units[opposing_side][index]
            )) {
            return true;
        }
    }
    return false;
}

static void die_flight(Exe6Obj *self)
{
    self->velocity_z += DIE_GRAVITY;
    self->z += self->velocity_z;
    self->x += self->velocity_x;
    self->y += self->velocity_y;
    exe6_pos_to_block();

    if (exe6_block_in_screen_check() == 0) {
        die_leave_field(self);
        return;
    }
    if (decrement_timer(&self->aux_timer) < 0) {
        if ((exe6_block_status_get(
                self->target_block_x,
                self->target_block_y
            ) & EXE6_BLOCK_FLAG_SOLID) != 0) {
            die_begin_bounce(self);
        } else {
            die_leave_field(self);
        }
    }
}

static void spawn_explosion_effects(Exe6Obj *self)
{
    for (int32_t y_offset = -1; y_offset <= 1; ++y_offset) {
        for (int32_t x_offset = -1; x_offset <= 1; ++x_offset) {
            uint32_t block_x = (uint32_t)(
                (int32_t)self->block_x + x_offset
            );
            uint32_t block_y = (uint32_t)(
                (int32_t)self->block_y + y_offset
            );
            if (exe6_block_in_screen_check_sub(block_x, block_y) == 0) {
                continue;
            }
            uint64_t position = exe6_get_block_pos(block_x, block_y);
            Exe6Obj *effect = exe6_set_efc00(
                0,
                (int32_t)(uint32_t)position,
                (int32_t)(uint32_t)(position >> 32),
                0,
                EXPLOSION_EFFECT
            );
            if (effect != NULL) {
                effect->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
            }
        }
    }
    exe6_sound_req(BN67_SONG_ID(numberman_explosion_song));
}

static void spawn_damage(Exe6Obj *self)
{
    Exe6Obj *damage = exe6_set_shl03_ev(
        self->block_x,
        self->block_y,
        self->owner_aux,
        0,
        DAMAGE_PROPERTIES,
        multiply_attack(self->attack, self->variant),
        3
    );
    if (damage != NULL) {
        damage->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    }
}

static void spawn_contact_damage(Exe6Obj *self)
{
    Exe6Obj *damage = exe6_set_shl03_ev(
        self->block_x,
        self->block_y,
        self->owner_aux,
        0,
        CONTACT_DAMAGE_PROPERTIES,
        self->attack,
        3
    );
    if (damage != NULL) {
        damage->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    }
}

static void spawn_contact_effect(Exe6Obj *self)
{
    Exe6Obj *effect = exe6_set_efc00(
        0,
        self->x,
        self->y,
        self->z,
        EXPLOSION_EFFECT
    );
    if (effect != NULL) {
        effect->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    }
    exe6_sound_req(0x70);
}

static void die_explode(Exe6Obj *self)
{
    spawn_damage(self);
    spawn_explosion_effects(self);
    exe6_sound_req(0x10F);
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    self->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static void die_contact_explode(Exe6Obj *self)
{
    spawn_contact_damage(self);
    spawn_contact_effect(self);
    self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    self->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static void die_bounce(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        self->velocity_z = DIE_BOUNCE_Z_VELOCITY;
        exe6_sound_req(0x115);
        self->substate = UPDATE_STEP_ACTIVE;
    }

    self->velocity_z += DIE_GRAVITY;
    self->z += self->velocity_z;
    if (self->z <= 0) {
        self->z = 0;
        self->phase = DIE_PHASE_ROLL;
        self->substate = UPDATE_STEP_INIT;
    }
}

static void die_show_roll(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, (uint8_t)(self->variant - 1u));
        self->timer = ROLL_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }

    if (decrement_timer(&self->timer) < 0) {
        exe6_obj_flash_reset();
        self->phase = DIE_PHASE_EXPLODE;
        self->substate = UPDATE_STEP_INIT;
        return;
    }
    exe6_obj_flash_reset();
    if (self->timer < ROLL_FLASH_FRAMES && (self->timer & 2u) != 0) {
        exe6_obj_flash_set();
    }
}

static void die_update(Exe6Obj *self)
{
    if (self->phase == DIE_PHASE_FLIGHT) {
        die_flight(self);
        if (self->state == EXE6_OBJECT_STATE_ACTIVE
            && self->z < DIE_CONTACT_HEIGHT
            && die_lands_on_target(self)) {
            self->phase = DIE_PHASE_CONTACT;
        }
    } else if (self->phase == DIE_PHASE_BOUNCE) {
        die_bounce(self);
    } else if (self->phase == DIE_PHASE_ROLL) {
        die_show_roll(self);
    } else if (self->phase == DIE_PHASE_EXPLODE) {
        die_explode(self);
    } else {
        die_contact_explode(self);
    }
    exe6_battle_obj_char_move2();
}

static void die_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(numberman_die_sprite),
        BN67_SPRITE_ID(numberman_die_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    set_animation(self, 6);
    exe6_obj_prio_set(DIE_PRIORITY);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->aux_timer = FLIGHT_FRAMES;
    self->phase = DIE_PHASE_FLIGHT;
    self->substate = UPDATE_STEP_ACTIVE;
    die_update(self);
}

BN67_SHELL(numberman_die_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        die_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        die_update(self);
    } else {
        die_finish(self);
    }
}

BN67_ENEMY(numberman_actor_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        actor_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        actor_update(self);
    } else {
        actor_destroy(self);
    }
}

BN67_SUMMON_ATTACK(0x0ef, numberman_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(numberman_actor_main), spawn_parameters
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
