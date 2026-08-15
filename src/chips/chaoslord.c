#include "common.h"
#include "runtime.h"

BN67_SPRITE(chaoslord_main_sprite, "build/chaoslord-bass-sprite.bin");
BN67_SPRITE(chaoslord_aura_sprite, "build/chaoslord-aura-sprite.bin");
BN67_SPRITE(chaoslord_teardown_sprite, "build/chaoslord-teardown-sprite.bin");
BN67_SPRITE(chaoslord_apparition_sprite, "build/chaoslord-apparition-sprite.bin");

#if !FALZAR
BN67_INCBIN(chaoslord_icon, "build/chaoslord-icon.bin");
BN67_INCBIN(chaoslord_image, "build/chaoslord-image.bin");
BN67_INCBIN(chaoslord_palette, "build/chaoslord-palette.bin");
#endif

BN67_INCBIN(chaoslord_trig_table, "build/chaoslord-trig.bin");

#if FALZAR
#define EFFECT_FLAGS                                                 \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK)
#define ICON ((const uint8_t *)0x0872C414u)
#define IMAGE ((const uint8_t *)0x08720B74u)
#define PALETTE ((const uint8_t *)0x08725554u)
#else
#define EFFECT_FLAGS                                                    \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING | EXE6_CHIP_EFFECT_FLAG_ATTACK | \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#define ICON chaoslord_icon
#define IMAGE chaoslord_image
#define PALETTE chaoslord_palette
#endif

BN67_CHIP_RECORD(0x12e) {
    .codes = {
        EXE6_CHIP_CODE_X,
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
        .counter_settings = 0xB2,
        .family = BN67_ATTACK_FAMILY(chaoslord_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(chaoslord_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x00,
        .lock_on = 0x00,
        .object_spawn = {0},
        .delay = 0,
    },
    /* Preserve BigHook's second Giga-library position and metadata. */
    .library_number = 0x02,
    .library_flags = 0x18,
    .library_lock_on_type = 0x00,
    .alphabetical_sort = 0,
    .power = 500,
    .library_sort_order = 0x012E,
    .library_gate_usage = 0x01,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE,
};

enum BallPhase {
    BALL_PHASE_INIT,
    BALL_PHASE_FOLLOW = 4,
};

enum ControllerPhase {
    CONTROLLER_PHASE_INTRO,
    CONTROLLER_PHASE_APPARITION = 4,
    CONTROLLER_PHASE_APPROACH = 8,
    CONTROLLER_PHASE_ATTACK = 12,
    CONTROLLER_PHASE_OUTRO = 16,
};

enum AttackPhase {
    ATTACK_PHASE_FORM,
    ATTACK_PHASE_FLIGHT = 4,
};

enum AuraPhase {
    AURA_PHASE_APPEAR,
    AURA_PHASE_LIFESPAN = 4,
    AURA_PHASE_FADE = 8,
};

enum AuraAppearStep {
    AURA_APPEAR_STEP_INIT,
    AURA_APPEAR_STEP_TRANSITION,
    AURA_APPEAR_STEP_HOLD,
};
static const uint16_t INTRO_FRAMES = 0x8E;
static const uint16_t APPARITION_FRAMES = 0x25;
static const uint16_t APPROACH_FRAMES = 0x48;
static const uint16_t ATTACK_FRAMES = 0x80;
static const uint16_t ATTACK_SPAWN_TIMER = 0x80;
static const uint16_t OUTRO_FRAMES = 0x28;
static const uint16_t BALL_FLIGHT_FRAMES = 0x0F;
static const Exe6BlockDamageProperties DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CENTERED_3X3,
    .hit_effect = EXE6_HIT_EFFECT_NONE,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_15,
};
struct BurstEntry {
    int8_t x;
    int8_t y;
    uint8_t palette;
};

struct AttackSpriteEntry {
    uint8_t group;
    uint8_t index;
    uint8_t animation;
    uint8_t palette;
};

struct BlockOffset {
    int8_t x;
    int8_t y;
};

struct ControllerWork {
    uint32_t scale_low;                  // +0x60
    uint32_t scale_middle;               // +0x64
    uint32_t scale_high;                 // +0x68
};

struct BurstWork {
    int32_t radius;                      // +0x60
    int32_t radius_step;                 // +0x64
    int32_t center_x;                    // +0x68
    uint32_t reserved_6c;
    uint32_t vector;                     // +0x70
    uint32_t elevated;                   // +0x74
};

struct ImpactWork {
    uint8_t blocks[9];                   // +0x60
};

static const struct BurstEntry BURST_TABLE[] = {
    { 0x30, -0x13, 1 },
    { 0x03, -0x16, 0 },
    { 0x0A, -0x34, 0 },
    { -0x06, -0x40, 0 },
    { -0x0E, -0x28, 0 },
    { -0x18, -0x0A, 1 },
    { 0x03, -0x03, 1 },
    { 0x16, -0x25, 1 },
    { 0x17, -0x4D, 1 },
    { 0x1F, -0x3A, 1 },
};

/* BN6 block pattern 0x11, used to find room for the entrance. */
static const struct BlockOffset ENTRANCE_BLOCK_PATTERN[] = {
    { 0, 0 },
    { 0, -1 },
    { 0, 1 },
    { 1, 0 },
    { 1, -1 },
    { 1, 1 },
};

/* BN6 block pattern 0x0F, used by the native type-4 impact effect. */
static const struct BlockOffset IMPACT_BLOCK_PATTERN[] = {
    { 0, 0 },
    { 0, -1 },
    { 0, 1 },
    { 1, 0 },
    { -1, 0 },
    { 1, -1 },
    { -1, 1 },
    { 1, 1 },
    { -1, -1 },
};

static const struct AttackSpriteEntry ATTACK_SPRITE_TABLE[] = {
    { 0x14, 0x08, 0x00, 0x02 },
    { 0x14, 0x19, 0x00, 0x00 },
};

static uint32_t packed_scale(Exe6Obj *self)
{
    struct ControllerWork *work = (struct ControllerWork *)self->work;
    return work->scale_low
        | (work->scale_middle << 5)
        | (work->scale_high << 10);
}

static uint32_t integer_sqrt(uint32_t value)
{
    uint32_t result = 0;
    uint32_t bit = 1u << 30;
    while (bit > value) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (value >= result + bit) {
            value -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}

static void ball_request_destroy(Exe6Obj *ball)
{
    if (ball != NULL) {
        ball->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static Exe6Obj *spawn_ball(Exe6Obj *controller)
{
    Exe6ObjSpawnParameters spawn_parameters = {
        .variant = (uint8_t)BN67_SPRITE_GROUP(chaoslord_main_sprite),
        .subvariant = (uint8_t)BN67_SPRITE_ID(chaoslord_main_sprite),
        .animation_state = 1,
        .removal_state = 0x10,
    };
    Exe6Obj *ball = exe6_em_open(
        BN67_OBJ_ID(chaoslord_ball_main), spawn_parameters
    );
    if (ball == NULL) {
        return NULL;
    }
    ball->parent = controller;
    ball->owner = controller->owner;
    return ball;
}

static void ball_phase_active(Exe6Obj *self)
{
    exe6_obj_bld_link_copy(self->parent);
    exe6_battle_obj_char_move2();
}

static void ball_update(Exe6Obj *self)
{
    Exe6Obj *controller = self->parent;
    uint8_t animation = (uint8_t)(
        controller->animation + self->removal_state
    );
    self->animation = animation;
    if (animation != self->palette) {
        exe6_obj_dma_seq_set(animation);
        exe6_obj_char_set();
    }

    self->x = controller->x;
    self->y = controller->y;
    self->z = controller->z;
    self->header_flags = (uint8_t)(
        (self->header_flags & (uint8_t)~EXE6_OBJ_FLAG_VISIBLE)
        | (controller->header_flags & EXE6_OBJ_FLAG_VISIBLE)
    );
    exe6_obj_col_efc_set(exe6_obj_col_efc_link_get(controller));
    exe6_obj_flash_link_copy(controller);
    exe6_obj_mosaic_link_copy(controller);
    self->owner_aux = controller->owner_aux;
    exe6_obj_flip_set(exe6_enemy_flip_check());

    ball_phase_active(self);
}

static void ball_init(Exe6Obj *self)
{
    exe6_obj_char_init(0x80, self->variant, self->subvariant);
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    exe6_obj_clt_set(exe6_obj_clt_link_get(self->parent));
    self->animation_word = (uint16_t)(
        self->parent->animation + self->removal_state
    );
    exe6_obj_dma_seq_set(self->animation);
    exe6_obj_char_set();
    exe6_obj_char_move();
    set_phase(self, BALL_PHASE_FOLLOW);
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    ball_update(self);
}

BN67_ENEMY(chaoslord_ball_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        ball_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        ball_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

static Exe6Obj *spawn_aura(
    Exe6Obj *controller,
    int32_t x,
    int32_t y,
    int32_t z,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *aura = exe6_efc_open_at(
        BN67_OBJ_ID(chaoslord_aura_main),
        x,
        y,
        z,
        spawn_parameters
    );
    if (aura == NULL) {
        return NULL;
    }
    aura->parent = controller;
    aura->owner_word = controller->owner_word;
    return aura;
}

static Exe6Obj *spawn_burst(
    Exe6Obj *controller,
    int32_t x,
    int32_t y,
    uint32_t vector,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    Exe6Obj *burst = exe6_efc_open_at(
        BN67_OBJ_ID(chaoslord_burst_main),
        x,
        y,
        0,
        spawn_parameters
    );
    if (burst == NULL) {
        return NULL;
    }
    burst->parent = controller;
    struct BurstWork *work = (struct BurstWork *)burst->work;
    work->vector = vector;
    burst->owner_word = controller->owner_word;
    burst->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    return burst;
}

static Exe6Obj *spawn_teardown(Exe6Obj *controller)
{
    Exe6Obj *effect = exe6_efc_open_at(
        BN67_OBJ_ID(chaoslord_teardown_main),
        controller->x,
        controller->y,
        controller->z + (16 << 16),
        exe6_obj_spawn_with_variant(0x12)
    );
    if (effect == NULL) {
        return NULL;
    }
    effect->owner_word = controller->owner_word;
    return effect;
}

static Exe6Obj *spawn_flash(void)
{
    Exe6ObjSpawnParameters spawn_parameters = {
        .variant = 1,
        .subvariant = 4,
        .animation_state = 1,
    };
    Exe6Obj *flash = exe6_efc_open(
        BN67_OBJ_ID(chaoslord_flash_main), spawn_parameters
    );
    if (flash == NULL) {
        return NULL;
    }
    flash->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_PAUSE
        | EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    return flash;
}

static Exe6Obj *spawn_attack_obj(
    Exe6Obj *controller,
    uint8_t variant
)
{
    Exe6Obj *attack = exe6_shl_open(
        BN67_OBJ_ID(chaoslord_attack_obj_main),
        controller->x,
        controller->y,
        controller->z,
        exe6_obj_spawn_with_variant(variant)
    );
    if (attack == NULL) {
        return NULL;
    }
    attack->parameter = 0;
    attack->attack = controller->attack;
    attack->parent = NULL;
    attack->owner_word = controller->owner_word;
    attack->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    return attack;
}

static void delete_obstacles(void)
{
    Exe6Obj **obstacles = exe6_runtime()->battle_context->obstacles;
    for (size_t index = 0; index < EXE6_OBSTACLE_SLOT_COUNT; ++index) {
        if (obstacles[index] != NULL) {
            obstacles[index]->hp = 0;
        }
    }
}

static void controller_update(Exe6Obj *self);

static bool entrance_has_block(
    uint32_t center_x,
    uint32_t center_y,
    uint32_t area,
    uint32_t side
)
{
    int32_t direction = 1 - (int32_t)(side * 2u);
    for (
        uint32_t index = 0;
        index < sizeof(ENTRANCE_BLOCK_PATTERN) / sizeof(ENTRANCE_BLOCK_PATTERN[0]);
        ++index
    ) {
        uint32_t block_x = (uint32_t)(
            (int32_t)center_x + ENTRANCE_BLOCK_PATTERN[index].x * direction
        );
        uint32_t block_y = (uint32_t)(
            (int32_t)center_y + ENTRANCE_BLOCK_PATTERN[index].y
        );
        if (exe6_block_move_check(block_x, block_y, area, 0) != 0) {
            return true;
        }
    }
    return false;
}

static void controller_init(Exe6Obj *self)
{
    exe6_sound_req(0x94);
    delete_obstacles();

    uint32_t side = self->owner ^ self->owner_aux;
    uint32_t search_x = side * 5u + 1u;
    uint32_t search_area = self->owner == 0
        ? EXE6_BLOCK_FLAG_SIDE_1_HIT
        : EXE6_BLOCK_FLAG_SIDE_0_HIT;
    if (entrance_has_block(search_x, 2, search_area, side)) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
        return;
    }

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    uint32_t block_x = (self->owner ^ self->owner_aux) * 5u + 1u;
    uint64_t coordinates = exe6_get_block_pos(block_x, 2);
    int32_t x = (int32_t)(uint32_t)coordinates + direction * (8 << 16);
    int32_t y = (int32_t)(uint32_t)(coordinates >> 32);
    (void)spawn_aura(self, x, y, 0, (Exe6ObjSpawnParameters){
        .variant = 0x0A,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Exe6ObjSpawnParameters){
        .variant = 0x07,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Exe6ObjSpawnParameters){
        .variant = 0x04,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Exe6ObjSpawnParameters){
        .variant = 0x01,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Exe6ObjSpawnParameters){
        .variant = 0x0D,
        .subvariant = 0x0A,
        .animation_state = 0x7C,
        .removal_state = 0x01,
    });

    self->timer = INTRO_FRAMES;
    self->velocity_x = direction * 0x00009D89;
    struct ControllerWork *work = (struct ControllerWork *)self->work;
    work->scale_low = 5;
    work->scale_middle = 1;
    work->scale_high = 0x17;
    self->block_x = (uint8_t)block_x;
    self->block_y = 2;
    exe6_block_to_pos();
    self->x += direction * (8 << 16);
    self->y += 28 << 16;
    self->z = 18 << 16;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    controller_update(self);
}

static void controller_phase_intro(Exe6Obj *self)
{
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    uint32_t block_x = (self->owner ^ self->owner_aux) * 5u + 1u;
    uint64_t coordinates = exe6_get_block_pos(block_x, 2);
    int32_t x = (int32_t)(uint32_t)coordinates + direction * (8 << 16);
    int32_t y = (int32_t)(uint32_t)(coordinates >> 32) + (27 << 16);

    for (uint32_t index = 0; index < sizeof(BURST_TABLE) / sizeof(BURST_TABLE[0]); ++index) {
        const struct BurstEntry *entry = &BURST_TABLE[index];
        int16_t vector_x = (int16_t)(entry->x * direction);
        int16_t vector_y = entry->y;
        uint32_t vector = ((uint32_t)(uint16_t)vector_x << 16)
            | (uint16_t)vector_y;
        Exe6ObjSpawnParameters spawn_parameters = {
            .variant = 0x1E,
            .subvariant = entry->palette,
            .removal_state = 1,
        };
        (void)spawn_burst(self, x, y, vector, spawn_parameters);
    }

    exe6_sound_req(0x107);
    self->timer = APPARITION_FRAMES;
    set_phase(self, CONTROLLER_PHASE_APPARITION);
}

static void controller_phase_apparition(Exe6Obj *self)
{
    if (self->timer == 7) {
        exe6_obj_char_init(
            0x80,
            BN67_SPRITE_GROUP(chaoslord_apparition_sprite),
            BN67_SPRITE_ID(chaoslord_apparition_sprite)
        );
        exe6_obj_no_shadow();
        self->animation_word = 0x0101;
        exe6_obj_dma_seq_set(1);
        exe6_obj_char_set();
        self->z += 8 << 16;
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }

    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(chaoslord_main_sprite),
        BN67_SPRITE_ID(chaoslord_main_sprite)
    );
    exe6_obj_shadow_set();
    exe6_obj_no_trans_flag_num_set(0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    self->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    self->z -= 8 << 16;
    exe6_obj_clt_set(0);
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->parent = spawn_ball(self);
    self->timer = APPROACH_FRAMES;
    exe6_obj_col_efc_set(packed_scale(self));
    set_phase(self, CONTROLLER_PHASE_APPROACH);
}

static void controller_phase_approach(Exe6Obj *self)
{
    uint16_t timer = self->timer;
    if (timer >= 0x34) {
        if (timer == 0x34) {
            exe6_sound_req(0x12A);
        }
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        if ((timer & 2u) != 0) {
            self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        } else if ((timer & 4u) != 0) {
            exe6_obj_col_efc_set(packed_scale(self));
        } else {
            exe6_obj_col_efc_set(0x00007FFF);
        }
    } else if (timer <= 0x24 && (timer & 3u) == 0) {
        struct ControllerWork *work = (struct ControllerWork *)self->work;
        work->scale_low = work->scale_low >= 2 ? work->scale_low - 2 : 0;
        work->scale_middle =
            work->scale_middle >= 2 ? work->scale_middle - 2 : 0;
        work->scale_high = work->scale_high >= 2 ? work->scale_high - 2 : 0;
        exe6_obj_col_efc_set(packed_scale(self));
    }

    self->z += 0x0000C000;
    self->x += self->velocity_x;
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }
    self->timer = ATTACK_FRAMES;
    set_phase(self, CONTROLLER_PHASE_ATTACK);
}

static void controller_phase_attack(Exe6Obj *self)
{
    if (self->timer == ATTACK_SPAWN_TIMER) {
        self->animation = 0x0E;
        exe6_sound_req(0x141);
        (void)spawn_attack_obj(self, 1);
    }
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }
    self->animation = 0x0F;
    self->timer = OUTRO_FRAMES;
    set_phase(self, CONTROLLER_PHASE_OUTRO);
}

static void controller_phase_outro(Exe6Obj *self)
{
    if (decrement_timer(&self->timer) < 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void controller_update(Exe6Obj *self)
{
    switch (self->phase) {
    case CONTROLLER_PHASE_INTRO:
        controller_phase_intro(self);
        break;
    case CONTROLLER_PHASE_APPARITION:
        controller_phase_apparition(self);
        break;
    case CONTROLLER_PHASE_APPROACH:
        controller_phase_approach(self);
        break;
    case CONTROLLER_PHASE_ATTACK:
        controller_phase_attack(self);
        break;
    default:
        controller_phase_outro(self);
        break;
    }
}

static void controller_destroy(Exe6Obj *self)
{
    (void)spawn_teardown(self);
    ball_request_destroy(self->parent);
    *self->completion = 0;
    exe6_obj_move_delete();
}

BN67_ENEMY(chaoslord_controller_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        controller_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        controller_update(self);
    } else {
        controller_destroy(self);
    }
    exe6_battle_obj_char_move2();
}

BN67_SUMMON_ATTACK(0x12E, chaoslord_attack_main)
{
    (void)spawn_parameters;
    Exe6Obj *controller = exe6_em_open(
        BN67_OBJ_ID(chaoslord_controller_main),
        exe6_obj_spawn_empty()
    );
    if (controller == NULL) {
        return;
    }
    controller->block_x = (uint8_t)block_x;
    controller->block_y = (uint8_t)block_y;
    controller->parameter = (uint8_t)parameter;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->completion = completion;
    *completion = 1;
}

static void attack_set_sprite(Exe6Obj *self, uint32_t stage)
{
    uint32_t group;
    uint32_t index;
    uint32_t animation;
    uint32_t palette;
    if (stage < 2) {
        const struct AttackSpriteEntry *entry = &ATTACK_SPRITE_TABLE[stage];
        group = entry->group;
        index = entry->index;
        animation = entry->animation;
        palette = entry->palette;
    } else {
        group = BN67_SPRITE_GROUP(chaoslord_main_sprite);
        index = BN67_SPRITE_ID(chaoslord_main_sprite);
        animation = stage == 2 ? 0x27 : 0x28;
        palette = 0;
    }
    exe6_obj_char_init(0x80, group, index);
    exe6_obj_no_shadow();
    self->animation_word = (uint16_t)(
        animation | (animation << 8)
    );
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
    exe6_obj_clt_set(palette);
}

static void attack_flash_target(Exe6Obj *self)
{
    if (self->variant != 0) {
        return;
    }
    uint16_t timer = self->aux_timer;
    self->aux_timer = (uint16_t)(timer - 1);
    if ((timer & 4u) == 0 && self->animation_state != 0) {
        exe6_block_flash(self->animation_state, self->removal_state);
    }
}

static void attack_apply_blocks(Exe6Obj *self)
{
    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    for (
        uint32_t index = 0;
        index < sizeof(IMPACT_BLOCK_PATTERN) / sizeof(IMPACT_BLOCK_PATTERN[0]);
        ++index
    ) {
        uint32_t block_x = (uint32_t)(
            (int32_t)self->animation_state
            + IMPACT_BLOCK_PATTERN[index].x * direction
        );
        uint32_t block_y = (uint32_t)(
            (int32_t)self->removal_state + IMPACT_BLOCK_PATTERN[index].y
        );
        uint32_t block_flags = exe6_block_status_get(block_x, block_y);
        if (self->variant == 0) {
            if ((block_flags & EXE6_BLOCK_FLAG_CRACKED) == 0) {
                exe6_block_crack_set(block_x, block_y);
            }
        } else if ((block_flags & EXE6_BLOCK_FLAG_SOLID) != 0) {
            exe6_block_out_set3(block_x, block_y);
        }
    }
}

static void spawn_impact_effect(Exe6Obj *self)
{
    Exe6Obj *effect = exe6_efc_open(
        0x24,
        exe6_obj_spawn_with_variant((uint8_t)(self->variant + 5u))
    );
    if (effect == NULL) {
        return;
    }

    effect->aux_timer = 1;
    struct ImpactWork *work = (struct ImpactWork *)effect->work;
    uint8_t *blocks = work->blocks;
    uint8_t count = 0;
    int32_t direction = 1 - (int32_t)(self->owner * 2u);
    for (
        uint32_t index = 0;
        index < sizeof(IMPACT_BLOCK_PATTERN) / sizeof(IMPACT_BLOCK_PATTERN[0]);
        ++index
    ) {
        uint32_t block_x = (uint32_t)(
            (int32_t)effect->block_x
            + IMPACT_BLOCK_PATTERN[index].x * direction
        );
        uint32_t block_y = (uint32_t)(
            (int32_t)effect->block_y + IMPACT_BLOCK_PATTERN[index].y
        );
        if (exe6_block_move_check(
                block_x,
                block_y,
                EXE6_BLOCK_FLAG_VALID,
                0
            ) != 0) {
            blocks[count++] = (uint8_t)(block_x | (block_y << 4));
        }
    }
    effect->subvariant = count;
}

static void attack_impact(Exe6Obj *self)
{
    spawn_impact_effect(self);
    Exe6Obj *damage = exe6_set_shl03_ev(
        self->animation_state,
        self->removal_state,
        0,
        0,
        DAMAGE_PROPERTIES,
        self->attack,
        3
    );
    if (damage != NULL) {
        damage->header_flags = (uint8_t)(
            (damage->header_flags
                & (uint8_t)~EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING)
            | (self->header_flags
                & EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING)
        );
    }
    attack_apply_blocks(self);
    exe6_camera_quake_set(3, 0x14);
    exe6_sound_req(0x142);
    (void)spawn_flash();
    self->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static void attack_fly(Exe6Obj *self)
{
    attack_flash_target(self);
    self->x += self->velocity_x;
    self->y += self->velocity_y;
    self->z -= self->velocity_z;
    if (decrement_timer(&self->timer) < 0) {
        attack_impact(self);
    }
}

static void attack_form(Exe6Obj *self)
{
    uint16_t timer = self->timer;
    uint16_t first = self->variant == 0 ? 0x3A : 0x58;
    uint16_t second = self->variant == 0 ? 0x32 : 0x50;
    uint16_t third = self->variant == 0 ? 0x2D : 0x4B;
    if (timer == first) {
        attack_set_sprite(self, 1);
        self->z += 18 << 16;
    } else if (timer == second) {
        attack_set_sprite(self, 2);
        exe6_sound_req(0x12A);
    } else if (timer == third) {
        attack_set_sprite(self, 3);
        self->z += 12 << 16;
    }

    if (timer == 0x0F) {
        if (self->variant != 0) {
            self->animation_state = (uint8_t)(
                ((self->owner ^ self->owner_aux) ^ 1u) * 3u + 2u
            );
            self->removal_state = 2;
        } else {
            self->animation_state = self->parent->block_x;
            self->removal_state = self->parent->block_y;
        }
    }

    attack_flash_target(self);
    if (decrement_timer(&self->timer) >= 0) {
        return;
    }

    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    uint64_t coordinates = exe6_get_block_pos(
        self->animation_state,
        self->removal_state
    );
    int32_t target_x = (int32_t)(uint32_t)coordinates;
    int32_t target_y = (int32_t)(uint32_t)(coordinates >> 32) + (8 << 16);
    self->velocity_x = (target_x - self->x) / (int32_t)BALL_FLIGHT_FRAMES;
    self->velocity_y = (target_y - self->y) / (int32_t)BALL_FLIGHT_FRAMES;
    self->velocity_z = self->z / (int32_t)BALL_FLIGHT_FRAMES;
    self->timer = BALL_FLIGHT_FRAMES;
    exe6_sound_req(0x11F);
    self->phase = ATTACK_PHASE_FLIGHT;
}

static void attack_update(Exe6Obj *self)
{
    if (exe6_battle_end_check() != 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    } else if (self->phase == ATTACK_PHASE_FORM) {
        attack_form(self);
    } else {
        attack_fly(self);
    }
}

static void attack_init(Exe6Obj *self)
{
    attack_set_sprite(self, 0);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->timer = self->variant == 0 ? 0x4B : 0x69;
    self->x += (int32_t)exe6_calc_pl_em_dir_spd_for(self) * (46 << 16);
    self->z += 50 << 16;
    self->y += 8 << 16;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    attack_update(self);
}

BN67_SHELL(chaoslord_attack_obj_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        attack_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        attack_update(self);
    } else {
        exe6_obj_move_delete();
    }
    exe6_battle_obj_char_move();
}

static void aura_appear_special(Exe6Obj *self)
{
    if (self->phase_timer_low == AURA_APPEAR_STEP_INIT) {
        self->phase_timer_low = AURA_APPEAR_STEP_TRANSITION;
        self->timer = 0x1E;
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
    if (self->phase_timer_low == AURA_APPEAR_STEP_TRANSITION) {
        uint32_t blend = self->timer >> 1;
        exe6_obj_mosaic_set(blend, blend);
        exe6_obj_bld_set(0x10u - blend);
        int32_t timer = self->timer;
        if (timer > 0x10) {
            --timer;
        }
        --timer;
        self->timer = (uint16_t)timer;
        if (timer >= 0) {
            return;
        }
        self->timer = 0x19;
        self->phase_timer_low = AURA_APPEAR_STEP_HOLD;
    }
    if (decrement_timer(&self->timer) < 0) {
        self->phase = AURA_PHASE_LIFESPAN;
    }
}

static void aura_appear_normal(Exe6Obj *self)
{
    if (self->phase_timer_low == AURA_APPEAR_STEP_INIT) {
        self->phase_timer_low = AURA_APPEAR_STEP_TRANSITION;
        self->timer = 0x22;
    }
    if (self->phase_timer_low == AURA_APPEAR_STEP_TRANSITION) {
        if (decrement_timer(&self->timer) >= 0) {
            return;
        }
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        self->timer = 0x0F;
        self->phase_timer_low = AURA_APPEAR_STEP_HOLD;
    }

    uint32_t scale = (uint32_t)self->timer << 1;
    uint32_t packed = (0x20u | scale);
    packed = (packed << 5) | scale;
    packed = (packed << 5) | scale;
    exe6_obj_col_efc_set(packed);
    if (decrement_timer(&self->timer) < 0) {
        self->phase = AURA_PHASE_LIFESPAN;
    }
}

static void aura_appear(Exe6Obj *self)
{
    if (self->animation == 0x0D) {
        aura_appear_special(self);
    } else {
        aura_appear_normal(self);
    }
}

static void aura_lifespan(Exe6Obj *self)
{
    --self->animation_state_word;
    if (self->animation_state_word == 0) {
        self->phase = AURA_PHASE_FADE;
    }
}

static void aura_fade(Exe6Obj *self)
{
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    if ((self->subvariant & 2u) == 0) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    }
    --self->subvariant;
    if (self->subvariant == 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void aura_update(Exe6Obj *self)
{
    if (self->phase == AURA_PHASE_APPEAR) {
        aura_appear(self);
    } else if (self->phase == AURA_PHASE_LIFESPAN) {
        aura_lifespan(self);
    } else {
        aura_fade(self);
    }
    exe6_obj_char_move();
}

static void aura_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(chaoslord_aura_sprite),
        BN67_SPRITE_ID(chaoslord_aura_sprite)
    );
    self->animation_word = (uint16_t)(self->variant | (self->variant << 8));
    exe6_obj_dma_seq_set(self->variant);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_no_shadow();
    exe6_obj_flip_set(exe6_enemy_flip_check());
    self->y += 26 << 16;
    self->z += 26 << 16;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    aura_update(self);
}

BN67_EFFECT(chaoslord_aura_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        aura_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        aura_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

static void burst_set_position(Exe6Obj *self)
{
    const int16_t *sine = (const int16_t *)chaoslord_trig_table;
    struct BurstWork *work = (struct BurstWork *)self->work;
    uint8_t angle = self->animation_state;
    int32_t radius = work->radius;
    self->x = work->center_x
        + ((int32_t)sine[0x40 + angle] * radius >> 8);
    self->z = -((int32_t)sine[angle] * radius >> 8);
    if (work->elevated != 0) {
        self->z += 27 << 16;
    }
}

static void burst_update(Exe6Obj *self)
{
    struct BurstWork *work = (struct BurstWork *)self->work;
    int32_t direction = -exe6_calc_pl_em_spd();
    uint8_t lifetime = self->variant;
    if (lifetime == 0x14) {
        self->owner_aux = (uint8_t)(((exe6_rand() & 7u) + 3u) * direction);
    } else if (lifetime == 0x0A) {
        self->owner_aux = (uint8_t)(
            self->owner_aux + ((exe6_rand() & 7u) + 0x0Au) * direction
        );
        work->radius -= work->radius_step;
    } else if (lifetime == 5) {
        self->owner_aux = (uint8_t)(
            self->owner_aux + ((exe6_rand() & 7u) + 0x10u) * direction
        );
        work->radius_step <<= 1;
        work->radius -= work->radius_step;
    } else if (lifetime < 0x14) {
        work->radius -= work->radius_step;
    } else {
        work->radius -= work->radius_step >> 2;
    }

    self->animation_state = (uint8_t)(
        self->animation_state - self->owner_aux
    );
    burst_set_position(self);
    --self->variant;
    if (self->variant == 0) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
}

static void burst_init(Exe6Obj *self)
{
    struct BurstWork *work = (struct BurstWork *)self->work;
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(chaoslord_aura_sprite),
        BN67_SPRITE_ID(chaoslord_aura_sprite)
    );
    self->animation = 0x15;
    exe6_obj_dma_seq_set(0x15);
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    exe6_obj_clt_set(self->subvariant);
    work->elevated = self->removal_state;
    work->center_x = self->x;

    int32_t vector_x = (int16_t)(work->vector >> 16);
    int32_t vector_y = (int16_t)work->vector;
    self->x += vector_x * 0x10000;
    self->z = -vector_y * 0x10000;
    uint32_t magnitude = (uint32_t)(
        vector_x * vector_x + vector_y * vector_y
    );
    work->radius = (int32_t)(integer_sqrt(magnitude) << 16);
    work->radius_step = work->radius / self->variant;
    self->animation_state = (uint8_t)exe6_calc_degree(
        vector_y * 0x10000,
        vector_x * 0x10000
    );
    self->owner_aux = (uint8_t)-exe6_calc_pl_em_spd();
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    if (work->elevated != 0) {
        self->z += 27 << 16;
    }
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
}

BN67_EFFECT(chaoslord_burst_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        burst_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        burst_update(self);
    } else {
        exe6_obj_move_delete();
    }
    exe6_battle_obj_char_move();
}

static void teardown_update(Exe6Obj *self)
{
    int32_t timer = decrement_timer(&self->timer);
    bool finished = timer == 0;
    if (!finished
        && (exe6_obj_seq_info_get()
            & EXE6_ANIMATION_FRAME_FLAG_END) != 0) {
        finished = (int16_t)self->timer <= 0;
    }
    if (finished) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
    }
    exe6_obj_char_move();
}

static void teardown_init(Exe6Obj *self)
{
    exe6_obj_char_init(
        0x80,
        BN67_SPRITE_GROUP(chaoslord_teardown_sprite),
        BN67_SPRITE_ID(chaoslord_teardown_sprite)
    );
    exe6_obj_char_set();
    exe6_obj_no_shadow();
    self->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_clt_set(self->animation_state);
    exe6_obj_flip_set(self->subvariant);
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    if (self->removal_state != 0) {
        exe6_obj_bld_set(0);
    }
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
}

BN67_EFFECT(chaoslord_teardown_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        teardown_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        teardown_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

static void flash_update(Exe6Obj *self)
{
    int32_t timer = decrement_timer(&self->aux_timer);
    if (timer < 0) {
        exe6_col_fade_kill(0x14);
        exe6_col_fade_kill(0x15);
        exe6_obj_move_delete();
        return;
    }

    uint32_t color = self->removal_state == 0 ? 0x00007FFF : 0x1F;
    exe6_col_fade_set(0, color, 0x0F, 0x14, EXE6_PALETTE_OBJ_OUTPUT_00);
    exe6_col_fade_set(0, color, 0x0F, 0x15, EXE6_PALETTE_BG_OUTPUT_00);
}

BN67_EFFECT(chaoslord_flash_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        self->aux_timer = self->subvariant;
        self->state_word = EXE6_OBJECT_STATE_ACTIVE;
        flash_update(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        flash_update(self);
    } else {
        exe6_obj_move_delete();
    }
}
