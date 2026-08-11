#include "common.h"
#include "runtime.h"

BN67_USE_SONG(common_navi_summon_song);
BN67_SPRITE(laserman_battle_sprite, "build/laserman-battle-sprite.bin");

BN67_INCBIN(laserman_image, "build/laserman-image.bin");
BN67_INCBIN(laserman_palette_base, "build/laserman-pal-base.bin");
BN67_ASM_RESOURCE(
    laserman_palette_ex,
    ".incbin \"build/laserman-pal-base.bin\",0,0x02\n"
    ".short 0x00C0,0x0180,0x0280,0x03E0,0x0060\n"
    ".incbin \"build/laserman-pal-base.bin\",0x0C,0x14\n"
);
BN67_INCBIN(laserman_palette_sp, "build/laserman-pal-sp.bin");
BN67_SONG(
    laserman_fire_song,
    BN67_PCM(
        laserman_fire,
        0x40,
        0x00,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBF,0x40\n"
        ".byte 0xBE,0x7F,0xF6,0x2F,0x7F,0xA2,0x81,0xBE\n"
        ".byte 0x60,0x84,0x40,0x84,0x20,0x84,0x10,0x84,0xB1\n",
        "build/laserman-fire-sample.bin"
    )
);

#if FALZAR
#define LASERMAN_ICON ((const uint8_t *)0x0872BE14u)
#define LASERMAN_LIBRARY_FLAGS 0x01
#define LASERMAN_LIBRARY_NUMBER_BASE 0x03
#define LASERMAN_LIBRARY_SORT_BASE 0x00F2
#else
#define LASERMAN_ICON ((const uint8_t *)0x08729D50u)
#define LASERMAN_LIBRARY_FLAGS 0x00
#define LASERMAN_LIBRARY_NUMBER_BASE 0x07
#define LASERMAN_LIBRARY_SORT_BASE 0x00E3
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
    .library_number = LASERMAN_LIBRARY_NUMBER_BASE,
    .library_flags = LASERMAN_LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 100,
    .library_sort_order = LASERMAN_LIBRARY_SORT_BASE,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = LASERMAN_ICON,
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
    .library_number = LASERMAN_LIBRARY_NUMBER_BASE + 1,
    .library_flags = LASERMAN_LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 150,
    .library_sort_order = LASERMAN_LIBRARY_SORT_BASE + 1,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = LASERMAN_ICON,
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
    .library_number = LASERMAN_LIBRARY_NUMBER_BASE + 2,
    .library_flags = LASERMAN_LIBRARY_FLAGS,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 200,
    .library_sort_order = LASERMAN_LIBRARY_SORT_BASE + 2,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = LASERMAN_ICON,
    .image = laserman_image,
    .palette = laserman_palette_sp,
};

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t DESTROY_STATE = 8;
static const uint8_t HIT_VISUAL = 25;
static const uint16_t WAIT_FRAMES = 20;
static const uint16_t RAISE_FRAMES = 30;
static const uint16_t LASER_FRAMES = 80;
static const uint16_t BEAM_FRAMES = 60;
static const Exe6HitType HIT_SELECTOR =
    EXE6_HIT_TYPE_STANDARD_TARGET;

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

static void set_animation(Exe6Obj *self, uint8_t animation)
{
    self->animation = animation;
    self->palette = UINT8_MAX;
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
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

static void actor_destroy(Exe6Obj *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    exe6_obj_move_delete();
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
            exe6_sound_req(BN67_SONG_ID(laserman_fire_song));
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
    if ((exe6_obj_seq_info_get()
            & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = DESTROY_STATE;
    }
}

static void actor_update(Exe6Obj *self)
{
    if (self->phase == 0) {
        if (!timer_expired(self)) {
            return;
        }
        if (exe6_block_move_check(
                self->block_x,
                self->block_y,
                EXE6_BLOCK_FLAG_SOLID,
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

static void actor_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(laserman_battle_sprite),
        BN67_SPRITE_ID(laserman_battle_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    self->animation = 0;
    self->palette = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(0);
    exe6_obj_prio_set(1);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    self->phase = 0;
    self->substate = 0;
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
    struct LasermanHitWork *work =
        (struct LasermanHitWork *)hit->work;
    hit->block_x = (uint8_t)block_x;
    hit->block_y = (uint8_t)block_y;
    hit->animation_word = command;
    work->command_stream = beam->variant;
    hit->owner_word = beam->owner_word;
    hit->attack = command == COMMAND_MARKER ? beam->attack : 0;
    if (command != COMMAND_MARKER) {
        hit->phase_timer_low = 0;
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
    if (self->phase == 0) {
        if ((exe6_obj_seq_info_get()
                & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
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
    if ((exe6_obj_seq_info_get()
            & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        self->state_word = DESTROY_STATE;
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
    set_animation(self, 17);
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_clt_set(
        self->variant == 0 || self->variant == 2 ? 0 : 10
    );
    exe6_obj_prio_set(1);
    exe6_obj_col_efc_set(BEAM_SCALES[self->variant]);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
    self->phase = 0;
}

static void apply_command_effect(Exe6Obj *hit, uint16_t command)
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
        uint32_t current = exe6_navi_status_get(target_side, 5);
        Exe6Obj *player = exe6_get_navi_adrs(target_side);
        if (player != NULL) {
            Exe6PlayerRuntime *runtime = player->runtime_data;
            if (runtime->active_power_attack == current) {
                hit->phase_timer_low = 1;
            }
        }
        exe6_navi_status_set(target_side, 4, 0);
        property = 5;
        value = 1;
        break;
    }
    case COMMAND_EFFECT_REDUCE_CUSTOM:
        value = exe6_navi_status_get(target_side, 0x0A);
        if (value > 2) {
            --value;
        }
        property = 0x0A;
        break;
    default:
        return;
    }
    exe6_navi_status_set(target_side, property, value);
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
    if (effect == COMMAND_EFFECT_RESTORE_CHARGE_SHOT) {
        if (hit->phase_timer_low != 0) {
            runtime->active_power_attack =
                (uint8_t)exe6_navi_status_get(target_side, 5);
        }
    } else if (effect == COMMAND_EFFECT_DISABLE_B_LEFT) {
        runtime->b_left =
            (uint8_t)exe6_navi_status_get(target_side, 7);
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
        if (exe6_navi_status_get(target_side, PROPERTIES[index]) == 0) {
            exe6_battle_hit_status_flag_off(
                player,
                STATUS_FLAGS[index]
            );
        }
    }
}

static void apply_selected_command(Exe6Obj *hit)
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
    self->state_word = ACTIVE_STATE;
    return true;
}

static void hit_update(Exe6Obj *self)
{
    Exe6Hit *hit = self->hit;
    exe6_battle_hit_check(hit);
    exe6_battle_hit_hit_mark_check();
    if (hit->received_hit_flags != 0) {
        Exe6Obj *target = exe6_get_navi_adrs(self->owner ^ 1u);
        if (target != NULL
            && target->block_x == self->block_x
            && target->block_y == self->block_y
            && (uint8_t)self->animation_word == COMMAND_MARKER) {
            apply_selected_command(self);
        }
    }
    exe6_battle_hit_off(hit);
    exe6_battle_hit_close(self->hit);
    exe6_obj_move_delete();
}

BN67_SHELL(laserman_hit_main)
{
    if (self->state == 0) {
        (void)hit_init(self);
    } else if (self->state == ACTIVE_STATE) {
        hit_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_ENEMY(laserman_beam_main)
{
    if (self->state == 0) {
        beam_init(self);
    } else if (self->state == ACTIVE_STATE) {
        beam_update(self);
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_obj_char_move();
}

BN67_ENEMY(laserman_actor_main)
{
    if (self->state == 0) {
        actor_init(self);
    } else if (self->state == ACTIVE_STATE) {
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
