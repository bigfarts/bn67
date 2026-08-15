#include "common.h"
#include "runtime.h"

/* BN5 ProtoMan's base Django, coffin, and sunlight archives. */
BN67_SPRITE(django_battle_sprite, "build/django-battle-sprite.bin");
BN67_SPRITE(django_sun_sprite, "build/django-sun-sprite.bin");
BN67_SPRITE(django_coffin_sprite, "build/django-coffin-sprite.bin");

BN67_INCBIN(django_icon, "build/django-icon.bin");
BN67_INCBIN(django_image, "build/django-image.bin");
BN67_INCBIN(django_palette, "build/django-palette.bin");
BN67_ASM_RESOURCE(
    django2_palette,
    ".incbin \"build/django-palette.bin\",0,0x1A\n"
    ".short 0x6BF7,0x5BAD,0x4743\n"
);
BN67_INCBIN(django3_palette, "build/django3-palette.bin");

#define DJANGO_RECORD(                                                       \
    chip_id, second_code, rarity_value, mb_value, power_value, palette_value \
)                                                                            \
    BN67_CHIP_RECORD(chip_id) {                                              \
        .codes = {                                                           \
            EXE6_CHIP_CODE_D, second_code, EXE6_CHIP_CODE_NONE,             \
            EXE6_CHIP_CODE_NONE,                                             \
        },                                                                   \
        .attack_element = 0,                                                 \
        .rarity = rarity_value,                                              \
        .element = EXE6_CHIP_ELEMENT_NULL,                                   \
        .chip_class = EXE6_CHIP_CLASS_MEGA,                                  \
        .mb = mb_value,                                                      \
        .behavior = {                                                        \
            .effect_flags = EXE6_CHIP_EFFECT_FLAG_DIMMING |                 \
                            EXE6_CHIP_EFFECT_FLAG_ATTACK |                   \
                            EXE6_CHIP_EFFECT_FLAG_NAVI |                     \
                            EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE,         \
            .counter_settings = 0x8A,                                       \
            .family = BN67_ATTACK_FAMILY(django_attack_main),               \
            .subfamily = BN67_ATTACK_SUBFAMILY(django_attack_main),         \
            .dark_soul_usage = 0,                                           \
            .unknown_0e = 0,                                                \
            .lock_on = 0,                                                    \
            .object_spawn = {0},                                             \
            .delay = 0,                                                      \
        },                                                                   \
        .library_number = 0x2B + ((chip_id) - 0x116),                       \
        .library_flags = 0,                                                  \
        .library_lock_on_type = 0,                                           \
        .alphabetical_sort = 0,                                              \
        .power = power_value,                                                \
        .library_sort_order = chip_id,                                       \
        .library_gate_usage = 0x01,                                          \
        .dark_chip_id = UINT8_MAX,                                           \
        .icon = django_icon,                                                 \
        .image = django_image,                                               \
        .palette = palette_value,                                            \
    }

/* All three restored slots use the base attack with fixed, scaled power. */
DJANGO_RECORD(
    0x116, EXE6_CHIP_CODE_ASTERISK, 2, 30, 130, django_palette
);
DJANGO_RECORD(
    0x117, EXE6_CHIP_CODE_NONE, 3, 70, 180, django2_palette
);
DJANGO_RECORD(
    0x118, EXE6_CHIP_CODE_NONE, 4, 90, 260, django3_palette
);

enum ActorPhase {
    ACTOR_PHASE_APPEAR,
    ACTOR_PHASE_WAIT = 4,
    ACTOR_PHASE_READY = 8,
    ACTOR_PHASE_BUILDUP = 12,
    ACTOR_PHASE_PAUSE = 16,
    ACTOR_PHASE_SUNLIGHT = 20,
    ACTOR_PHASE_COOLDOWN = 24,
    ACTOR_PHASE_EXIT = 28,
};

enum UpdateStep {
    UPDATE_STEP_INIT,
    UPDATE_STEP_ACTIVE = 4,
};

enum LightVariant {
    LIGHT_VARIANT_CHARGE,
    LIGHT_VARIANT_SUN_CONTROLLER,
    LIGHT_VARIANT_SUN_FIRST,
    LIGHT_VARIANT_SUN_SECOND,
    LIGHT_VARIANT_SUN_THIRD,
};

static const uint16_t APPEAR_FRAMES = 4;
static const uint16_t WAIT_FRAMES = 20;
static const uint16_t READY_FRAMES = 8;
static const uint16_t BUILDUP_LAST_FRAME = 60;
static const uint16_t ROCK_SPAWN_FRAME = 20;
static const uint16_t PAUSE_FRAMES = 20;
static const uint16_t COOLDOWN_FRAMES = 30;
static const uint16_t EXIT_FRAMES = 4;
static const uint16_t CHARGE_SOUND_PERIOD = 16;
/* BN5 advances the sunlight orbit by eight angle units each frame. */
static const uint8_t SUNLIGHT_ORBIT_STEP = 8;
static const uint8_t SUNLIGHT_PULSE_DAMAGE = 1;
/* BN6's native "Rock (Gregar drops these)" sprite. */
static const uint8_t GREGAR_ROCK_SPRITE_GROUP = 0x10;
static const uint8_t GREGAR_ROCK_SPRITE_ID = 0x05;
static const int8_t ROCK_BLOCK_OFFSETS[4][2] = {
    {-1, 0}, {1, 0}, {0, -1}, {0, 1},
};
static const Exe6BlockDamageProperties SUNLIGHT_DAMAGE_PROPERTIES = {
    .region = EXE6_HIT_REGION_CENTERED_3X3,
    .hit_effect = EXE6_HIT_EFFECT_NONE,
    .target_hit_type = EXE6_HIT_TYPE_STANDARD_TARGET,
    .self_hit_type = EXE6_HIT_TYPE_17,
};

struct ActorWork {
    uint8_t has_target;
    uint8_t rocks_spawned;
    uint8_t sun_spawned;
    uint8_t target_was_visible;
    uint16_t damage_remaining;
    uint8_t sunlight_sound_timer;
    Exe6Obj *target;
};

_Static_assert(
    sizeof(struct ActorWork) <= sizeof(((Exe6Obj *)0)->work),
    "Django work must fit in an enemy object"
);

static bool object_is_active(const Exe6Obj *object)
{
    return object != NULL
        && (object->header_flags & EXE6_OBJ_FLAG_ACTIVE) != 0;
}

static bool actor_is_active(const Exe6Obj *actor)
{
    return object_is_active(actor)
        && actor->state == EXE6_OBJECT_STATE_ACTIVE;
}

/* BN5 Django selects the closest opposing unit in his row. */
static Exe6Obj *find_target_in_row(
    Exe6Obj *self,
    uint8_t *target_x,
    uint8_t *target_y
)
{
    Exe6Runtime *runtime = exe6_runtime();
    if (runtime == NULL || runtime->battle_context == NULL) {
        return NULL;
    }

    int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
    int32_t closest_distance = 7;
    Exe6Obj *found = NULL;
    Exe6BattleContext *battle = runtime->battle_context;
    uint32_t opposing_side = self->owner ^ 1u;
    for (size_t index = 0; index < 4; ++index) {
        Exe6Obj *target = battle->active_units[opposing_side][index];
        if (!object_is_active(target) || target->block_y != self->block_y) {
            continue;
        }
        int32_t distance =
            ((int32_t)target->block_x - (int32_t)self->block_x) * direction;
        if (distance <= 0 || distance >= closest_distance) {
            continue;
        }
        closest_distance = distance;
        *target_x = target->block_x;
        *target_y = target->block_y;
        found = target;
    }
    return found;
}

static void hide_target(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    Exe6Obj *target = work->target;
    if (!object_is_active(target)) {
        return;
    }
    if ((target->header_flags & EXE6_OBJ_FLAG_VISIBLE) != 0) {
        work->target_was_visible = 1;
    }
    target->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
}

static void restore_target(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    if (work->target_was_visible != 0 && object_is_active(work->target)) {
        work->target->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
    work->target_was_visible = 0;
}

static Exe6Obj *spawn_effect(
    Exe6Obj *actor,
    uint8_t object_id,
    uint8_t variant,
    uint8_t block_x,
    uint8_t block_y
)
{
    Exe6Obj *effect = exe6_efc_open(
        object_id,
        exe6_obj_spawn_with_variant(variant)
    );
    if (effect == NULL) {
        return NULL;
    }
    effect->block_x = block_x;
    effect->block_y = block_y;
    effect->owner_word = actor->owner_word;
    effect->parent = actor;
    effect->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    return effect;
}

static void spawn_coffin(Exe6Obj *actor)
{
    (void)spawn_effect(
        actor,
        BN67_OBJ_ID(django_coffin_main),
        0,
        actor->target_block_x,
        actor->target_block_y
    );
}

static void spawn_charge_light(Exe6Obj *actor)
{
    (void)spawn_effect(
        actor,
        BN67_OBJ_ID(django_light_main),
        LIGHT_VARIANT_CHARGE,
        actor->block_x,
        actor->block_y
    );
}

static void spawn_rocks(Exe6Obj *actor)
{
    exe6_camera_quake_set(2, 30);
    exe6_sound_req(0xE5);
    for (uint8_t variant = 0; variant < 4; ++variant) {
        (void)spawn_effect(
            actor,
            BN67_OBJ_ID(django_rock_main),
            variant,
            (uint8_t)(
                actor->target_block_x + ROCK_BLOCK_OFFSETS[variant][0]
            ),
            (uint8_t)(
                actor->target_block_y + ROCK_BLOCK_OFFSETS[variant][1]
            )
        );
    }
}

static void spawn_sunlight(Exe6Obj *actor)
{
    (void)spawn_effect(
        actor,
        BN67_OBJ_ID(django_light_main),
        LIGHT_VARIANT_SUN_CONTROLLER,
        actor->target_block_x,
        actor->target_block_y
    );
}

static void spawn_sunlight_damage(Exe6Obj *self)
{
    Exe6Obj *damage = exe6_set_shl03_ev(
        self->target_block_x,
        self->target_block_y,
        self->owner_aux,
        0,
        SUNLIGHT_DAMAGE_PROPERTIES,
        SUNLIGHT_PULSE_DAMAGE,
        3
    );
    if (damage != NULL) {
        damage->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    }
}

static void actor_appear(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        exe6_sound_req(0x94);
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        self->timer = APPEAR_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        set_phase(self, ACTOR_PHASE_WAIT);
    }
}

static void actor_wait(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 0);
        self->timer = WAIT_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        set_phase(
            self,
            work->has_target != 0 ? ACTOR_PHASE_READY : ACTOR_PHASE_EXIT
        );
    }
}

static void actor_ready(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 3);
        self->timer = READY_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (self->timer == READY_FRAMES / 2) {
        spawn_charge_light(self);
    }
    if (decrement_timer(&self->timer) <= 0) {
        set_phase(self, ACTOR_PHASE_BUILDUP);
    }
}

static void actor_buildup(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 4);
        self->timer = 0;
        self->aux_timer = CHARGE_SOUND_PERIOD;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (self->timer == ROCK_SPAWN_FRAME && work->rocks_spawned == 0) {
        work->rocks_spawned = 1;
        spawn_rocks(self);
    }

    ++self->timer;
    ++self->aux_timer;
    if (self->aux_timer >= CHARGE_SOUND_PERIOD) {
        self->aux_timer = 0;
        exe6_sound_req(0x149);
    }
    if (self->timer > BUILDUP_LAST_FRAME) {
        set_phase(self, ACTOR_PHASE_PAUSE);
    }
}

static void actor_pause(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 0);
        self->timer = PAUSE_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        set_phase(self, ACTOR_PHASE_SUNLIGHT);
    }
}

static void actor_sunlight(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 5);
        work->damage_remaining = (uint16_t)(self->attack & 0x7FFFu);
        work->sunlight_sound_timer = 0;
        if (work->sun_spawned == 0) {
            work->sun_spawned = 1;
            spawn_sunlight(self);
        }
        self->substate = UPDATE_STEP_ACTIVE;
    }

    if (work->damage_remaining != 0) {
        spawn_sunlight_damage(self);
        --work->damage_remaining;
        if (work->sunlight_sound_timer == 0) {
            exe6_sound_req(0xF9);
            work->sunlight_sound_timer = 11;
        }
        --work->sunlight_sound_timer;
    } else {
        set_phase(self, ACTOR_PHASE_COOLDOWN);
    }
}

static void actor_cooldown(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 0);
        self->timer = COOLDOWN_FRAMES;
        self->substate = UPDATE_STEP_ACTIVE;
    }
    if (decrement_timer(&self->timer) <= 0) {
        restore_target(self);
        set_phase(self, ACTOR_PHASE_EXIT);
    }
}

static void actor_exit(Exe6Obj *self)
{
    if (self->substate == UPDATE_STEP_INIT) {
        set_animation(self, 2);
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
    } else if (self->phase == ACTOR_PHASE_READY) {
        actor_ready(self);
    } else if (self->phase == ACTOR_PHASE_BUILDUP) {
        actor_buildup(self);
    } else if (self->phase == ACTOR_PHASE_PAUSE) {
        actor_pause(self);
    } else if (self->phase == ACTOR_PHASE_SUNLIGHT) {
        actor_sunlight(self);
    } else if (self->phase == ACTOR_PHASE_COOLDOWN) {
        actor_cooldown(self);
    } else {
        actor_exit(self);
    }
    if (self->phase < ACTOR_PHASE_EXIT) {
        hide_target(self);
    }
    exe6_battle_obj_char_move2();
}

static void actor_init(Exe6Obj *self)
{
    struct ActorWork *work = (struct ActorWork *)self->work;
    exe6_battle_obj_char_init(
        0x01010000u
        | (BN67_SPRITE_GROUP(django_battle_sprite) << 8)
        | BN67_SPRITE_ID(django_battle_sprite)
    );
    exe6_block_to_pos();
    self->z = 0;
    exe6_obj_flip_set(exe6_enemy_flip_check());
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    self->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    work->target = find_target_in_row(
        self,
        &self->target_block_x,
        &self->target_block_y
    );
    work->has_target = (uint8_t)(work->target != NULL);
    if (work->has_target != 0) {
        spawn_coffin(self);
        hide_target(self);
    }
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
    set_phase(self, ACTOR_PHASE_APPEAR);
}

static void set_effect_animation(Exe6Obj *self, uint8_t animation)
{
    if (self->animation != animation) {
        set_animation(self, animation);
    }
}

static void coffin_init(Exe6Obj *self)
{
    exe6_block_to_pos();
    exe6_battle_obj_char_init(
        (BN67_SPRITE_GROUP(django_coffin_sprite) << 8)
        | BN67_SPRITE_ID(django_coffin_sprite)
    );
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    self->z = 0;
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE
        | EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
}

static void coffin_update(Exe6Obj *self)
{
    Exe6Obj *actor = self->parent;
    if (!actor_is_active(actor)) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
        return;
    }
    if (actor->phase >= ACTOR_PHASE_EXIT) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
        return;
    }
    if (actor->phase == ACTOR_PHASE_SUNLIGHT) {
        set_effect_animation(self, 2);
    } else if (actor->phase >= ACTOR_PHASE_COOLDOWN) {
        set_effect_animation(self, 1);
    } else {
        set_effect_animation(self, 0);
    }
    exe6_battle_obj_char_move2();
}

static const uintptr_t SINE_TABLE_ADDRESS = 0x080065E0;
static const uint8_t SUNLIGHT_START_ANGLES[3] = {0x00, 0x55, 0xAA};

static void light_position(Exe6Obj *self)
{
    if (self->variant == LIGHT_VARIANT_CHARGE) {
        Exe6Obj *actor = self->parent;
        int32_t direction = (int32_t)exe6_calc_pl_em_dir_spd_for(self);
        self->x = actor->x - direction * (8 << 16);
        self->y = actor->y;
        self->z = actor->z + (64 << 16);
        return;
    }
    if (self->variant == LIGHT_VARIANT_SUN_CONTROLLER) {
        exe6_block_to_pos();
        self->z = 35 << 16;
        return;
    }

    Exe6Obj *controller = self->parent;
    const int16_t *sine = (const int16_t *)SINE_TABLE_ADDRESS;
    uint8_t angle = (uint8_t)self->aux_timer;
    int32_t direction = -(int32_t)exe6_calc_pl_em_dir_spd_for(self);
    self->x = controller->x
        + direction * (int32_t)sine[0x40u + angle] * 45 * 0x100;
    self->y = controller->y + (int32_t)sine[angle] * 28 * 0x100;
    self->z = 24 << 16;
}

static void spawn_sunlight_beams(Exe6Obj *controller)
{
    for (uint8_t variant = LIGHT_VARIANT_SUN_FIRST;
         variant <= LIGHT_VARIANT_SUN_THIRD;
         ++variant) {
        (void)spawn_effect(
            controller,
            BN67_OBJ_ID(django_light_main),
            variant,
            controller->block_x,
            controller->block_y
        );
    }
}

static void light_init(Exe6Obj *self)
{
    uint32_t animation = self->variant == LIGHT_VARIANT_CHARGE ? 0 : 2;
    exe6_battle_obj_char_init(
        (animation << 16)
        | (BN67_SPRITE_GROUP(django_sun_sprite) << 8)
        | BN67_SPRITE_ID(django_sun_sprite)
    );
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    light_position(self);
    if (self->variant == LIGHT_VARIANT_SUN_CONTROLLER) {
        spawn_sunlight_beams(self);
    } else if (self->variant != LIGHT_VARIANT_CHARGE) {
        self->aux_timer = SUNLIGHT_START_ANGLES[
            self->variant - LIGHT_VARIANT_SUN_FIRST
        ];
        self->aux_timer = (uint16_t)(
            self->aux_timer + SUNLIGHT_ORBIT_STEP
        );
        light_position(self);
    }
    if (self->variant == LIGHT_VARIANT_SUN_CONTROLLER) {
        self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    } else {
        self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
    self->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
}

static void light_update(Exe6Obj *self)
{
    if (self->variant == LIGHT_VARIANT_CHARGE) {
        Exe6Obj *actor = self->parent;
        if (!actor_is_active(actor)
            || actor->phase < ACTOR_PHASE_READY
            || actor->phase > ACTOR_PHASE_BUILDUP) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
        light_position(self);
    } else if (self->variant == LIGHT_VARIANT_SUN_CONTROLLER) {
        Exe6Obj *actor = self->parent;
        if (!actor_is_active(actor)
            || actor->phase != ACTOR_PHASE_SUNLIGHT) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
    } else {
        Exe6Obj *controller = self->parent;
        Exe6Obj *actor = object_is_active(controller)
            ? controller->parent
            : NULL;
        if (!actor_is_active(controller)
            || !actor_is_active(actor)
            || actor->phase != ACTOR_PHASE_SUNLIGHT) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
        self->aux_timer = (uint16_t)(
            self->aux_timer + SUNLIGHT_ORBIT_STEP
        );
        light_position(self);
    }
    exe6_battle_obj_char_move2();
}

static void rock_position(Exe6Obj *self)
{
    exe6_block_to_pos();
    self->z = (
        (int32_t)self->parent->target_block_y * 24 + 40
    ) << 16;
}

static void rock_init(Exe6Obj *self)
{
    rock_position(self);
    exe6_battle_obj_char_init(
        ((uint32_t)GREGAR_ROCK_SPRITE_GROUP << 8)
        | GREGAR_ROCK_SPRITE_ID
    );
    exe6_obj_shadow_set();
    exe6_obj_prio_set(EXE6_OBJ_PRIORITY_BATTLE);
    Exe6Obj *poof = exe6_efc_open_at(
        0,
        self->x,
        self->y,
        self->z,
        exe6_obj_spawn_with_variant(2)
    );
    if (poof != NULL) {
        poof->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    }
    self->header_flags |= EXE6_OBJ_FLAG_VISIBLE
        | EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    self->phase = 0;
    self->velocity_z = 0;
    self->state_word = EXE6_OBJECT_STATE_ACTIVE;
}

static void rock_update(Exe6Obj *self)
{
    if (!actor_is_active(self->parent)) {
        self->state_word = EXE6_OBJECT_STATE_DESTROY;
        return;
    }
    if (self->phase == 0) {
        self->velocity_z -= 0x00008000;
        self->z += self->velocity_z;
        if (self->z <= 0) {
            self->z = 0;
            self->timer = 20;
            self->phase = 4;
        }
    } else {
        if ((self->timer & 4u) != 0) {
            self->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
        } else {
            self->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
        }
        if (decrement_timer(&self->timer) <= 0) {
            self->state_word = EXE6_OBJECT_STATE_DESTROY;
            return;
        }
    }
    exe6_battle_obj_char_move2();
}

BN67_EFFECT(django_coffin_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        coffin_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        coffin_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_EFFECT(django_light_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        light_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        light_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_EFFECT(django_rock_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        rock_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        rock_update(self);
    } else {
        exe6_obj_move_delete();
    }
}

BN67_ENEMY(django_actor_main)
{
    if (self->state == EXE6_OBJECT_STATE_INIT) {
        actor_init(self);
    } else if (self->state == EXE6_OBJECT_STATE_ACTIVE) {
        actor_update(self);
    } else {
        restore_target(self);
        actor_destroy(self);
    }
}

BN67_SUMMON_ATTACK(0x116, django_attack_main)
{
    Exe6Obj *actor = exe6_em_open(
        BN67_OBJ_ID(django_actor_main), spawn_parameters
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
