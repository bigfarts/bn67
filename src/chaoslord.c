#include "runtime.h"

BN6_SPRITE(chaoslord_main_sprite, "build/chaoslord-bass-sprite.bin");
BN6_SPRITE(chaoslord_aura_sprite, "build/chaoslord-aura-sprite.bin");
BN6_SPRITE(chaoslord_teardown_sprite, "build/chaoslord-teardown-sprite.bin");
BN6_SPRITE(chaoslord_apparition_sprite, "build/chaoslord-apparition-sprite.bin");

#if !FALZAR
BN6_INCBIN(chaoslord_icon, "build/chaoslord-icon.bin");
BN6_INCBIN(chaoslord_image, "build/chaoslord-image.bin");
BN6_INCBIN(chaoslord_palette, "build/chaoslord-palette.bin");
#endif
BN6_INCBIN(chaoslord_trig_table, "build/chaoslord-trig.bin");

static const uint8_t ACTIVE_STATE = 4;
static const uint8_t DESTROY_STATE = 8;
static const uint8_t BALL_ACTIVE_PHASE = 4;
static const uint8_t APPARITION_PHASE = 4;
static const uint8_t APPROACH_PHASE = 8;
static const uint8_t ATTACK_PHASE = 12;
static const uint8_t OUTRO_PHASE = 16;
static const uint8_t AURA_LIFESPAN_PHASE = 4;
static const uint8_t AURA_FADE_PHASE = 8;
static const uint16_t INTRO_FRAMES = 0x8E;
static const uint16_t APPARITION_FRAMES = 0x25;
static const uint16_t APPROACH_FRAMES = 0x48;
static const uint16_t ATTACK_FRAMES = 0x80;
static const uint16_t ATTACK_SPAWN_TIMER = 0x80;
static const uint16_t OUTRO_FRAMES = 0x28;
static const uint16_t BALL_FLIGHT_FRAMES = 0x0F;
static const Bn6PanelDamageProperties DAMAGE_PROPERTIES = {
    .region = BN6_COLLISION_REGION_CENTERED_3X3,
    .hit_effect = BN6_HIT_EFFECT_NONE,
    .target_collision_type = BN6_COLLISION_TYPE_STANDARD_TARGET,
    .self_collision_type = BN6_COLLISION_TYPE_15,
};
#if FALZAR
static const uintptr_t PANEL_DAMAGE_MAIN = 0x080C53C1;
#else
static const uintptr_t PANEL_DAMAGE_MAIN = 0x080C6C31;
#endif
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

struct PanelOffset {
    int8_t x;
    int8_t y;
};

struct ChaoslordControllerWork {
    uint32_t scale_low;                  // +0x60
    uint32_t scale_middle;               // +0x64
    uint32_t scale_high;                 // +0x68
};

struct ChaoslordBurstWork {
    int32_t radius;                      // +0x60
    int32_t radius_step;                 // +0x64
    int32_t center_x;                    // +0x68
    uint32_t reserved_6c;
    uint32_t vector;                     // +0x70
    uint32_t elevated;                   // +0x74
};

struct ChaoslordImpactWork {
    uint8_t panels[9];                   // +0x60
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

/* BN6 panel pattern 0x11, used to find room for the entrance. */
static const struct PanelOffset ENTRANCE_PANEL_PATTERN[] = {
    { 0, 0 },
    { 0, -1 },
    { 0, 1 },
    { 1, 0 },
    { 1, -1 },
    { 1, 1 },
};

/* BN6 panel pattern 0x0F, used by the native type-4 impact effect. */
static const struct PanelOffset IMPACT_PANEL_PATTERN[] = {
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

static bool timer_nonnegative_after_decrement(Object *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer >= 0;
}

static void set_phase(Object *self, uint8_t phase)
{
    self->phase = phase;
    self->phase_timer = 0;
}

static uint32_t packed_scale(Object *self)
{
    struct ChaoslordControllerWork *work =
        (struct ChaoslordControllerWork *)self->work;
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

static void ball_request_destroy(Object *ball)
{
    if (ball != NULL) {
        ball->state_word = DESTROY_STATE;
    }
}

static Object *spawn_ball(Object *controller)
{
    Bn6ObjectSpawnParameters spawn_parameters = {
        .variant = (uint8_t)BN6_SPRITE_GROUP(chaoslord_main_sprite),
        .subvariant = (uint8_t)BN6_SPRITE_ID(chaoslord_main_sprite),
        .animation_state = 1,
        .removal_state = 0x10,
    };
    Object *ball = bn6_spawn_type1(
        BN6_OBJECT_ID(chaoslord_ball_main), spawn_parameters
    );
    if (ball == NULL) {
        return NULL;
    }
    ball->parent = controller;
    ball->owner = controller->owner;
    return ball;
}

static void ball_phase_active(Object *self)
{
    bn6_self_sprite_copy_visibility(self->parent);
    bn6_self_object_update_dimming();
}

static void ball_update(Object *self)
{
    Object *controller = self->parent;
    uint8_t animation = (uint8_t)(
        controller->animation + self->removal_state
    );
    self->animation = animation;
    if (animation != self->palette) {
        bn6_self_sprite_set_animation(animation);
        bn6_self_sprite_load_animation_data();
    }

    self->x = controller->x;
    self->y = controller->y;
    self->z = controller->z;
    self->header_flags = (uint8_t)(
        (self->header_flags & (uint8_t)~BN6_OBJECT_FLAG_VISIBLE)
        | (controller->header_flags & BN6_OBJECT_FLAG_VISIBLE)
    );
    bn6_self_sprite_set_scale(bn6_sprite_get_scale(controller));
    bn6_self_sprite_copy_palette_bits(controller);
    bn6_self_sprite_copy_special_bits(controller);
    self->owner_aux = controller->owner_aux;
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());

    ball_phase_active(self);
}

static void ball_init(Object *self)
{
    bn6_self_sprite_load(0x80, self->variant, self->subvariant);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    bn6_self_sprite_set_palette(bn6_sprite_get_palette(self->parent));
    self->animation_word = (uint16_t)(
        self->parent->animation + self->removal_state
    );
    bn6_self_sprite_set_animation(self->animation);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    set_phase(self, BALL_ACTIVE_PHASE);
    self->state_word = ACTIVE_STATE;
    ball_update(self);
}

BN6_OBJECT1(chaoslord_ball_main)
{
    if (self->state == 0) {
        ball_init(self);
    } else if (self->state == ACTIVE_STATE) {
        ball_update(self);
    } else {
        bn6_self_object_free();
    }
}

static Object *spawn_aura(
    Object *controller,
    int32_t x,
    int32_t y,
    int32_t z,
    Bn6ObjectSpawnParameters spawn_parameters
)
{
    Object *aura = bn6_spawn_type4_at(
        BN6_OBJECT_ID(chaoslord_aura_main),
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

static Object *spawn_burst(
    Object *controller,
    int32_t x,
    int32_t y,
    uint32_t vector,
    Bn6ObjectSpawnParameters spawn_parameters
)
{
    Object *burst = bn6_spawn_type4_at(
        BN6_OBJECT_ID(chaoslord_burst_main),
        x,
        y,
        0,
        spawn_parameters
    );
    if (burst == NULL) {
        return NULL;
    }
    burst->parent = controller;
    struct ChaoslordBurstWork *work =
        (struct ChaoslordBurstWork *)burst->work;
    work->vector = vector;
    burst->owner_word = controller->owner_word;
    burst->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING;
    return burst;
}

static Object *spawn_teardown(Object *controller)
{
    Object *effect = bn6_spawn_type4_at(
        BN6_OBJECT_ID(chaoslord_teardown_main),
        controller->x,
        controller->y,
        controller->z + (16 << 16),
        bn6_object_spawn_with_variant(0x12)
    );
    if (effect == NULL) {
        return NULL;
    }
    effect->owner_word = controller->owner_word;
    return effect;
}

static Object *spawn_flash(void)
{
    Bn6ObjectSpawnParameters spawn_parameters = {
        .variant = 1,
        .subvariant = 4,
        .animation_state = 1,
    };
    Object *flash = bn6_spawn_type4(
        BN6_OBJECT_ID(chaoslord_flash_main), spawn_parameters
    );
    if (flash == NULL) {
        return NULL;
    }
    flash->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_PAUSE
        | BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING;
    return flash;
}

static Object *spawn_attack_object(
    Object *controller,
    uint8_t variant
)
{
    Object *attack = bn6_spawn_type3(
        BN6_OBJECT_ID(chaoslord_attack_object_main),
        controller->x,
        controller->y,
        controller->z,
        bn6_object_spawn_with_variant(variant)
    );
    if (attack == NULL) {
        return NULL;
    }
    attack->parameter = 0;
    attack->attack = controller->attack;
    attack->parent = NULL;
    attack->owner_word = controller->owner_word;
    attack->header_flags |= BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING;
    return attack;
}

static void delete_live_objects(void)
{
    Object **objects = bn6_battle_context()->live_objects;
    for (uint32_t index = 0; index < 8; ++index) {
        if (objects[index] != NULL) {
            objects[index]->hp = 0;
        }
    }
}

static void controller_update(Object *self);

static bool entrance_has_panel(
    uint32_t center_x,
    uint32_t center_y,
    uint32_t area,
    uint32_t side
)
{
    int32_t direction = 1 - (int32_t)(side * 2u);
    for (
        uint32_t index = 0;
        index < sizeof(ENTRANCE_PANEL_PATTERN) / sizeof(ENTRANCE_PANEL_PATTERN[0]);
        ++index
    ) {
        uint32_t panel_x = (uint32_t)(
            (int32_t)center_x + ENTRANCE_PANEL_PATTERN[index].x * direction
        );
        uint32_t panel_y = (uint32_t)(
            (int32_t)center_y + ENTRANCE_PANEL_PATTERN[index].y
        );
        if (bn6_panel_matches_flags(panel_x, panel_y, area, 0) != 0) {
            return true;
        }
    }
    return false;
}

static void controller_init(Object *self)
{
    bn6_play_sound(0x94);
    delete_live_objects();

    uint32_t side = self->owner ^ self->owner_aux;
    uint32_t search_x = side * 5u + 1u;
    uint32_t search_area = self->owner == 0
        ? BN6_PANEL_FLAG_SIDE_1_COLLISION
        : BN6_PANEL_FLAG_SIDE_0_COLLISION;
    if (entrance_has_panel(search_x, 2, search_area, side)) {
        self->state_word = DESTROY_STATE;
        return;
    }

    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    uint32_t panel_x = (self->owner ^ self->owner_aux) * 5u + 1u;
    uint64_t coordinates = bn6_panel_to_coords(panel_x, 2);
    int32_t x = (int32_t)(uint32_t)coordinates + direction * (8 << 16);
    int32_t y = (int32_t)(uint32_t)(coordinates >> 32);
    (void)spawn_aura(self, x, y, 0, (Bn6ObjectSpawnParameters){
        .variant = 0x0A,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Bn6ObjectSpawnParameters){
        .variant = 0x07,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Bn6ObjectSpawnParameters){
        .variant = 0x04,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Bn6ObjectSpawnParameters){
        .variant = 0x01,
        .subvariant = 0x0A,
        .animation_state = 0x5A,
    });
    (void)spawn_aura(self, x, y, 0, (Bn6ObjectSpawnParameters){
        .variant = 0x0D,
        .subvariant = 0x0A,
        .animation_state = 0x7C,
        .removal_state = 0x01,
    });

    self->timer = INTRO_FRAMES;
    self->velocity_x = direction * 0x00009D89;
    struct ChaoslordControllerWork *work =
        (struct ChaoslordControllerWork *)self->work;
    work->scale_low = 5;
    work->scale_middle = 1;
    work->scale_high = 0x17;
    self->panel_x = (uint8_t)panel_x;
    self->panel_y = 2;
    bn6_self_object_set_coords();
    self->x += direction * (8 << 16);
    self->y += 28 << 16;
    self->z = 18 << 16;
    self->state_word = ACTIVE_STATE;
    controller_update(self);
}

static void controller_phase_intro(Object *self)
{
    if (timer_nonnegative_after_decrement(self)) {
        return;
    }

    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    uint32_t panel_x = (self->owner ^ self->owner_aux) * 5u + 1u;
    uint64_t coordinates = bn6_panel_to_coords(panel_x, 2);
    int32_t x = (int32_t)(uint32_t)coordinates + direction * (8 << 16);
    int32_t y = (int32_t)(uint32_t)(coordinates >> 32) + (27 << 16);

    for (uint32_t index = 0; index < sizeof(BURST_TABLE) / sizeof(BURST_TABLE[0]); ++index) {
        const struct BurstEntry *entry = &BURST_TABLE[index];
        int16_t vector_x = (int16_t)(entry->x * direction);
        int16_t vector_y = entry->y;
        uint32_t vector = ((uint32_t)(uint16_t)vector_x << 16)
            | (uint16_t)vector_y;
        Bn6ObjectSpawnParameters spawn_parameters = {
            .variant = 0x1E,
            .subvariant = entry->palette,
            .removal_state = 1,
        };
        (void)spawn_burst(self, x, y, vector, spawn_parameters);
    }

    bn6_play_sound(0x107);
    self->timer = APPARITION_FRAMES;
    set_phase(self, APPARITION_PHASE);
}

static void controller_phase_apparition(Object *self)
{
    if (self->timer == 7) {
        bn6_self_sprite_load(
            0x80,
            BN6_SPRITE_GROUP(chaoslord_apparition_sprite),
            BN6_SPRITE_ID(chaoslord_apparition_sprite)
        );
        bn6_self_sprite_no_shadow();
        self->animation_word = 0x0101;
        bn6_self_sprite_set_animation(1);
        bn6_self_sprite_load_animation_data();
        self->z += 8 << 16;
        self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    }
    if (timer_nonnegative_after_decrement(self)) {
        return;
    }

    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(chaoslord_main_sprite),
        BN6_SPRITE_ID(chaoslord_main_sprite)
    );
    bn6_self_sprite_enable_shadow();
    bn6_self_sprite_hide_piece(0);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    self->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    self->z -= 8 << 16;
    bn6_self_sprite_set_palette(0);
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->parent = spawn_ball(self);
    self->timer = APPROACH_FRAMES;
    bn6_self_sprite_set_scale(packed_scale(self));
    set_phase(self, APPROACH_PHASE);
}

static void controller_phase_approach(Object *self)
{
    uint16_t timer = self->timer;
    if (timer >= 0x34) {
        if (timer == 0x34) {
            bn6_play_sound(0x12A);
        }
        self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
        if ((timer & 2u) != 0) {
            self->header_flags &= (uint8_t)~BN6_OBJECT_FLAG_VISIBLE;
        } else if ((timer & 4u) != 0) {
            bn6_self_sprite_set_scale(packed_scale(self));
        } else {
            bn6_self_sprite_set_scale(0x00007FFF);
        }
    } else if (timer <= 0x24 && (timer & 3u) == 0) {
        struct ChaoslordControllerWork *work =
            (struct ChaoslordControllerWork *)self->work;
        work->scale_low = work->scale_low >= 2 ? work->scale_low - 2 : 0;
        work->scale_middle =
            work->scale_middle >= 2 ? work->scale_middle - 2 : 0;
        work->scale_high = work->scale_high >= 2 ? work->scale_high - 2 : 0;
        bn6_self_sprite_set_scale(packed_scale(self));
    }

    self->z += 0x0000C000;
    self->x += self->velocity_x;
    if (timer_nonnegative_after_decrement(self)) {
        return;
    }
    self->timer = ATTACK_FRAMES;
    set_phase(self, ATTACK_PHASE);
}

static void controller_phase_attack(Object *self)
{
    if (self->timer == ATTACK_SPAWN_TIMER) {
        self->animation = 0x0E;
        bn6_play_sound(0x141);
        (void)spawn_attack_object(self, 1);
    }
    if (timer_nonnegative_after_decrement(self)) {
        return;
    }
    self->animation = 0x0F;
    self->timer = OUTRO_FRAMES;
    set_phase(self, OUTRO_PHASE);
}

static void controller_phase_outro(Object *self)
{
    if (!timer_nonnegative_after_decrement(self)) {
        self->state_word = DESTROY_STATE;
    }
}

static void controller_update(Object *self)
{
    switch (self->phase) {
    case 0:
        controller_phase_intro(self);
        break;
    case APPARITION_PHASE:
        controller_phase_apparition(self);
        break;
    case APPROACH_PHASE:
        controller_phase_approach(self);
        break;
    case ATTACK_PHASE:
        controller_phase_attack(self);
        break;
    default:
        controller_phase_outro(self);
        break;
    }
}

static void controller_destroy(Object *self)
{
    (void)spawn_teardown(self);
    ball_request_destroy(self->parent);
    *self->completion = 0;
    bn6_self_object_free();
}

BN6_OBJECT1(chaoslord_controller_main)
{
    if (self->state == 0) {
        controller_init(self);
    } else if (self->state == ACTIVE_STATE) {
        controller_update(self);
    } else {
        controller_destroy(self);
    }
    bn6_self_object_update_dimming();
}

BN6_SUMMON_ATTACK(0x12E, chaoslord_attack_main)
{
    (void)spawn_parameters;
    Object *controller = bn6_spawn_type1(
        BN6_OBJECT_ID(chaoslord_controller_main),
        bn6_object_spawn_empty()
    );
    if (controller == NULL) {
        return;
    }
    controller->panel_x = (uint8_t)panel_x;
    controller->panel_y = (uint8_t)panel_y;
    controller->parameter = (uint8_t)parameter;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->completion = completion;
    *completion = 1;
}

static void attack_set_sprite(Object *self, uint32_t stage)
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
        group = BN6_SPRITE_GROUP(chaoslord_main_sprite);
        index = BN6_SPRITE_ID(chaoslord_main_sprite);
        animation = stage == 2 ? 0x27 : 0x28;
        palette = 0;
    }
    bn6_self_sprite_load(0x80, group, index);
    bn6_self_sprite_no_shadow();
    self->animation_word = (uint16_t)(
        animation | (animation << 8)
    );
    bn6_self_sprite_set_animation(animation);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_set_palette(palette);
}

static void attack_flash_target(Object *self)
{
    if (self->variant != 0) {
        return;
    }
    uint16_t timer = self->aux_timer;
    self->aux_timer = (uint16_t)(timer - 1);
    if ((timer & 4u) == 0 && self->animation_state != 0) {
        bn6_panel_set_flash(self->animation_state, self->removal_state);
    }
}

static void attack_apply_panels(Object *self)
{
    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    for (
        uint32_t index = 0;
        index < sizeof(IMPACT_PANEL_PATTERN) / sizeof(IMPACT_PANEL_PATTERN[0]);
        ++index
    ) {
        uint32_t panel_x = (uint32_t)(
            (int32_t)self->animation_state
            + IMPACT_PANEL_PATTERN[index].x * direction
        );
        uint32_t panel_y = (uint32_t)(
            (int32_t)self->removal_state + IMPACT_PANEL_PATTERN[index].y
        );
        uint32_t panel_flags = bn6_panel_get_flags(panel_x, panel_y);
        if (self->variant == 0) {
            if ((panel_flags & BN6_PANEL_FLAG_CRACKED) == 0) {
                bn6_panel_crack_from_solid(panel_x, panel_y);
            }
        } else if ((panel_flags & BN6_PANEL_FLAG_SOLID) != 0) {
            bn6_panel_crack(panel_x, panel_y);
        }
    }
}

static void spawn_impact_effect(Object *self)
{
    Object *effect = bn6_spawn_type4(
        0x24,
        bn6_object_spawn_with_variant((uint8_t)(self->variant + 5u))
    );
    if (effect == NULL) {
        return;
    }

    effect->aux_timer = 1;
    struct ChaoslordImpactWork *work =
        (struct ChaoslordImpactWork *)effect->work;
    uint8_t *panels = work->panels;
    uint8_t count = 0;
    int32_t direction = 1 - (int32_t)(self->owner * 2u);
    for (
        uint32_t index = 0;
        index < sizeof(IMPACT_PANEL_PATTERN) / sizeof(IMPACT_PANEL_PATTERN[0]);
        ++index
    ) {
        uint32_t panel_x = (uint32_t)(
            (int32_t)effect->panel_x
            + IMPACT_PANEL_PATTERN[index].x * direction
        );
        uint32_t panel_y = (uint32_t)(
            (int32_t)effect->panel_y + IMPACT_PANEL_PATTERN[index].y
        );
        if (bn6_panel_matches_flags(
                panel_x,
                panel_y,
                BN6_PANEL_FLAG_VALID,
                0
            ) != 0) {
            panels[count++] = (uint8_t)(panel_x | (panel_y << 4));
        }
    }
    effect->subvariant = count;
}

static void attack_impact(Object *self)
{
    spawn_impact_effect(self);
    Object *damage = bn6_spawn_panel_damage(
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
                & (uint8_t)~BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING)
            | (self->header_flags
                & BN6_OBJECT_FLAG_UPDATE_DURING_DIMMING)
        );
        bn6_object_invoke(damage, PANEL_DAMAGE_MAIN);
    }
    attack_apply_panels(self);
    bn6_screen_shake_set(3, 0x14);
    bn6_play_sound(0x142);
    (void)spawn_flash();
    self->state_word = DESTROY_STATE;
}

static void attack_fly(Object *self)
{
    attack_flash_target(self);
    self->x += self->velocity_x;
    self->y += self->velocity_y;
    self->z -= self->velocity_z;
    if (!timer_nonnegative_after_decrement(self)) {
        attack_impact(self);
    }
}

static void attack_form(Object *self)
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
        bn6_play_sound(0x12A);
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
            self->animation_state = self->parent->panel_x;
            self->removal_state = self->parent->panel_y;
        }
    }

    attack_flash_target(self);
    if (timer_nonnegative_after_decrement(self)) {
        return;
    }

    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    uint64_t coordinates = bn6_panel_to_coords(
        self->animation_state,
        self->removal_state
    );
    int32_t target_x = (int32_t)(uint32_t)coordinates;
    int32_t target_y = (int32_t)(uint32_t)(coordinates >> 32) + (8 << 16);
    self->velocity_x = (target_x - self->x) / (int32_t)BALL_FLIGHT_FRAMES;
    self->velocity_y = (target_y - self->y) / (int32_t)BALL_FLIGHT_FRAMES;
    self->velocity_z = self->z / (int32_t)BALL_FLIGHT_FRAMES;
    self->timer = BALL_FLIGHT_FRAMES;
    bn6_play_sound(0x11F);
    self->phase = BALL_ACTIVE_PHASE;
}

static void attack_update(Object *self)
{
    if (bn6_battle_is_over() != 0) {
        self->state_word = DESTROY_STATE;
    } else if (self->phase == 0) {
        attack_form(self);
    } else {
        attack_fly(self);
    }
}

static void attack_init(Object *self)
{
    attack_set_sprite(self, 0);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->timer = self->variant == 0 ? 0x4B : 0x69;
    self->x += (int32_t)bn6_object_front_direction_for(self) * (46 << 16);
    self->z += 50 << 16;
    self->y += 8 << 16;
    self->state_word = ACTIVE_STATE;
    attack_update(self);
}

BN6_OBJECT3(chaoslord_attack_object_main)
{
    if (self->state == 0) {
        attack_init(self);
    } else if (self->state == ACTIVE_STATE) {
        attack_update(self);
    } else {
        bn6_self_object_free();
    }
    bn6_self_object_update();
}

static void aura_appear_special(Object *self)
{
    if (self->phase_timer_low == 0) {
        self->phase_timer_low = 1;
        self->timer = 0x1E;
        self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    }
    if (self->phase_timer_low == 1) {
        uint32_t blend = self->timer >> 1;
        bn6_self_sprite_set_blend(blend, blend);
        bn6_self_sprite_set_blend_mode(0x10u - blend);
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
        self->phase_timer_low = 2;
    }
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase = AURA_LIFESPAN_PHASE;
    }
}

static void aura_appear_normal(Object *self)
{
    if (self->phase_timer_low == 0) {
        self->phase_timer_low = 1;
        self->timer = 0x22;
    }
    if (self->phase_timer_low == 1) {
        if (timer_nonnegative_after_decrement(self)) {
            return;
        }
        self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
        self->timer = 0x0F;
        self->phase_timer_low = 2;
    }

    uint32_t scale = (uint32_t)self->timer << 1;
    uint32_t packed = (0x20u | scale);
    packed = (packed << 5) | scale;
    packed = (packed << 5) | scale;
    bn6_self_sprite_set_scale(packed);
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase = AURA_LIFESPAN_PHASE;
    }
}

static void aura_appear(Object *self)
{
    if (self->animation == 0x0D) {
        aura_appear_special(self);
    } else {
        aura_appear_normal(self);
    }
}

static void aura_lifespan(Object *self)
{
    --self->animation_state_word;
    if (self->animation_state_word == 0) {
        self->phase = AURA_FADE_PHASE;
    }
}

static void aura_fade(Object *self)
{
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    if ((self->subvariant & 2u) == 0) {
        self->header_flags &= (uint8_t)~BN6_OBJECT_FLAG_VISIBLE;
    }
    --self->subvariant;
    if (self->subvariant == 0) {
        self->state_word = DESTROY_STATE;
    }
}

static void aura_update(Object *self)
{
    if (self->phase == 0) {
        aura_appear(self);
    } else if (self->phase == AURA_LIFESPAN_PHASE) {
        aura_lifespan(self);
    } else {
        aura_fade(self);
    }
    bn6_self_sprite_update();
}

static void aura_init(Object *self)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(chaoslord_aura_sprite),
        BN6_SPRITE_ID(chaoslord_aura_sprite)
    );
    self->animation_word = (uint16_t)(self->variant | (self->variant << 8));
    bn6_self_sprite_set_animation(self->variant);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    bn6_self_sprite_no_shadow();
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());
    self->y += 26 << 16;
    self->z += 26 << 16;
    self->state_word = ACTIVE_STATE;
    aura_update(self);
}

BN6_OBJECT4(chaoslord_aura_main)
{
    if (self->state == 0) {
        aura_init(self);
    } else if (self->state == ACTIVE_STATE) {
        aura_update(self);
    } else {
        bn6_self_object_free();
    }
}

static void burst_set_position(Object *self)
{
    const int16_t *sine = (const int16_t *)chaoslord_trig_table;
    struct ChaoslordBurstWork *work =
        (struct ChaoslordBurstWork *)self->work;
    uint8_t angle = self->animation_state;
    int32_t radius = work->radius;
    self->x = work->center_x
        + ((int32_t)sine[0x40 + angle] * radius >> 8);
    self->z = -((int32_t)sine[angle] * radius >> 8);
    if (work->elevated != 0) {
        self->z += 27 << 16;
    }
}

static void burst_update(Object *self)
{
    struct ChaoslordBurstWork *work =
        (struct ChaoslordBurstWork *)self->work;
    int32_t direction = -bn6_self_object_side_direction();
    uint8_t lifetime = self->variant;
    if (lifetime == 0x14) {
        self->owner_aux = (uint8_t)(((bn6_rng_next() & 7u) + 3u) * direction);
    } else if (lifetime == 0x0A) {
        self->owner_aux = (uint8_t)(
            self->owner_aux + ((bn6_rng_next() & 7u) + 0x0Au) * direction
        );
        work->radius -= work->radius_step;
    } else if (lifetime == 5) {
        self->owner_aux = (uint8_t)(
            self->owner_aux + ((bn6_rng_next() & 7u) + 0x10u) * direction
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
        self->state_word = DESTROY_STATE;
    }
}

static void burst_init(Object *self)
{
    struct ChaoslordBurstWork *work =
        (struct ChaoslordBurstWork *)self->work;
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(chaoslord_aura_sprite),
        BN6_SPRITE_ID(chaoslord_aura_sprite)
    );
    self->animation = 0x15;
    bn6_self_sprite_set_animation(0x15);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    bn6_self_sprite_set_palette(self->subvariant);
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
    self->animation_state = (uint8_t)bn6_angle_from_vector(
        vector_y * 0x10000,
        vector_x * 0x10000
    );
    self->owner_aux = (uint8_t)-bn6_self_object_side_direction();
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    if (work->elevated != 0) {
        self->z += 27 << 16;
    }
    self->state_word = ACTIVE_STATE;
}

BN6_OBJECT4(chaoslord_burst_main)
{
    if (self->state == 0) {
        burst_init(self);
    } else if (self->state == ACTIVE_STATE) {
        burst_update(self);
    } else {
        bn6_self_object_free();
    }
    bn6_self_object_update();
}

static void teardown_update(Object *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    bool finished = timer == 0;
    if (!finished
        && (bn6_self_sprite_get_frame_flags()
            & BN6_ANIMATION_FRAME_FLAG_END) != 0) {
        finished = (int16_t)self->timer <= 0;
    }
    if (finished) {
        self->header_flags &= (uint8_t)~BN6_OBJECT_FLAG_VISIBLE;
        self->state_word = DESTROY_STATE;
    }
    bn6_self_sprite_update();
}

static void teardown_init(Object *self)
{
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(chaoslord_teardown_sprite),
        BN6_SPRITE_ID(chaoslord_teardown_sprite)
    );
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    self->animation_word = 0;
    bn6_self_sprite_set_animation(0);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_update();
    bn6_self_sprite_set_palette(self->animation_state);
    bn6_self_sprite_set_flip(self->subvariant);
    self->header_flags |= BN6_OBJECT_FLAG_VISIBLE;
    if (self->removal_state != 0) {
        bn6_self_sprite_set_blend_mode(0);
    }
    self->state_word = ACTIVE_STATE;
}

BN6_OBJECT4(chaoslord_teardown_main)
{
    if (self->state == 0) {
        teardown_init(self);
    } else if (self->state == ACTIVE_STATE) {
        teardown_update(self);
    } else {
        bn6_self_object_free();
    }
}

static void flash_update(Object *self)
{
    int32_t timer = (int32_t)self->aux_timer - 1;
    self->aux_timer = (uint16_t)timer;
    if (timer < 0) {
        bn6_palette_restore(0x14);
        bn6_palette_restore(0x15);
        bn6_self_object_free();
        return;
    }

    uint32_t color = self->removal_state == 0 ? 0x00007FFF : 0x1F;
    bn6_palette_write(0, color, 0x0F, 0x14, BN6_PALETTE_OBJ_OUTPUT_00);
    bn6_palette_write(0, color, 0x0F, 0x15, BN6_PALETTE_BG_OUTPUT_00);
}

BN6_OBJECT4(chaoslord_flash_main)
{
    if (self->state == 0) {
        self->aux_timer = self->subvariant;
        self->state_word = ACTIVE_STATE;
        flash_update(self);
    } else if (self->state == ACTIVE_STATE) {
        flash_update(self);
    } else {
        bn6_self_object_free();
    }
}
