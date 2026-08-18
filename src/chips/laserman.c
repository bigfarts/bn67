#include "common.h"
#include "runtime.h"

BN67_USE_SONG(common_navi_summon_song);
BN67_SPRITE(laserman_battle_sprite, "build/laserman_battle_sprite.bin");

BN67_INCBIN(laserman_image, "build/laserman_image.bin");
BN67_INCBIN(laserman_palette_base, "build/laserman_pal_base.bin");
BN67_ASM_RESOURCE(
    laserman_palette_ex,
    ".incbin \"build/laserman_pal_base.bin\",0,0x02\n"
    ".short 0x00C0,0x0180,0x0280,0x03E0,0x0060\n"
    ".incbin \"build/laserman_pal_base.bin\",0x0C,0x14\n"
);
BN67_INCBIN(laserman_palette_sp, "build/laserman_pal_sp.bin");
BN67_SONG(
    laserman_fire_song,
    BN67_PCM(
        laserman_fire,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xF6,0x2F,0x7F,0xA2,0x81,0xBE\n"
        ".byte 0x60,0x84,0x40,0x84,0x20,0x84,0x10,0x84,0xB1\n",
        "build/laserman_fire_sample.bin"
    )
);

#if FALZAR
#define ICON ((const uint8_t *)0x0872BE14u)
#define LIBRARY_FLAGS 0x01
#define LIBRARY_NUMBER_BASE 0x03
#define LIBRARY_SORT_BASE 0x00F2
#else
#define ICON ((const uint8_t *)0x08729D50u)
#define LIBRARY_FLAGS 0x00
#define LIBRARY_NUMBER_BASE 0x07
#define LIBRARY_SORT_BASE 0x00E3
#endif

BN67_CHIP_RECORD(0x0e3) {
    .codes = {
        EXE6_CHIP_CODE_L,
        EXE6_CHIP_CODE_ASTERISK,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 2,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 60,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(laserman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(laserman_attack_main),
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
    .power = 100,
    .library_sort_order = LIBRARY_SORT_BASE,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = laserman_image,
    .palette = laserman_palette_base,
};

BN67_CHIP_RECORD(0x0e4) {
    .codes = {
        EXE6_CHIP_CODE_L,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 3,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 80,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(laserman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(laserman_attack_main),
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
    .power = 150,
    .library_sort_order = LIBRARY_SORT_BASE + 1,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = laserman_image,
    .palette = laserman_palette_ex,
};

BN67_CHIP_RECORD(0x0e5) {
    .codes = {
        EXE6_CHIP_CODE_L,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_MEGA,
    .mb = 80,
    .behavior = {
        .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |
                        EXE6_CHIP_EFFECT_FLAG_ATTACK |
                        EXE6_CHIP_EFFECT_FLAG_NAVI |
                        EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,
        .counter_settings = 0x8A,
        .family = BN67_ATTACK_FAMILY(laserman_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(laserman_attack_main),
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
    .power = 200,
    .library_sort_order = LIBRARY_SORT_BASE + 2,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = laserman_image,
    .palette = laserman_palette_sp,
};

static const uint8_t HIT_VISUAL = 25;
static const uint16_t WAIT_FRAMES = 20;
static const uint16_t RAISE_FRAMES = 30;
static const uint16_t LASER_FRAMES = 80;
static const uint16_t BEAM_FRAMES = 60;
static const uint8_t HIT_CONFIRM_FRAMES = 8;
static const Exe6HitType HIT_SELECTOR =
    EXE6_HIT_TYPE_STANDARD_TARGET;

enum ActorPhase {
    ACTOR_PHASE_WAIT,
    ACTOR_PHASE_ATTACK = 4,
};

enum ActorAttackStep {
    ACTOR_ATTACK_STEP_INIT,
    ACTOR_ATTACK_STEP_RAISE = 4,
    ACTOR_ATTACK_STEP_FIRE = 8,
    ACTOR_ATTACK_STEP_LOWER = 12,
};

enum BeamPhase {
    BEAM_PHASE_FORM,
    BEAM_PHASE_ACTIVE = 4,
    BEAM_PHASE_DISAPPEAR = 8,
};

enum HitPhase {
    HIT_PHASE_COLLIDE,
    HIT_PHASE_CONFIRM_DAMAGE = 4,
};

enum ChargeShotRestoreState {
    CHARGE_SHOT_RESTORE_INACTIVE,
    CHARGE_SHOT_RESTORE_BASE,
    CHARGE_SHOT_RESTORE_CROSS,
};

struct HitWork {
    uint32_t reserved[5];
    uint32_t command_stream;             // +0x74
    uint16_t target_hp_before;           // +0x78
    uint8_t confirm_timer;               // +0x7A
};

_Static_assert(
    offsetof(struct HitWork, command_stream) == 0x14,
    "LaserMan hit work layout"
);

_Static_assert(
    sizeof(struct HitWork) <= sizeof(((Exe6Obj *)0)->work),
    "LaserMan hit work size"
);

enum CommandEffect {
    COMMAND_EFFECT_DISABLE_SUPER_ARMOR = 1,
    COMMAND_EFFECT_DISABLE_FLOAT_SHOES = 2,
    COMMAND_EFFECT_DISABLE_AIR_SHOES = 3,
    COMMAND_EFFECT_DISABLE_UNDERSHIRT = 4,
    COMMAND_EFFECT_RESET_ATTACK = 5,
    COMMAND_EFFECT_RESET_RAPID = 6,
    COMMAND_EFFECT_RESET_CHARGE = 7,
    COMMAND_EFFECT_DISABLE_STATUS_GUARD = 8,
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
    COMMAND_EFFECT_DISABLE_STATUS_GUARD,
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

/* Native player-runtime power-attack IDs for transformation IDs 1-10:
 * Heat, Elec, Slash, Erase, Charge, Aqua, Tomahawk, Tengu, Ground, Dust. */
static const uint8_t CROSS_POWER_ATTACKS[] = {
    1, 6, 11, 18, 20, 39, 12, 22, 15, 25, 40,
};

/* Native B+Left attacks supplied by the standard Crosses. */
static const uint8_t CROSS_B_LEFTS[] = {
    UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX, UINT8_MAX,
    UINT8_MAX, UINT8_MAX, 0x10, UINT8_MAX, 0x2A,
};

/* Down-targeted live flags that are innate to each standard Cross. TenguCross
 * supplies AirShoes and GroundCross supplies SuperArmor. */
static const uint32_t CROSS_INNATE_STATUS_FLAGS[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    EXE6_HIT_STATUS_FLAG_AIR_SHOES,
    EXE6_HIT_STATUS_FLAG_SUPER_ARMOR,
    0,
};

static uint8_t target_active_cross(uint32_t side)
{
    const Exe6NaviStatusWork *status =
        exe6_navi_status_work_adrs_get(side);
    if (status == NULL
        || status->active_form == 0
        || status->active_form > EXE6_STANDARD_CROSS_COUNT) {
        return 0;
    }
    return status->active_form;
}

static void read_command(Exe6Obj *actor)
{
    const uint8_t *input = exe6_battle_key_work_adrs_get(actor->owner);
    uint16_t keys = *(const uint16_t *)(input + 2);
    if ((keys & EXE6_KEY_UP) != 0) {
        actor->subvariant = 1;
    } else if ((keys & EXE6_KEY_DOWN) != 0) {
        actor->subvariant = 2;
    } else if ((keys & EXE6_KEY_RIGHT) != 0) {
        actor->subvariant = 3;
    } else if ((keys & EXE6_KEY_LEFT) != 0) {
        actor->subvariant = 4;
    }
}

static void spawn_laser(Exe6Obj *actor)
{
    Exe6Obj *beam = exe6_em_open(
        BN67_OBJ_ID(laserman_beam_main),
        exe6_obj_spawn_with_variant(actor->subvariant)
    );
    if (beam == NULL) {
        return;
    }
    beam->block_x = (uint8_t)(
        (int32_t)actor->block_x + exe6_calc_pl_em_spd()
    );
    beam->block_y = actor->block_y;
    beam->parameter = actor->parameter;
    beam->owner_word = actor->owner_word;
    beam->attack = actor->attack;
    beam->parent = actor;
    beam->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void actor_attack(Exe6Obj *self)
{
    if (self->substate == ACTOR_ATTACK_STEP_INIT) {
        set_animation_immediate(self, 2);
        self->timer = RAISE_FRAMES;
        self->substate = ACTOR_ATTACK_STEP_RAISE;
        return;
    }
    if (self->substate == ACTOR_ATTACK_STEP_RAISE) {
        read_command(self);
        if (decrement_timer(&self->timer) < 0) {
            set_animation_immediate(self, 3);
            spawn_laser(self);
            exe6_sound_req(BN67_SONG_ID(laserman_fire_song));
            self->timer = LASER_FRAMES;
            self->substate = ACTOR_ATTACK_STEP_FIRE;
        }
        return;
    }
    if (self->substate == ACTOR_ATTACK_STEP_FIRE) {
        if (decrement_timer(&self->timer) < 0) {
            set_animation_immediate(self, 4);
            self->substate = ACTOR_ATTACK_STEP_LOWER;
        }
        return;
    }
    if ((exe6_obj_seq_info_get()
            & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void actor_update(Exe6Obj *self)
{
    if (self->phase == ACTOR_PHASE_WAIT) {
        if (decrement_timer(&self->timer) >= 0) {
            return;
        }
        if (exe6_block_move_check(
                self->block_x,
                self->block_y,
                EXE6_BLOCK_FLAG_SOLID,
                0
            ) == 0) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
        self->phase = ACTOR_PHASE_ATTACK;
        self->substate = ACTOR_ATTACK_STEP_INIT;
        return;
    }
    actor_attack(self);
}

static void actor_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(laserman_battle_sprite),
        BN67_SPRITE_ID(laserman_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    set_animation_immediate(self, 0);
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->phase = ACTOR_PHASE_WAIT;
    self->substate = ACTOR_ATTACK_STEP_INIT;
    self->timer = WAIT_FRAMES;
    exe6_sound_req(BN67_SONG_ID(common_navi_summon_song));
}

static void spawn_hit(
    Exe6Obj *beam,
    uint32_t block_x,
    uint32_t block_y,
    uint16_t command
)
{
    Exe6Obj *hit = exe6_shl_open(
        BN67_OBJ_ID(laserman_hit_main),
        0,
        0,
        0,
        exe6_obj_spawn_with_variant(HIT_VISUAL)
    );
    if (hit == NULL) {
        return;
    }
    struct HitWork *work = (struct HitWork *)hit->work;
    hit->block_x = (uint8_t)block_x;
    hit->block_y = (uint8_t)block_y;
    hit->animation_word = command;
    work->command_stream = beam->variant;
    hit->owner_word = beam->owner_word;
    hit->attack = command == COMMAND_MARKER ? beam->attack : 0;
    if (command != COMMAND_MARKER) {
        hit->phase_timer_low = CHARGE_SHOT_RESTORE_INACTIVE;
    }
    hit->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void spawn_row_event(Exe6Obj *beam, uint16_t command)
{
    for (uint32_t block_x = 1; block_x <= 6; ++block_x) {
        spawn_hit(beam, block_x, beam->block_y, command);
    }
}

static void beam_command_tick(Exe6Obj *self)
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

static void beam_update(Exe6Obj *self)
{
    if (self->phase == BEAM_PHASE_FORM) {
        if ((exe6_obj_seq_info_get()
                & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
            set_animation_immediate(self, 18);
            self->animation_state = 0;
            self->removal_state = 0;
            self->timer = BEAM_FRAMES;
            self->phase = BEAM_PHASE_ACTIVE;
        }
        return;
    }
    if (self->phase == BEAM_PHASE_ACTIVE) {
        beam_command_tick(self);
        if (decrement_timer(&self->timer) < 0) {
            set_animation_immediate(self, 19);
            self->phase = BEAM_PHASE_DISAPPEAR;
        }
        return;
    }
    if ((exe6_obj_seq_info_get()
            & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void beam_init(Exe6Obj *self)
{
    exe6_block_to_pos();
    self->z = 0;
    self->x += exe6_calc_pl_em_spd() * (0x40 << 16);
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(laserman_battle_sprite),
        BN67_SPRITE_ID(laserman_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    set_animation_immediate(self, 17);
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(
        self->variant == 0 || self->variant == 2 ? 0 : 10
    );
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    exe6_obj_col_efc_set(BEAM_SCALES[self->variant]);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->phase = BEAM_PHASE_FORM;
}

static void apply_command_effect(Exe6Obj *hit, uint16_t command)
{
    uint32_t target_side = hit->owner ^ 1u;
    enum CommandEffect effect = (enum CommandEffect)(uint8_t)command;

    switch (effect) {
    case COMMAND_EFFECT_DISABLE_SUPER_ARMOR:
        exe6_navi_status_set(target_side, 0x23, 0);
        break;
    case COMMAND_EFFECT_DISABLE_FLOAT_SHOES:
        exe6_navi_status_set(target_side, 0x1B, 0);
        break;
    case COMMAND_EFFECT_DISABLE_AIR_SHOES:
        exe6_navi_status_set(target_side, 0x1C, 0);
        break;
    case COMMAND_EFFECT_DISABLE_UNDERSHIRT:
        exe6_navi_status_set(target_side, 0x1D, 0);
        break;
    case COMMAND_EFFECT_DISABLE_STATUS_GUARD:
        exe6_navi_status_set(target_side, 0x52, 0);
        break;
    case COMMAND_EFFECT_RESET_ATTACK:
        exe6_navi_status_set(target_side, 0x01, 0);
        break;
    case COMMAND_EFFECT_RESET_RAPID:
        exe6_navi_status_set(target_side, 0x02, 0);
        break;
    case COMMAND_EFFECT_RESET_CHARGE:
        exe6_navi_status_set(target_side, 0x03, 0);
        break;
    case COMMAND_EFFECT_DISABLE_B_LEFT:
        exe6_navi_status_set(target_side, 0x07, UINT8_MAX);
        break;
    case COMMAND_EFFECT_RESTORE_CHARGE_SHOT: {
        uint32_t current = exe6_navi_status_get(target_side, 0x05);
        Exe6Obj *player = exe6_get_navi_adrs(target_side);
        if (player != NULL) {
            Exe6PlayerRuntime *runtime = player->runtime_data;
            if (target_active_cross(target_side) != 0) {
                hit->phase_timer_low = CHARGE_SHOT_RESTORE_CROSS;
            } else if (runtime->active_power_attack == current) {
                hit->phase_timer_low = CHARGE_SHOT_RESTORE_BASE;
            }
        }
        exe6_navi_status_set(target_side, 0x04, 0);
        exe6_navi_status_set(target_side, 0x05, 1);
        break;
    }
    case COMMAND_EFFECT_REDUCE_CUSTOM:
        {
            uint32_t value = exe6_navi_status_get(target_side, 0x0a);
            if (value > 2) {
                --value;
            }
            exe6_navi_status_set(target_side, 0x0a, value);
            break;
        }
    default:
        return;
    }
}

static void refresh_target_player(Exe6Obj *hit, uint16_t command)
{
    uint32_t target_side = hit->owner ^ 1u;
    Exe6Obj *player = exe6_get_navi_adrs(target_side);
    if (player == NULL) {
        return;
    }
    Exe6PlayerRuntime *runtime = player->runtime_data;
    enum CommandEffect effect = (enum CommandEffect)(uint8_t)command;
    uint8_t active_cross = target_active_cross(target_side);

    if (effect == COMMAND_EFFECT_RESTORE_CHARGE_SHOT) {
        if (hit->phase_timer_low == CHARGE_SHOT_RESTORE_CROSS) {
            if (active_cross != 0) {
                runtime->active_power_attack =
                    CROSS_POWER_ATTACKS[active_cross];
            }
        } else if (hit->phase_timer_low == CHARGE_SHOT_RESTORE_BASE) {
            runtime->active_power_attack =
                (uint8_t)exe6_navi_status_get(target_side, 5);
        }
    } else if (effect == COMMAND_EFFECT_DISABLE_B_LEFT) {
        runtime->b_left = active_cross == 0
            ? (uint8_t)exe6_navi_status_get(target_side, 7)
            : CROSS_B_LEFTS[active_cross];
    }

    if (command < COMMAND_EFFECT_DISABLE_SUPER_ARMOR
        || command > COMMAND_EFFECT_DISABLE_UNDERSHIRT) {
        return;
    }
    static const uint8_t PROPERTIES[] = {0x1B, 0x1C, 0x1D, 0x23};
    static const uint32_t STATUS_FLAGS[] = {
        EXE6_HIT_STATUS_FLAG_FLOAT_SHOES,
        EXE6_HIT_STATUS_FLAG_AIR_SHOES,
        EXE6_HIT_STATUS_FLAG_UNDERSHIRT,
        EXE6_HIT_STATUS_FLAG_SUPER_ARMOR,
    };
    for (size_t index = 0; index < 4; ++index) {
        bool cross_supplies_ability = active_cross != 0
            && (CROSS_INNATE_STATUS_FLAGS[active_cross]
                & STATUS_FLAGS[index]) != 0;
        if (!cross_supplies_ability
            && exe6_navi_status_get(target_side, PROPERTIES[index]) == 0) {
            exe6_battle_hit_status_flag_off(
                player,
                STATUS_FLAGS[index]
            );
        }
    }
}

static void apply_selected_command(Exe6Obj *hit)
{
    struct HitWork *work = (struct HitWork *)hit->work;
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

static bool hit_init(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0 || exe6_block_in_screen_check() == 0) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_block_to_pos();
    self->z = 0x10 << 16;
    Exe6Hit *hit = exe6_battle_hit_open();
    if (hit == NULL) {
        exe6_obj_move_delete();
        return false;
    }
    exe6_battle_hit_data_set(
        hit,
        (Exe6HitType)self->variant,
        HIT_SELECTOR,
        3
    );
    exe6_battle_hit_hit_mark_set(EXE6_HIT_EFFECT_NORMAL);
    exe6_battle_hit_set(0, self->variant);
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->phase = HIT_PHASE_COLLIDE;
    return true;
}

static void close_hit(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    if (hit == NULL) {
        return;
    }
    exe6_battle_hit_off(hit);
    exe6_battle_hit_close(hit);
    self->hit = NULL;
}

static void confirm_hit_damage(Exe6Obj *self)
{
    struct HitWork *work = (struct HitWork *)self->work;
    Exe6Obj *target = exe6_get_navi_adrs(self->owner ^ 1u);
    if (target != NULL && target->hp < work->target_hp_before) {
        apply_selected_command(self);
        exe6_obj_move_delete();
        return;
    }
    if (work->confirm_timer == 0 || exe6_battle_end_check() != 0) {
        exe6_obj_move_delete();
        return;
    }
    --work->confirm_timer;
}

static void check_hit_contact(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    Exe6Obj *target = exe6_get_navi_adrs(self->owner ^ 1u);
    uint16_t target_hp_before = target == NULL ? 0 : target->hp;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    bool contacted_target = hit->received_hit_flags != 0
        && target != NULL
        && target->block_x == self->block_x
        && target->block_y == self->block_y
        && (uint8_t)self->animation_word == COMMAND_MARKER;
    close_hit(self);
    if (!contacted_target) {
        exe6_obj_move_delete();
        return;
    }
    struct HitWork *work = (struct HitWork *)self->work;
    work->target_hp_before = target_hp_before;
    work->confirm_timer = HIT_CONFIRM_FRAMES;
    self->phase = HIT_PHASE_CONFIRM_DAMAGE;
}

static void hit_update(Exe6Obj *self)
{
    if (self->phase == HIT_PHASE_CONFIRM_DAMAGE) {
        confirm_hit_damage(self);
        return;
    }
    check_hit_contact(self);
}

BN67_SHELL(laserman_hit_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        (void)hit_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        hit_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_ENEMY(laserman_beam_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        beam_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        beam_update(self);
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_obj_char_move();
}

BN67_ENEMY(laserman_actor_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        actor_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        actor_update(self);
    } else {
        actor_destroy(self);
        return;
    }
    exe6_obj_char_move();
}

BN67_SUMMON_ATTACK(0x0E3, laserman_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(laserman_actor_main), spawn_parameters
    );
    if (actor == NULL) {
        return;
    }
    actor->block_x = (uint8_t)block_x;
    actor->block_y = (uint8_t)block_y;
    actor->parameter = (uint8_t)parameter;
    actor->owner_word = owner->owner_word;
    actor->subvariant = 0;
    read_command(actor);
    actor->parent = owner;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
