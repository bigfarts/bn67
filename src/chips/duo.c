#include "common.h"
#include "runtime.h"

BN67_SPRITE(duo_battle_sprite, "build/duo-battle-sprite.bin");
BN67_SONG(
    duo_summon_song,
    BN67_PCM(
        duo_summon,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xEB,0x37,0x7F,0x9C,0x81,0xB1\n",
        "build/duo-fist-sound-sample.bin"
    )
);
BN67_SONG(
    duo_arrival_song,
    BN67_PCM(
        duo_arrival,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x60,0xEF,0x3C,0x7F,0xA0,0x83,0xB1\n",
        "build/duo-arrival-sound-sample.bin"
    )
);
BN67_SONG(
    duo_fist_song,
    BN67_PCM(
        duo_fist,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xED,0x3C,0x7F,0x9E,0xB1\n",
        "build/duo-fist-sound-sample.bin"
    )
);

#if FALZAR
BN67_INCBIN(duo_icon, "build/duo-icon.bin");
BN67_INCBIN(duo_image, "build/duo-image.bin");
BN67_INCBIN(duo_palette, "build/duo-palette.bin");
#endif

#if FALZAR
#define EFFECT_FLAGS                                                      \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK | \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#define ICON duo_icon
#define IMAGE duo_image
#define PALETTE duo_palette
#else
#define EFFECT_FLAGS                                                  \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK)
#define ICON ((const uint8_t *)0x0872A350u)
#define IMAGE ((const uint8_t *)0x0871EAB0u)
#define PALETTE ((const uint8_t *)0x08723490u)
#endif

/* BN4 Blue Moon array chip 0x137, installed over BN6's MetrKnuk 0x133. */
BN67_CHIP_RECORD(0x133) {
    .codes = {
        EXE6_CHIP_CODE_D,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_GIGA,
    .mb = 99,
    .behavior = {
        .effect_flags = EFFECT_FLAGS,
        .counter_settings = 0x94,
        .family = BN67_ATTACK_FAMILY(duo_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(duo_attack_main),
        .dark_soul_usage = 0x05,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    /* Preserve MetrKnuk's third Giga-library position and ordering. */
    .library_number = 0x02,
    .library_flags = 0x14,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0x013B,
    .power = 200,
    .library_sort_order = 0x0133,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE,
};

enum ActorPhase {
    ACTOR_PHASE_PANEL,
    ACTOR_PHASE_ENTRY = 4,
    ACTOR_PHASE_WAIT = 8,
    ACTOR_PHASE_BARRAGE = 12,
    ACTOR_PHASE_EXIT = 16,
};

enum ExitStep {
    EXIT_STEP_LEAD_IN,
    EXIT_STEP_MOVING = 4,
};

enum FistPhase {
    FIST_PHASE_FALL,
    FIST_PHASE_IMPACT = 4,
};

static const uint8_t ACTOR_ANIMATION = 0;
static const uint8_t FIST_ANIMATION_LEFT = 19;
static const uint8_t FIST_ANIMATION_RIGHT = 18;
static const uint8_t FIST_ATTEMPTS = 17;
static const uint8_t FIST_STEP_DELAY = 6;
static const uint16_t PANEL_FRAMES = 40;
static const uint16_t ENTRY_STEPS = 10;
static const uint16_t WAIT_FRAMES = 60;
static const uint16_t EXIT_LEAD_FRAMES = 30;
static const uint16_t EXIT_STEPS = 20;
static const int32_t ENTRY_X = 140 << 16;
static const int32_t ENTRY_VELOCITY = 8 << 16;
static const int32_t SCREEN_Y = 28 << 16;
static const uint16_t FIST_IMPACT_FRAMES = 7;
static const int32_t FIST_X_OFFSET = 134 << 16;
static const int32_t FIST_Y_OFFSET = 16 << 16;
static const int32_t FIST_START_HEIGHT = 120 << 16;
static const int32_t FIST_X_VELOCITY = 0x00118000;
static const int32_t FIST_Z_VELOCITY = 0x000E0000;
static const int32_t FIST_GRAVITY = 0x0000D200;
static const uint32_t SIDE_1_PANEL_FLAG = 0x00000020;
/* BN4 generic effect 6 moved to BN6 generic effect 5. */
static const uint8_t IMPACT_EFFECT = 5;

static const Exe6BlockDamageProperties DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CURRENT_BLOCK,
    .hit_effect = EXE6_HIT_EFFECT_NORMAL,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_15,
};

struct ActorWork {
    uint32_t target_mask;
    uint8_t primary_target;
    uint8_t last_target;
    uint8_t first_target_pending;
    uint8_t shots_remaining;
    uint8_t fists_spawned;
    uint8_t panels_saved;
    uint8_t saved_panel_active[6];
};

static uint32_t target_bit(uint32_t block_x, uint32_t block_y)
{
    return 1u << ((block_y - 1u) * 6u + block_x - 1u);
}

static void mark_target_area(
    struct ActorWork *work,
    uint32_t owner,
    uint32_t center_x,
    uint32_t center_y
)
{
    for (int32_t y_offset = -1; y_offset <= 1; ++y_offset) {
        for (int32_t x_offset = -1; x_offset <= 1; ++x_offset) {
            int32_t block_x = (int32_t)center_x + x_offset;
            int32_t block_y = (int32_t)center_y + y_offset;
            if (block_x < 1 || block_x > 6 || block_y < 1 || block_y > 3) {
                continue;
            }
            uint32_t required_flags = EXE6_BLOCK_FLAG_VALID;
            uint32_t excluded_flags = 0;
            if (owner == 0) {
                required_flags |= SIDE_1_PANEL_FLAG;
            } else {
                excluded_flags = SIDE_1_PANEL_FLAG;
            }
            if (exe6_block_move_check(
                    (uint32_t)block_x,
                    (uint32_t)block_y,
                    required_flags,
                    excluded_flags
                ) == 0) {
                continue;
            }
            work->target_mask |= target_bit(
                (uint32_t)block_x,
                (uint32_t)block_y
            );
        }
    }
}

static void prepare_targets(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    Exe6Obj *const *units =
        exe6_runtime()->battle_context->battle_units[self->owner ^ 1u];
    work->target_mask = 0;
    work->primary_target = 0;
    size_t target_count = 0;
    for (size_t index = 0; index < 4; ++index) {
        Exe6Obj *unit = units[index];
        if (unit == NULL || (unit->header_flags & EXE6_OBJ_FLAG_ACTIVE) == 0) {
            continue;
        }
        ++target_count;
    }
    size_t primary_index = target_count == 0 ? 0 : exe6_rand() % target_count;
    size_t live_index = 0;
    for (size_t index = 0; index < 4; ++index) {
        Exe6Obj *unit = units[index];
        if (unit == NULL || (unit->header_flags & EXE6_OBJ_FLAG_ACTIVE) == 0) {
            continue;
        }
        if (live_index++ == primary_index) {
            work->primary_target = (uint8_t)(
                unit->block_x | (unit->block_y << 4)
            );
        }
        mark_target_area(
            work,
            self->owner,
            unit->block_x,
            unit->block_y
        );
    }

    work->last_target = 0;
    work->first_target_pending = 1;
}

static uint8_t take_target(struct ActorWork *work)
{
    if (work->first_target_pending != 0) {
        work->first_target_pending = 0;
        work->last_target = work->primary_target;
        return work->primary_target;
    }

    uint32_t available = work->target_mask;
    if (work->last_target != 0) {
        available &= ~target_bit(
            work->last_target & 0x0Fu,
            work->last_target >> 4
        );
    }
    uint32_t count = 0;
    for (uint32_t bits = available; bits != 0; bits >>= 1) {
        count += bits & 1u;
    }
    if (count == 0) {
        return 0;
    }

    uint32_t selected = exe6_rand() % count;
    for (uint32_t index = 0; index < 18; ++index) {
        uint32_t bit = 1u << index;
        if ((available & bit) == 0) {
            continue;
        }
        if (selected-- != 0) {
            continue;
        }
        work->last_target = (uint8_t)(
            (index % 6u + 1u) | ((index / 6u + 1u) << 4)
        );
        return work->last_target;
    }
    return 0;
}

static void spawn_fist(Exe6Obj *actor, uint32_t index)
{
    struct ActorWork *work = (struct ActorWork *)actor->work;
    uint8_t target = take_target(work);
    if (target == 0) {
        return;
    }
    uint32_t block_x = target & 0x0Fu;
    uint32_t block_y = target >> 4;
    Exe6ObjSpawnParameters parameters = {
        /* BN4 alternates variants 0/1, which select animations 19/18. */
        .variant = (uint8_t)(index & 1u),
        /* Duo's native fist opener forces subvariant 4. */
        .subvariant = 4,
    };
    Exe6Obj *fist = exe6_efc_open_at(
        BN67_OBJ_ID(duo_fist_main),
        0,
        0,
        FIST_START_HEIGHT,
        parameters
    );
    if (fist == NULL) {
        return;
    }
    fist->block_x = (uint8_t)block_x;
    fist->block_y = (uint8_t)block_y;
    fist->parameter = actor->parameter;
    fist->owner_word = actor->owner_word;
    fist->attack = actor->attack;
    fist->parent = actor;
    fist->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void apply_damage(Exe6Obj *fist)
{
    (void)exe6_set_shl03_ev(
        fist->block_x,
        fist->block_y,
        fist->owner_aux,
        (uint32_t)fist->z,
        DAMAGE_PROPERTIES,
        fist->attack,
        3
    );
}

static void begin_barrage(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    prepare_targets(self);
    work->fists_spawned = 0;
    work->shots_remaining = FIST_ATTEMPTS;
    self->aux_timer = 0;
    self->phase = ACTOR_PHASE_BARRAGE;
}

static void begin_exit(Exe6Obj *self)
{
    self->timer = EXIT_LEAD_FRAMES;
    self->substate = EXIT_STEP_LEAD_IN;
    self->phase = ACTOR_PHASE_EXIT;
}

static void set_panels(Exe6Obj *self, bool visible)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    uint32_t first_x = self->owner * 4u + 1u;
    size_t saved_index = 0;
    for (uint32_t block_x = first_x; block_x < first_x + 2u; ++block_x) {
        for (uint32_t block_y = 1; block_y <= 3; ++block_y) {
            Exe6Block *block = exe6_block_at(block_x, block_y);
            if (work->panels_saved == 0) {
                work->saved_panel_active[saved_index] = block->active;
            }
            block->active = visible
                ? work->saved_panel_active[saved_index]
                : 0;
            ++saved_index;
        }
    }
    work->panels_saved = 1;
}

static void break_obstacles(void)
{
    Exe6BattleContext *battle = exe6_runtime()->battle_context;
    for (size_t index = 0; index < EXE6_OBSTACLE_SLOT_COUNT; ++index) {
        Exe6Obj *obstacle = battle->obstacles[index];
        if (obstacle != NULL) {
            obstacle->hp = 0;
        }
    }
}

static bool entrance_has_player(uint32_t owner)
{
    /* BN4 area pattern 17 covers the user's rear two columns, all three rows. */
    uint32_t first_x = owner * 4u + 1u;
    uint32_t player_flag = owner == 0
        ? EXE6_BLOCK_FLAG_SIDE_1_HIT
        : EXE6_BLOCK_FLAG_SIDE_0_HIT;
    for (uint32_t block_x = first_x; block_x < first_x + 2u; ++block_x) {
        for (uint32_t block_y = 1; block_y <= 3; ++block_y) {
            if (exe6_block_move_check(
                    block_x,
                    block_y,
                    player_flag,
                    0
                ) != 0) {
                return true;
            }
        }
    }
    return false;
}

static void begin_entry(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(duo_battle_sprite),
        BN67_SPRITE_ID(duo_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    set_animation_immediate(self, ACTOR_ANIMATION);
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x = -direction * ENTRY_X;
    self->y = SCREEN_Y;
    self->z = 0;
    self->velocity_x = direction * ENTRY_VELOCITY;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->timer = ENTRY_STEPS;
    self->phase = ACTOR_PHASE_ENTRY;
    exe6_sound_req(BN67_SONG_ID(duo_summon_song));
}

static void actor_update(Exe6Obj *self)
{
    switch (self->phase) {
    case ACTOR_PHASE_PANEL:
        set_panels(self, (self->timer & 4u) != 0);
        if (decrement_timer(&self->timer) < 0) {
            begin_entry(self);
        }
        break;
    case ACTOR_PHASE_ENTRY:
        self->x += self->velocity_x;
        if (decrement_timer(&self->timer) < 0) {
            self->timer = WAIT_FRAMES;
            self->phase = ACTOR_PHASE_WAIT;
            exe6_camera_quake_set(1, 30);
            exe6_sound_req(BN67_SONG_ID(duo_arrival_song));
        }
        break;
    case ACTOR_PHASE_WAIT:
        if (decrement_timer(&self->timer) < 0) {
            begin_barrage(self);
        }
        break;
    case ACTOR_PHASE_BARRAGE:
        {
            struct ActorWork *work = (struct ActorWork *)self->work;
            if (self->aux_timer != 0) {
                --self->aux_timer;
                break;
            }
            spawn_fist(self, work->fists_spawned++);
            if (--work->shots_remaining == 0) {
                begin_exit(self);
            } else {
                self->aux_timer = FIST_STEP_DELAY;
            }
        }
        break;
    default:
        if (self->substate == EXIT_STEP_LEAD_IN) {
            if (decrement_timer(&self->timer) >= 0) {
                break;
            }
            exe6_camera_quake_set(1, 20);
            self->timer = EXIT_STEPS;
            self->substate = EXIT_STEP_MOVING;
        }
        self->x -= self->velocity_x;
        if (decrement_timer(&self->timer) < 0) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
        }
        break;
    }
}

static void actor_init(Exe6Obj *self)
{
    self->block_x = (uint8_t)(self->owner * 7u);
    self->block_y = 2;
    self->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    self->timer = PANEL_FRAMES;
    ((struct ActorWork *)self->work)->panels_saved = 0;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->phase = ACTOR_PHASE_PANEL;
    actor_update(self);
}

static void destroy_actor(Exe6Obj *self)
{
    set_panels(self, true);
    if (self->completion != NULL) {
        *self->completion = 0;
    }
    exe6_obj_move_delete();
}

BN67_ENEMY(duo_actor_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        actor_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        actor_update(self);
    } else {
        destroy_actor(self);
        return;
    }
    if (self->phase != ACTOR_PHASE_PANEL) {
        exe6_battle_obj_char_move2();
    }
}

static void fist_init(Exe6Obj *self)
{
    exe6_block_to_pos();
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(duo_battle_sprite),
        BN67_SPRITE_ID(duo_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    uint32_t animation = self->variant == 0
        ? FIST_ANIMATION_LEFT
        : FIST_ANIMATION_RIGHT;
    set_animation_immediate(self, animation);
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x -= direction * FIST_X_OFFSET;
    self->y += FIST_Y_OFFSET;
    self->velocity_x = direction * FIST_X_VELOCITY;
    self->velocity_z = FIST_Z_VELOCITY;
    self->phase = FIST_PHASE_FALL;
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    exe6_sound_req(BN67_SONG_ID(duo_fist_song));
}

static void fist_update(Exe6Obj *self)
{
    if (self->phase == FIST_PHASE_FALL) {
        self->x += self->velocity_x;
        self->velocity_z += FIST_GRAVITY;
        int32_t next_z = self->z - self->velocity_z;
        if (next_z >= 0) {
            self->z = next_z;
            return;
        }
        uint32_t random = exe6_rand();
        int32_t impact_x = self->x - (3 << 16)
            + ((int32_t)(random & 3u) - 1) * (1 << 16);
        int32_t impact_z = self->z
            + ((int32_t)((random >> 16) & 3u) - 1) * (1 << 16);
        Exe6Obj *impact = exe6_efc_open_at(
            0,
            impact_x,
            self->y,
            impact_z,
            exe6_obj_spawn_with_variant(IMPACT_EFFECT)
        );
        if (impact != NULL) {
            impact->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
        }
        apply_damage(self);
        uint32_t block_flags = exe6_block_status_get(
            self->block_x,
            self->block_y
        );
        if (self->subvariant != 0
            || ((block_flags & EXE6_BLOCK_FLAG_CRACKED) == 0
                && (exe6_rand() & 1u) == 0)) {
            exe6_block_crack_set(self->block_x, self->block_y);
        }
        self->timer = FIST_IMPACT_FRAMES;
        self->phase = FIST_PHASE_IMPACT;
        if ((block_flags & EXE6_BLOCK_FLAG_SOLID) != 0) {
            exe6_camera_quake_set(1, 5);
        }
        self->z = 0;
        return;
    }

    if (self->timer <= 4) {
        self->x -= self->velocity_x;
        self->z += self->velocity_z;
    }
    if (self->timer != 0) {
        --self->timer;
    }
    if (self->timer == 0) {
        exe6_obj_move_delete();
    }
}

BN67_EFFECT(duo_fist_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        fist_init(self);
        fist_update(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        fist_update(self);
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

BN67_SUMMON_ATTACK(0x133, duo_attack_main)
{
    break_obstacles();
    if (entrance_has_player(owner->owner)) {
        return;
    }
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(duo_actor_main), spawn_parameters
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
