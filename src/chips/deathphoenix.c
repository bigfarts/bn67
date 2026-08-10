#include "runtime.h"

BN67_SPRITE(deathphoenix_battle_sprite, "build/deathphoenix-battle-sprite.bin");
BN67_SPRITE(deathphoenix_strike_sprite, "build/deathphoenix-strike-sprite.bin");

#if FALZAR
BN67_INCBIN(deathphoenix_icon, "build/deathphoenix-icon.bin");
BN67_INCBIN(deathphoenix_image, "build/deathphoenix-image.bin");
BN67_INCBIN(deathphoenix_palette, "build/deathphoenix-palette.bin");
#endif

#if FALZAR
#define DEATHPHOENIX_EFFECT_FLAGS 0x43
#define DEATHPHOENIX_ICON deathphoenix_icon
#define DEATHPHOENIX_IMAGE deathphoenix_image
#define DEATHPHOENIX_PALETTE deathphoenix_palette
#else
#define DEATHPHOENIX_EFFECT_FLAGS 0x03
#define DEATHPHOENIX_ICON ((const uint8_t *)0x0872A3D0u)
#define DEATHPHOENIX_IMAGE ((const uint8_t *)0x0871EFF0u)
#define DEATHPHOENIX_PALETTE ((const uint8_t *)0x087234B0u)
#endif

BN67_CHIP_RECORD(0x134) {
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
    .mb = 93,
    .behavior = {
        .effect_flags = DEATHPHOENIX_EFFECT_FLAGS,
        .counter_settings = 0x94,
        .family = BN67_ATTACK_FAMILY(deathphoenix_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(deathphoenix_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 0x00,
    .library_flags = 0x00,
    .library_lock_on_type = 0x10,
    .alphabetical_sort = 0,
    .power = 150,
    .library_sort_order = 0x0134,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = DEATHPHOENIX_ICON,
    .image = DEATHPHOENIX_IMAGE,
    .palette = DEATHPHOENIX_PALETTE,
};

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t DESTROY_STATE = 8;
static const uint8_t APPEAR_PHASE = 4;
static const uint8_t WAIT_BEFORE_PHASE = 8;
static const uint8_t STRIKES_PHASE = 12;
static const uint8_t WAIT_AFTER_PHASE = 16;
static const uint8_t DISAPPEAR_PHASE = 20;
static const uint8_t RECYCLE_PHASE = 24;
static const uint8_t RECYCLE_INVOKE_STEP = 4;
static const uint8_t RECYCLE_WAIT_STEP = 8;
static const uint8_t RECYCLE_CLEANUP_STEP = 12;
static const uint8_t STRIKE_ATTACK_PHASE = 8;
static const uint8_t STRIKE_COUNT = 12;
static const uint8_t PULSES_PER_STRIKE = 10;
static const uint16_t APPEAR_FRAMES = 16;
static const uint16_t BEFORE_STRIKES_FRAMES = 30;
static const uint16_t STRIKE_INTERVAL = 12;
static const uint16_t AFTER_STRIKES_FRAMES = 40;
static const uint16_t RECYCLE_WAIT_FRAMES = 30;
static const uint16_t STRIKE_LEAD_FRAMES = 10;
static const uint16_t PULSE_FRAMES = 4;
static const uint16_t FLAME_DELAY_FRAMES = 10;
static const uint16_t FLAME_MOTION_FRAMES = 16;
static const uint16_t FIRST_CONTACT_VISUAL = 0x19;
static const uint16_t SECOND_CONTACT_VISUAL = 0x1F;
static const Exe6BlockDamageProperties CONTACT_PROPERTIES = {
    .region = EXE6_HIT_REGION_CURRENT_BLOCK,
    .hit_effect = EXE6_HIT_EFFECT_NORMAL,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_OBJECT_HITTING_ATTACK,
};
static const Exe6BlockDamageProperties INTRO_PROPERTIES = {
    .region = EXE6_HIT_REGION_ALL_VALID_BLOCKS,
    .hit_effect = EXE6_HIT_EFFECT_NONE,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_27,
};
static const uintptr_t SAVED_NAVI_ADDRESS = 0x0203C960;
static const uintptr_t SAVED_NAVI_DISPATCH_REFERENCE = 0x08017BBC;
static const uintptr_t SINE_TABLE_ADDRESS = 0x080065E0;

static const uint8_t STRIKE_PATTERNS[] = {
    2, 1, 3,
    1, 2, 3,
    1, 3, 3,
    2, 1, 2,
};

struct Exe6SavedNaviFields {
    uint8_t id;
    uint8_t parameter;
    uint16_t offset;
    uint32_t data;
    uint32_t properties;
};

struct Exe6DeathphoenixWork {
    uint32_t pattern_row;                // +0x60
    uint8_t recycle_completion;          // +0x64
    uint8_t reserved_65[3];
    const uint8_t *pattern;              // +0x68
    uint8_t blocks[3][4];                // +0x6C
    uint32_t pattern_column;             // +0x78
};

_Static_assert(
    offsetof(struct Exe6DeathphoenixWork, pattern_column) == 0x18,
    "DeathPhoenix work layout"
);

static bool timer_positive_after_decrement(Exe6Obj *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer > 0;
}

static bool timer_nonnegative_after_decrement(Exe6Obj *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer >= 0;
}

static void set_phase(Exe6Obj *self, uint8_t phase)
{
    self->phase = phase;
    self->phase_timer = 0;
}

static void finish(Exe6Obj *self)
{
    *self->completion = 0;
    self->state_word = DESTROY_STATE;
}

static void build_block_list(
    Exe6Obj *self,
    uint32_t block_y,
    uint8_t *output
)
{
    uint8_t candidates[8];
    uint8_t shuffled[16];
    exe6_mem_clear8(candidates, sizeof(candidates));
    exe6_mem_clear8(shuffled, sizeof(shuffled));

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    int32_t block_x = (int32_t)self->block_x + direction;
    uint32_t count = 0;
    while (exe6_block_in_screen_check_sub((uint32_t)block_x, block_y) != 0) {
        if (exe6_block_move_check(
                (uint32_t)block_x,
                block_y,
                EXE6_BLOCK_FLAG_SOLID,
                0
            ) != 0) {
            candidates[count++] = (uint8_t)(block_x | (block_y << 4));
        }
        block_x += direction;
    }

    for (uint32_t written = 0; count != 0 && written < 4; written += count) {
        exe6_mem_trans8(candidates, shuffled + written, count);
        exe6_shuffle_sub(shuffled + written, count, count);
    }
    exe6_mem_trans8(shuffled, (void *)output, 4);
}

static void actor_init(Exe6Obj *self)
{
    struct Exe6DeathphoenixWork *work =
        (struct Exe6DeathphoenixWork *)self->work;
    exe6_battle_obj_char_init(
        0x00010000u
        | (BN67_SPRITE_GROUP(deathphoenix_battle_sprite) << 8)
        | BN67_SPRITE_ID(deathphoenix_battle_sprite)
    );
    exe6_obj_flip_set(exe6_enemy_flip_check());

    self->block_x = (uint8_t)((self->owner ^ self->owner_aux) * 5u + 1u);
    self->block_y = 3;
    exe6_block_to_pos();
    self->z = 0;
    self->y += 3 << 16;
    self->z += 3 << 16;

    build_block_list(self, 1, work->blocks[0]);
    build_block_list(self, 2, work->blocks[1]);
    build_block_list(self, 3, work->blocks[2]);
    self->state_word = ACTIVE_STATE;
}

static void begin(Exe6Obj *self)
{
    (void)exe6_set_shl03_ev(1, 1, 0, 0, INTRO_PROPERTIES, 0, 0);
    set_phase(self, APPEAR_PHASE);
}

static void appear(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        exe6_sound_req(0x94);
        self->timer = 0;
        self->substate = 4;
    }

    uint32_t blend = (uint32_t)self->timer + 1u;
    self->timer = (uint16_t)blend;
    if (blend < APPEAR_FRAMES) {
        exe6_obj_bld_set(blend);
        return;
    }
    exe6_obj_bld_reset();
    set_phase(self, WAIT_BEFORE_PHASE);
}

static void wait_before_strikes(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->timer = BEFORE_STRIKES_FRAMES;
        self->substate = 4;
    }
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->timer = 0;
    self->aux_timer = 0;
    struct Exe6DeathphoenixWork *work =
        (struct Exe6DeathphoenixWork *)self->work;
    work->pattern_row = 0;
    work->pattern_column = 0;
    set_phase(self, STRIKES_PHASE);
}

static void strike_spawn(
    Exe6Obj *actor,
    uint32_t block_x,
    uint32_t block_y,
    uint32_t alternate
)
{
    Exe6Obj *strike = exe6_efc_open(
        BN67_OBJ_ID(deathphoenix_strike_main),
        exe6_obj_spawn_with_variant((uint8_t)alternate)
    );
    if (strike == NULL) {
        return;
    }
    strike->block_x = (uint8_t)block_x;
    strike->block_y = (uint8_t)block_y;
    strike->attack = actor->attack;
    strike->owner_word = actor->owner_word;
    strike->parent = actor;
}

static void spawn_strike_for_block(
    Exe6Obj *self,
    uint32_t block_x,
    uint32_t block_y
)
{
    if (block_x == 0) {
        return;
    }
    uint32_t distance = self->block_x > block_x
        ? self->block_x - block_x
        : block_x - self->block_x;
    uint32_t alternate = distance > 2;
    if (distance == 3 && (exe6_rand2() & 0x10u) != 0) {
        alternate = 0;
    }
    strike_spawn(self, block_x, block_y, alternate);
}

static void strikes(Exe6Obj *self)
{
    struct Exe6DeathphoenixWork *work =
        (struct Exe6DeathphoenixWork *)self->work;
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->timer = STRIKE_INTERVAL;

    if (work->pattern_row == 0) {
        uint32_t pattern = exe6_rand2() % 3u;
        work->pattern = &STRIKE_PATTERNS[pattern * 3u];
    }
    const uint8_t *pattern = work->pattern;
    uint32_t row = pattern[work->pattern_row] - 1u;
    uint8_t packed = work->blocks[row][work->pattern_column];
    spawn_strike_for_block(self, packed & 7u, packed >> 4);

    ++work->pattern_row;
    if (work->pattern_row >= 3) {
        work->pattern_row = 0;
        ++work->pattern_column;
    }
    if (++self->aux_timer >= STRIKE_COUNT) {
        set_phase(self, WAIT_AFTER_PHASE);
    }
}

static void wait_after_strikes(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->timer = AFTER_STRIKES_FRAMES;
        self->substate = 4;
    }
    if (!timer_positive_after_decrement(self)) {
        set_phase(self, DISAPPEAR_PHASE);
    }
}

static bool saved_navi_is_available(void)
{
    const struct Exe6SavedNaviFields *saved =
        (const struct Exe6SavedNaviFields *)SAVED_NAVI_ADDRESS;
    return saved->data != 0 && saved->id != UINT8_MAX;
}

static void disappear(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        self->timer = APPEAR_FRAMES;
        self->substate = 4;
    }
    if (timer_nonnegative_after_decrement(self)) {
        exe6_obj_bld_set(self->timer);
        return;
    }

    self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    exe6_obj_bld_reset();
    if (self->parent->hp != 0 && saved_navi_is_available()) {
        set_phase(self, RECYCLE_PHASE);
    } else {
        finish(self);
    }
}

static void recycle_intro(Exe6Obj *self)
{
    if (self->substate == 0) {
        exe6_set_efc0c(self->parent, 1);
        self->timer = RECYCLE_WAIT_FRAMES;
        self->substate = 4;
    }
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase_timer = RECYCLE_INVOKE_STEP;
    }
}

static void recycle_invoke(Exe6Obj *self)
{
    struct Exe6DeathphoenixWork *work =
        (struct Exe6DeathphoenixWork *)self->work;
    uint8_t *completion = &work->recycle_completion;
    if (self->substate == 0) {
        const struct Exe6SavedNaviFields *saved =
            (const struct Exe6SavedNaviFields *)SAVED_NAVI_ADDRESS;
        const uintptr_t *dispatch = *(const uintptr_t *const *)
            SAVED_NAVI_DISPATCH_REFERENCE;
        Exe6Obj *owner = self->parent;
        exe6_saved_navi_dispatch(
            dispatch[saved->id],
            owner,
            owner->block_x,
            owner->block_y,
            saved->parameter,
            saved->data + saved->offset,
            saved->properties,
            completion
        );
        self->substate = 4;
    }
    if (*completion == 0) {
        self->phase_timer = RECYCLE_WAIT_STEP;
    }
}

static void recycle_wait(Exe6Obj *self)
{
    if (self->substate == 0) {
        self->timer = RECYCLE_WAIT_FRAMES;
        self->substate = 4;
    }
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase_timer = RECYCLE_CLEANUP_STEP;
    }
}

static void recycle_update(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        recycle_intro(self);
    } else if (self->phase_timer_low == RECYCLE_INVOKE_STEP) {
        recycle_invoke(self);
    } else if (self->phase_timer_low == RECYCLE_WAIT_STEP) {
        recycle_wait(self);
    } else {
        finish(self);
    }
}

static void actor_update(Exe6Obj *self)
{
    switch (self->phase) {
    case 0:
        begin(self);
        break;
    case APPEAR_PHASE:
        appear(self);
        break;
    case WAIT_BEFORE_PHASE:
        wait_before_strikes(self);
        break;
    case STRIKES_PHASE:
        strikes(self);
        break;
    case WAIT_AFTER_PHASE:
        wait_after_strikes(self);
        break;
    case DISAPPEAR_PHASE:
        disappear(self);
        break;
    default:
        recycle_update(self);
        break;
    }
}

static void flame_spawn(Exe6Obj *strike, uint32_t alternate)
{
    Exe6Obj *flame = exe6_efc_open(
        BN67_OBJ_ID(deathphoenix_flame_main),
        exe6_obj_spawn_with_variant((uint8_t)alternate)
    );
    if (flame == NULL) {
        return;
    }
    flame->block_x = strike->block_x;
    flame->block_y = strike->block_y;
    flame->owner_word = strike->owner_word;
    flame->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    if (alternate != 0) {
        flame->owner_aux ^= 1u;
    }
}

static void spawn_contact(
    Exe6Obj *self,
    uint32_t block_x,
    uint32_t block_y,
    uint16_t visual
)
{
    Exe6Obj *contact = exe6_set_shl03_ev(
        block_x,
        block_y,
        0,
        0x00100000,
        CONTACT_PROPERTIES,
        self->attack,
        3
    );
    if (contact != NULL) {
        contact->timer = visual;
    }
}

static void strike_attack(Exe6Obj *self)
{
    uint32_t frame = self->aux_timer++;
    if (frame == 0) {
        spawn_contact(
            self,
            self->block_x,
            self->block_y,
            FIRST_CONTACT_VISUAL
        );
    } else if (frame == 16) {
        int32_t direction = (self->owner ^ self->variant) == 0 ? 1 : -1;
        spawn_contact(
            self,
            (uint32_t)((int32_t)self->block_x + direction),
            self->block_y,
            SECOND_CONTACT_VISUAL
        );
    }

    if (self->phase_timer_low == 0) {
        self->phase_timer_low = 1;
        flame_spawn(self, self->variant);
        self->timer = PULSE_FRAMES;
    }
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->phase_timer_low = 0;
    if (++self->substate >= PULSES_PER_STRIKE) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = DESTROY_STATE;
    }
}

static void strike_lead_in(Exe6Obj *self)
{
    if (self->animation == 0) {
        if ((exe6_obj_seq_info_get()
                & EXE6_ANIMATION_FRAME_FLAG_END) == 0) {
            return;
        }
        self->animation = 1;
    }
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->aux_timer = 0;
    set_phase(self, STRIKE_ATTACK_PHASE);
    exe6_sound_req(0x145);
}

static void strike_update(Exe6Obj *self)
{
    if (self->phase == 0) {
        self->timer = STRIKE_LEAD_FRAMES;
        set_phase(self, 4);
    } else if (self->phase == 4) {
        strike_lead_in(self);
    } else {
        strike_attack(self);
    }
}

static void strike_init(Exe6Obj *self)
{
    exe6_battle_obj_char_init(
        0x01000000u
        | (BN67_SPRITE_GROUP(deathphoenix_strike_sprite) << 8)
        | BN67_SPRITE_ID(deathphoenix_strike_sprite)
    );
    exe6_obj_shadow_all_set();
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    exe6_block_to_pos();
    self->z = 0;
    exe6_sound_req(0x144);
    self->state_word = ACTIVE_STATE;
    strike_update(self);
}

static void flame_motion(Exe6Obj *self)
{
    if (self->phase_timer_low == 0) {
        self->phase_timer_low = 1;
        self->variant_word = 0;
        self->timer = FLAME_MOTION_FRAMES;
        self->animation_state_word = 0x0800;
        self->velocity_x =
            (int32_t)exe6_calc_pl_em_dir_spd_for(self) * 0x00028000;
    }
    self->x += self->velocity_x;

    uint16_t angle = self->variant_word;
    uint16_t step = self->animation_state_word;
    if (self->timer > 8) {
        step = (uint16_t)(step - (step >> 2));
    } else {
        step = (uint16_t)(step + (step >> 2));
    }
    angle = (uint16_t)(angle + step);
    self->variant_word = angle;

    const int16_t *sine = (const int16_t *)SINE_TABLE_ADDRESS;
    int32_t height = sine[angle >> 8];
    self->z = (height * 20 << 8) + 0x00040000;
    if (!timer_positive_after_decrement(self)) {
        self->state_word = DESTROY_STATE;
    }
}

static void flame_update(Exe6Obj *self)
{
    if (self->phase == 0) {
        if (self->phase_timer_low == 0) {
            self->phase_timer_low = 1;
            self->timer = FLAME_DELAY_FRAMES;
        }
        if (!timer_positive_after_decrement(self)) {
            set_phase(self, 4);
        }
    } else {
        flame_motion(self);
    }
}

static void flame_init(Exe6Obj *self)
{
    exe6_block_to_pos();
    self->y -= 1 << 16;
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(deathphoenix_strike_sprite),
        BN67_SPRITE_ID(deathphoenix_strike_sprite)
    );
    self->animation = 2;
    exe6_obj_dma_seq_set(2);
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    exe6_obj_clt_set(0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->state_word = ACTIVE_STATE;
}

BN67_EFFECT(deathphoenix_flame_main)
{
    if (self->state == 0) {
        flame_init(self);
    } else if (self->state == ACTIVE_STATE) {
        flame_update(self);
    } else {
        exe6_obj_move_delete();
    }
    exe6_battle_obj_char_move();
}

BN67_EFFECT(deathphoenix_strike_main)
{
    if (self->state == 0) {
        strike_init(self);
    } else if (self->state == ACTIVE_STATE) {
        strike_update(self);
    } else {
        exe6_obj_move_delete();
    }
    exe6_battle_obj_char_move();
}

BN67_ENEMY(deathphoenix_actor_main)
{
    if (self->state == 0) {
        actor_init(self);
    } else if (self->state == ACTIVE_STATE) {
        actor_update(self);
    } else {
        exe6_obj_move_delete();
        return;
    }
    exe6_battle_obj_char_move2();
}

BN67_SUMMON_ATTACK(0x134, deathphoenix_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(deathphoenix_actor_main), spawn_parameters
    );
    if (actor == NULL) {
        return;
    }
    actor->block_x = (uint8_t)block_x;
    actor->block_y = (uint8_t)block_y;
    actor->parameter = (uint8_t)parameter;
    actor->parent = owner;
    actor->owner_word = owner->owner_word;
    actor->attack = attack;
    actor->completion = completion;
    *completion = 1;
}
