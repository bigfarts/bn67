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
#define DUO_EFFECT_FLAGS                                                  \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK | \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#define DUO_ICON duo_icon
#define DUO_IMAGE duo_image
#define DUO_PALETTE duo_palette
#else
#define DUO_EFFECT_FLAGS                                               \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK)
#define DUO_ICON ((const uint8_t *)0x0872A350u)
#define DUO_IMAGE ((const uint8_t *)0x0871EAB0u)
#define DUO_PALETTE ((const uint8_t *)0x08723490u)
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
        .effect_flags = DUO_EFFECT_FLAGS,
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
    .icon = DUO_ICON,
    .image = DUO_IMAGE,
    .palette = DUO_PALETTE,
};

static const uint8_t DUO_ACTIVE_STATE = 4;
static const uint8_t DUO_DESTROY_STATE = 8;
static const uint8_t DUO_PANEL_PHASE = 0;
static const uint8_t DUO_ENTRY_PHASE = 4;
static const uint8_t DUO_WAIT_PHASE = 8;
static const uint8_t DUO_BARRAGE_PHASE = 12;
static const uint8_t DUO_EXIT_PHASE = 16;
static const uint8_t DUO_ACTOR_ANIMATION = 0;
static const uint8_t DUO_FIST_ANIMATION_LEFT = 19;
static const uint8_t DUO_FIST_ANIMATION_RIGHT = 18;
/* Keep every Duo battle sprite behind BN6's Custom gauge and HUD. */
static const uint8_t DUO_BATTLE_SPRITE_PRIORITY = 2;
static const uint8_t DUO_FIST_ATTEMPTS = 17;
static const uint8_t DUO_FIST_STEP_DELAY = 6;
static const uint16_t DUO_PANEL_FRAMES = 40;
static const uint16_t DUO_ENTRY_STEPS = 10;
static const uint16_t DUO_WAIT_FRAMES = 60;
static const uint16_t DUO_EXIT_LEAD_FRAMES = 30;
static const uint16_t DUO_EXIT_STEPS = 20;
static const int32_t DUO_ENTRY_X = 140 << 16;
static const int32_t DUO_ENTRY_VELOCITY = 8 << 16;
static const int32_t DUO_SCREEN_Y = 28 << 16;
static const uint16_t DUO_FIST_IMPACT_FRAMES = 7;
static const int32_t DUO_FIST_X_OFFSET = 134 << 16;
static const int32_t DUO_FIST_Y_OFFSET = 16 << 16;
static const int32_t DUO_FIST_START_HEIGHT = 120 << 16;
static const int32_t DUO_FIST_X_VELOCITY = 0x00118000;
static const int32_t DUO_FIST_Z_VELOCITY = 0x000E0000;
static const int32_t DUO_FIST_GRAVITY = 0x0000D200;
static const uint32_t DUO_SIDE_1_PANEL_FLAG = 0x00000020;
/* BN4 generic effect 6 moved to BN6 generic effect 5. */
static const uint8_t DUO_IMPACT_EFFECT = 5;

static const Exe6BlockDamageProperties DUO_DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CURRENT_BLOCK,
    .hit_effect = EXE6_HIT_EFFECT_NORMAL,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_15,
};

struct DuoWork {
    uint32_t target_mask;
    uint8_t primary_target;
    uint8_t last_target;
    uint8_t first_target_pending;
    uint8_t shots_remaining;
    uint8_t fists_spawned;
    uint8_t panels_saved;
    uint8_t saved_panel_active[6];
};

static void duo_set_animation(Exe6Obj *self, uint32_t animation)
{
    self->animation = (uint8_t)animation;
    self->palette = 0;
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
}

static uint32_t duo_target_bit(uint32_t block_x, uint32_t block_y)
{
    return 1u << ((block_y - 1u) * 6u + block_x - 1u);
}

static void duo_mark_target_area(
    struct DuoWork *work,
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
                required_flags |= DUO_SIDE_1_PANEL_FLAG;
            } else {
                excluded_flags = DUO_SIDE_1_PANEL_FLAG;
            }
            if (exe6_block_move_check(
                    (uint32_t)block_x,
                    (uint32_t)block_y,
                    required_flags,
                    excluded_flags
                ) == 0) {
                continue;
            }
            work->target_mask |= duo_target_bit(
                (uint32_t)block_x,
                (uint32_t)block_y
            );
        }
    }
}

static void duo_prepare_targets(Exe6Obj *self)
{
    struct DuoWork *work = (struct DuoWork *)self->work;
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
        duo_mark_target_area(
            work,
            self->owner,
            unit->block_x,
            unit->block_y
        );
    }

    work->last_target = 0;
    work->first_target_pending = 1;
}

static uint8_t duo_take_target(struct DuoWork *work)
{
    if (work->first_target_pending != 0) {
        work->first_target_pending = 0;
        work->last_target = work->primary_target;
        return work->primary_target;
    }

    uint32_t available = work->target_mask;
    if (work->last_target != 0) {
        available &= ~duo_target_bit(
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

static void duo_spawn_fist(Exe6Obj *actor, uint32_t index)
{
    struct DuoWork *work = (struct DuoWork *)actor->work;
    uint8_t target = duo_take_target(work);
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
        DUO_FIST_START_HEIGHT,
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

static void duo_apply_damage(Exe6Obj *fist)
{
    (void)exe6_set_shl03_ev(
        fist->block_x,
        fist->block_y,
        fist->owner_aux,
        (uint32_t)fist->z,
        DUO_DAMAGE_PROPERTIES,
        fist->attack,
        3
    );
}

static void duo_begin_barrage(Exe6Obj *self)
{
    struct DuoWork *work = (struct DuoWork *)self->work;
    duo_prepare_targets(self);
    work->fists_spawned = 0;
    work->shots_remaining = DUO_FIST_ATTEMPTS;
    self->aux_timer = 0;
    self->phase = DUO_BARRAGE_PHASE;
}

static void duo_begin_exit(Exe6Obj *self)
{
    self->timer = DUO_EXIT_LEAD_FRAMES;
    self->substate = 0;
    self->phase = DUO_EXIT_PHASE;
}

static void duo_set_panels(Exe6Obj *self, bool visible)
{
    struct DuoWork *work = (struct DuoWork *)self->work;
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

static void duo_break_obstacles(void)
{
    Exe6BattleContext *battle = exe6_runtime()->battle_context;
    for (size_t index = 0; index < EXE6_OBSTACLE_SLOT_COUNT; ++index) {
        Exe6Obj *obstacle = battle->obstacles[index];
        if (obstacle != NULL) {
            obstacle->hp = 0;
        }
    }
}

static void duo_begin_entry(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(duo_battle_sprite),
        BN67_SPRITE_ID(duo_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    duo_set_animation(self, DUO_ACTOR_ANIMATION);
    exe6_obj_prio_set(DUO_BATTLE_SPRITE_PRIORITY);
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x = -direction * DUO_ENTRY_X;
    self->y = DUO_SCREEN_Y;
    self->z = 0;
    self->velocity_x = direction * DUO_ENTRY_VELOCITY;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->timer = DUO_ENTRY_STEPS;
    self->phase = DUO_ENTRY_PHASE;
    exe6_sound_req(BN67_SONG_ID(duo_summon_song));
}

static void duo_actor_update(Exe6Obj *self)
{
    switch (self->phase) {
    case DUO_PANEL_PHASE:
        duo_set_panels(self, (self->timer & 4u) != 0);
        if (timer_expired(self)) {
            duo_begin_entry(self);
        }
        break;
    case DUO_ENTRY_PHASE:
        self->x += self->velocity_x;
        if (timer_expired(self)) {
            self->timer = DUO_WAIT_FRAMES;
            self->phase = DUO_WAIT_PHASE;
            exe6_camera_quake_set(1, 30);
            exe6_sound_req(BN67_SONG_ID(duo_arrival_song));
        }
        break;
    case DUO_WAIT_PHASE:
        if (timer_expired(self)) {
            duo_begin_barrage(self);
        }
        break;
    case DUO_BARRAGE_PHASE:
        {
            struct DuoWork *work = (struct DuoWork *)self->work;
            if (self->aux_timer != 0) {
                --self->aux_timer;
                break;
            }
            duo_spawn_fist(self, work->fists_spawned++);
            if (--work->shots_remaining == 0) {
                duo_begin_exit(self);
            } else {
                self->aux_timer = DUO_FIST_STEP_DELAY;
            }
        }
        break;
    default:
        if (self->substate == 0) {
            if (!timer_expired(self)) {
                break;
            }
            exe6_camera_quake_set(1, 20);
            self->timer = DUO_EXIT_STEPS;
            self->substate = 4;
        }
        self->x -= self->velocity_x;
        if (timer_expired(self)) {
            self->state_word = DUO_DESTROY_STATE;
        }
        break;
    }
}

static void duo_actor_init(Exe6Obj *self)
{
    self->block_x = (uint8_t)(self->owner * 7u);
    self->block_y = 2;
    self->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    self->timer = DUO_PANEL_FRAMES;
    ((struct DuoWork *)self->work)->panels_saved = 0;
    self->state_word = DUO_ACTIVE_STATE;
    self->phase = DUO_PANEL_PHASE;
    duo_actor_update(self);
}

static void duo_actor_destroy(Exe6Obj *self)
{
    duo_set_panels(self, true);
    if (self->completion != NULL) {
        *self->completion = 0;
    }
    exe6_obj_move_delete();
}

BN67_ENEMY(duo_actor_main)
{
    if (self->state == 0) {
        duo_actor_init(self);
    } else if (self->state == DUO_ACTIVE_STATE) {
        duo_actor_update(self);
    } else {
        duo_actor_destroy(self);
        return;
    }
    if (self->phase != DUO_PANEL_PHASE) {
        exe6_battle_obj_char_move2();
    }
}

static void duo_fist_init(Exe6Obj *self)
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
        ? DUO_FIST_ANIMATION_LEFT
        : DUO_FIST_ANIMATION_RIGHT;
    duo_set_animation(self, animation);
    exe6_obj_prio_set(DUO_BATTLE_SPRITE_PRIORITY);
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x -= direction * DUO_FIST_X_OFFSET;
    self->y += DUO_FIST_Y_OFFSET;
    self->velocity_x = direction * DUO_FIST_X_VELOCITY;
    self->velocity_z = DUO_FIST_Z_VELOCITY;
    self->phase = 0;
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = DUO_ACTIVE_STATE;
    exe6_sound_req(BN67_SONG_ID(duo_fist_song));
}

static void duo_fist_update(Exe6Obj *self)
{
    if (self->phase == 0) {
        self->x += self->velocity_x;
        self->velocity_z += DUO_FIST_GRAVITY;
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
            exe6_obj_spawn_with_variant(DUO_IMPACT_EFFECT)
        );
        if (impact != NULL) {
            impact->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
        }
        duo_apply_damage(self);
        uint32_t block_flags = exe6_block_status_get(
            self->block_x,
            self->block_y
        );
        if (self->subvariant != 0
            || ((block_flags & EXE6_BLOCK_FLAG_CRACKED) == 0
                && (exe6_rand() & 1u) == 0)) {
            exe6_block_crack_set(self->block_x, self->block_y);
        }
        self->timer = DUO_FIST_IMPACT_FRAMES;
        self->phase = 4;
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
    if (self->state == 0) {
        duo_fist_init(self);
        duo_fist_update(self);
    } else if (self->state == DUO_ACTIVE_STATE) {
        duo_fist_update(self);
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

BN67_SUMMON_ATTACK(0x133, duo_attack_main)
{
    duo_break_obstacles();
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
