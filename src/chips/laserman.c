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

#define NAVI_PROPERTY_ATTACK 0x01u
#define NAVI_PROPERTY_RAPID 0x02u
#define NAVI_PROPERTY_CHARGE 0x03u
#define NAVI_PROPERTY_B_BUTTON 0x04u
#define NAVI_PROPERTY_POWER_ATTACK 0x05u
#define NAVI_PROPERTY_CUSTOM_LEVEL 0x0Au

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

struct HitWork {
    uint32_t reserved[5];
    uint32_t command;                    // +0x74
    uint16_t target_hp_before;           // +0x78
    uint8_t confirm_timer;               // +0x7A
};

_Static_assert(
    offsetof(struct HitWork, command) == 0x14,
    "LaserMan hit work layout"
);

_Static_assert(
    sizeof(struct HitWork) <= sizeof(((Exe6Obj *)0)->work),
    "LaserMan hit work size"
);

enum LaserCommand {
    LASERMAN_COMMAND_NONE,
    LASERMAN_COMMAND_UP,
    LASERMAN_COMMAND_DOWN,
    LASERMAN_COMMAND_RIGHT,
    LASERMAN_COMMAND_LEFT,
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
        actor->subvariant = LASERMAN_COMMAND_UP;
    } else if ((keys & EXE6_KEY_DOWN) != 0) {
        actor->subvariant = LASERMAN_COMMAND_DOWN;
    } else if ((keys & EXE6_KEY_RIGHT) != 0) {
        actor->subvariant = LASERMAN_COMMAND_RIGHT;
    } else if ((keys & EXE6_KEY_LEFT) != 0) {
        actor->subvariant = LASERMAN_COMMAND_LEFT;
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

static void spawn_hit(Exe6Obj *beam, uint32_t block_x, uint32_t block_y)
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
    work->command = beam->variant;
    hit->owner_word = beam->owner_word;
    hit->attack = beam->attack;
    hit->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
}

static void spawn_row_event(Exe6Obj *beam)
{
    for (uint32_t block_x = 1; block_x <= 6; ++block_x) {
        spawn_hit(beam, block_x, beam->block_y);
    }
}

static uint8_t command_hit_delay(uint8_t command)
{
    switch ((enum LaserCommand)command) {
    case LASERMAN_COMMAND_UP:
        return 3;
    case LASERMAN_COMMAND_DOWN:
        return 5;
    case LASERMAN_COMMAND_RIGHT:
    case LASERMAN_COMMAND_LEFT:
        return 1;
    default:
        return 0;
    }
}

static void beam_command_tick(Exe6Obj *self)
{
    if (self->removal_state != 0) {
        --self->removal_state;
        return;
    }
    uint8_t hit_delay = command_hit_delay(self->variant);
    if (self->animation_state > hit_delay) {
        return;
    }
    if (self->animation_state == hit_delay) {
        spawn_row_event(self);
    }
    ++self->animation_state;
    self->removal_state = 5;
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
        self->variant == LASERMAN_COMMAND_NONE
            || self->variant == LASERMAN_COMMAND_DOWN
            ? 0
            : 10
    );
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    exe6_obj_col_efc_set(BEAM_SCALES[self->variant]);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    self->phase = BEAM_PHASE_FORM;
}

static void apply_selected_command(Exe6Obj *hit)
{
    struct HitWork *work = (struct HitWork *)hit->work;
    uint32_t target_side = hit->owner ^ 1u;

    switch ((enum LaserCommand)work->command) {
    case LASERMAN_COMMAND_UP:
        exe6_navi_status_set(target_side, NAVI_PROPERTY_ATTACK, 0);
        exe6_navi_status_set(target_side, NAVI_PROPERTY_RAPID, 0);
        exe6_navi_status_set(target_side, NAVI_PROPERTY_CHARGE, 0);
        break;
    case LASERMAN_COMMAND_DOWN: {
        Exe6Obj *target = exe6_get_navi_adrs(target_side);
        if (target != NULL) {
            /* Native Uninstall also clears B+Left and its live base cache. */
            exe6_navi_uninstall(target);
        }
        break;
    }
    case LASERMAN_COMMAND_RIGHT: {
        uint32_t current = exe6_navi_status_get(
            target_side,
            NAVI_PROPERTY_POWER_ATTACK
        );
        Exe6Obj *target = exe6_get_navi_adrs(target_side);
        uint8_t active_cross = target_active_cross(target_side);
        bool restore_base = target != NULL
            && active_cross == 0
            && target->runtime_data->active_power_attack == current;

        exe6_navi_status_set(target_side, NAVI_PROPERTY_B_BUTTON, 0);
        exe6_navi_status_set(target_side, NAVI_PROPERTY_POWER_ATTACK, 1);
        if (target != NULL && active_cross != 0) {
            target->runtime_data->active_power_attack =
                CROSS_POWER_ATTACKS[active_cross];
        } else if (restore_base) {
            target->runtime_data->active_power_attack =
                (uint8_t)exe6_navi_status_get(
                    target_side,
                    NAVI_PROPERTY_POWER_ATTACK
                );
        }
        break;
    }
    case LASERMAN_COMMAND_LEFT: {
        uint32_t value = exe6_navi_status_get(
            target_side,
            NAVI_PROPERTY_CUSTOM_LEVEL
        );
        if (value > 2) {
            --value;
        }
        exe6_navi_status_set(
            target_side,
            NAVI_PROPERTY_CUSTOM_LEVEL,
            value
        );
        break;
    }
    default:
        break;
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
    Exe6Obj *target = exe6_get_navi_adrs(self->owner ^ 1u);
    struct HitWork *work = (struct HitWork *)self->work;
    work->target_hp_before = target == NULL ? 0 : target->hp;
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
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    bool contacted_target = hit->received_hit_flags != 0
        && target != NULL
        && target->block_x == self->block_x
        && target->block_y == self->block_y;
    if (!contacted_target) {
        close_hit(self);
        exe6_obj_move_delete();
        return;
    }
    close_hit(self);
    struct HitWork *work = (struct HitWork *)self->work;
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
