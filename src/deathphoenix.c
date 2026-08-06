#include "runtime.h"

BN6_SPRITE(deathphoenix_battle_sprite, "build/deathphoenix-battle-sprite.bin");
BN6_SPRITE(deathphoenix_strike_sprite, "build/deathphoenix-strike-sprite.bin");

#if FALZAR
BN6_INCBIN(DeathphoenixIcon, "build/deathphoenix-icon.bin");
BN6_INCBIN(DeathphoenixImage, "build/deathphoenix-image.bin");
BN6_INCBIN(DeathphoenixPalette, "build/deathphoenix-palette.bin");
#endif

static const uint8_t VISIBLE_FLAG = 2;
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
static const uint32_t VALID_PANEL_FLAGS = 0x10;
static const uint32_t CONTACT_PROPERTIES = 0x0A050001;
static const uint32_t INTRO_PROPERTIES = 0x2705FF80;
static const uintptr_t SAVED_NAVI_ADDRESS = 0x0203C960;
static const uintptr_t SAVED_NAVI_DISPATCH = 0x0802CD5C;
static const uintptr_t SINE_TABLE_ADDRESS = 0x080065E0;

static const uint8_t STRIKE_PATTERNS[] = {
    2, 1, 3,
    1, 2, 3,
    1, 3, 3,
    2, 1, 2,
};

struct SavedNaviFields {
    uint8_t id;
    uint8_t parameter;
    uint16_t offset;
    uint32_t data;
    uint32_t properties;
};

struct DeathphoenixWork {
    uint32_t pattern_row;                // +0x60
    uint8_t recycle_completion;          // +0x64
    uint8_t reserved_65[3];
    const uint8_t *pattern;              // +0x68
    uint8_t panels[3][4];                // +0x6C
    uint32_t pattern_column;             // +0x78
};

_Static_assert(
    offsetof(struct DeathphoenixWork, pattern_column) == 0x18,
    "DeathPhoenix work layout"
);

static bool timer_positive_after_decrement(Object *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer > 0;
}

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

static void finish(Object *self)
{
    *self->completion = 0;
    self->state_word = DESTROY_STATE;
}

static void build_panel_list(
    Object *self,
    uint32_t panel_y,
    volatile uint8_t *output
)
{
    uint8_t candidates[8];
    uint8_t shuffled[16];
    bn6_memory_clear(candidates, sizeof(candidates));
    bn6_memory_clear(shuffled, sizeof(shuffled));

    int32_t direction = (int32_t)bn6_object_front_direction_for(self);
    int32_t panel_x = (int32_t)self->panel_x + direction;
    uint32_t count = 0;
    while (bn6_panel_is_valid_xy((uint32_t)panel_x, panel_y) != 0) {
        if (bn6_panel_has_flags(
                (uint32_t)panel_x,
                panel_y,
                VALID_PANEL_FLAGS,
                0
            ) != 0) {
            candidates[count++] = (uint8_t)(panel_x | (panel_y << 4));
        }
        panel_x += direction;
    }

    for (uint32_t written = 0; count != 0 && written < 4; written += count) {
        bn6_memory_copy(candidates, shuffled + written, count);
        bn6_shuffle_bytes(shuffled + written, count, count);
    }
    bn6_memory_copy(shuffled, (void *)output, 4);
}

static void actor_init(Object *self)
{
    volatile struct DeathphoenixWork *work =
        (volatile struct DeathphoenixWork *)self->work;
    bn6_self_object_load_navi_sprite(
        0x00010000u
        | (BN6_SPRITE_GROUP(deathphoenix_battle_sprite) << 8)
        | BN6_SPRITE_ID(deathphoenix_battle_sprite)
    );
    bn6_self_sprite_set_flip(bn6_self_object_get_flip());

    self->panel_x = (uint8_t)((self->owner ^ self->owner_aux) * 5u + 1u);
    self->panel_y = 3;
    bn6_self_object_set_coords();
    self->z = 0;
    self->y += 3 << 16;
    self->z += 3 << 16;

    build_panel_list(self, 1, work->panels[0]);
    build_panel_list(self, 2, work->panels[1]);
    build_panel_list(self, 3, work->panels[2]);
    self->state_word = ACTIVE_STATE;
}

static void begin(Object *self)
{
    (void)bn6_spawn_panel_damage(1, 1, 0, 0, INTRO_PROPERTIES, 0, 0);
    set_phase(self, APPEAR_PHASE);
}

static void appear(Object *self)
{
    if (self->substate == 0) {
        self->flags |= VISIBLE_FLAG;
        bn6_play_sound(0x94);
        self->timer = 0;
        self->substate = 4;
    }

    uint32_t blend = (uint32_t)self->timer + 1u;
    self->timer = (uint16_t)blend;
    if (blend < APPEAR_FRAMES) {
        bn6_self_sprite_set_blend_mode(blend);
        return;
    }
    bn6_self_sprite_property_2cce();
    set_phase(self, WAIT_BEFORE_PHASE);
}

static void wait_before_strikes(Object *self)
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
    volatile struct DeathphoenixWork *work =
        (volatile struct DeathphoenixWork *)self->work;
    work->pattern_row = 0;
    work->pattern_column = 0;
    set_phase(self, STRIKES_PHASE);
}

static void strike_spawn(
    Object *actor,
    uint32_t panel_x,
    uint32_t panel_y,
    uint32_t alternate
)
{
    Object *strike = bn6_spawn_type4(CUSTOM_TYPE4_ID, alternate);
    if (strike == NULL) {
        return;
    }
    strike->panel_x = (uint8_t)panel_x;
    strike->panel_y = (uint8_t)panel_y;
    strike->attack = actor->attack;
    strike->owner_word = actor->owner_word;
    strike->parent = actor;
    strike->kind = BN6_OBJECT_KIND(deathphoenix_strike_main);
}

static void spawn_strike_for_panel(
    Object *self,
    uint32_t panel_x,
    uint32_t panel_y
)
{
    if (panel_x == 0) {
        return;
    }
    uint32_t distance = self->panel_x > panel_x
        ? self->panel_x - panel_x
        : panel_x - self->panel_x;
    uint32_t alternate = distance > 2;
    if (distance == 3 && (bn6_battle_rng() & 0x10u) != 0) {
        alternate = 0;
    }
    strike_spawn(self, panel_x, panel_y, alternate);
}

static void strikes(Object *self)
{
    volatile struct DeathphoenixWork *work =
        (volatile struct DeathphoenixWork *)self->work;
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->timer = STRIKE_INTERVAL;

    if (work->pattern_row == 0) {
        uint32_t pattern = bn6_battle_rng() % 3u;
        work->pattern = &STRIKE_PATTERNS[pattern * 3u];
    }
    const uint8_t *pattern = work->pattern;
    uint32_t row = pattern[work->pattern_row] - 1u;
    uint8_t packed = work->panels[row][work->pattern_column];
    spawn_strike_for_panel(self, packed & 7u, packed >> 4);

    ++work->pattern_row;
    if (work->pattern_row >= 3) {
        work->pattern_row = 0;
        ++work->pattern_column;
    }
    if (++self->aux_timer >= STRIKE_COUNT) {
        set_phase(self, WAIT_AFTER_PHASE);
    }
}

static void wait_after_strikes(Object *self)
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
    volatile struct SavedNaviFields *saved =
        (volatile struct SavedNaviFields *)SAVED_NAVI_ADDRESS;
    return saved->data != 0 && saved->id != UINT8_MAX;
}

static void disappear(Object *self)
{
    if (self->substate == 0) {
        self->flags |= VISIBLE_FLAG;
        self->timer = APPEAR_FRAMES;
        self->substate = 4;
    }
    if (timer_nonnegative_after_decrement(self)) {
        bn6_self_sprite_set_blend_mode(self->timer);
        return;
    }

    self->flags &= (uint8_t)~VISIBLE_FLAG;
    bn6_self_sprite_property_2cce();
    if (self->parent->hp != 0 && saved_navi_is_available()) {
        set_phase(self, RECYCLE_PHASE);
    } else {
        finish(self);
    }
}

static void recycle_intro(Object *self)
{
    if (self->substate == 0) {
        bn6_saved_navi_intro(self->parent, 1);
        self->timer = RECYCLE_WAIT_FRAMES;
        self->substate = 4;
    }
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase_timer = RECYCLE_INVOKE_STEP;
    }
}

static void recycle_invoke(Object *self)
{
    volatile struct DeathphoenixWork *work =
        (volatile struct DeathphoenixWork *)self->work;
    volatile uint8_t *completion = &work->recycle_completion;
    if (self->substate == 0) {
        volatile struct SavedNaviFields *saved =
            (volatile struct SavedNaviFields *)SAVED_NAVI_ADDRESS;
        volatile uintptr_t *dispatch =
            (volatile uintptr_t *)SAVED_NAVI_DISPATCH;
        Object *owner = self->parent;
        bn6_saved_navi_dispatch(
            dispatch[saved->id],
            owner,
            owner->panel_x,
            owner->panel_y,
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

static void recycle_wait(Object *self)
{
    if (self->substate == 0) {
        self->timer = RECYCLE_WAIT_FRAMES;
        self->substate = 4;
    }
    if (!timer_nonnegative_after_decrement(self)) {
        self->phase_timer = RECYCLE_CLEANUP_STEP;
    }
}

static void recycle_update(Object *self)
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

static void actor_update(Object *self)
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

static void flame_spawn(Object *strike, uint32_t alternate)
{
    Object *flame = bn6_spawn_type4(CUSTOM_TYPE4_ID, alternate);
    if (flame == NULL) {
        return;
    }
    flame->panel_x = strike->panel_x;
    flame->panel_y = strike->panel_y;
    flame->owner_word = strike->owner_word;
    flame->kind = BN6_OBJECT_KIND(deathphoenix_flame_main);
    flame->flags |= 0x10u;
    if (alternate != 0) {
        flame->owner_aux ^= 1u;
    }
}

static void spawn_contact(
    Object *self,
    uint32_t panel_x,
    uint32_t panel_y,
    uint16_t visual
)
{
    Object *contact = bn6_spawn_panel_damage(
        panel_x,
        panel_y,
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

static void strike_attack(Object *self)
{
    uint32_t frame = self->aux_timer++;
    if (frame == 0) {
        spawn_contact(
            self,
            self->panel_x,
            self->panel_y,
            FIRST_CONTACT_VISUAL
        );
    } else if (frame == 16) {
        int32_t direction = (self->owner ^ self->variant) == 0 ? 1 : -1;
        spawn_contact(
            self,
            (uint32_t)((int32_t)self->panel_x + direction),
            self->panel_y,
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
        self->flags &= (uint8_t)~VISIBLE_FLAG;
        self->state_word = DESTROY_STATE;
    }
}

static void strike_lead_in(Object *self)
{
    if (self->animation == 0) {
        if ((bn6_self_sprite_get_animation_flags() & 0x80u) == 0) {
            return;
        }
        self->animation = 1;
    }
    if (timer_positive_after_decrement(self)) {
        return;
    }
    self->aux_timer = 0;
    set_phase(self, STRIKE_ATTACK_PHASE);
    bn6_play_sound(0x145);
}

static void strike_update(Object *self)
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

static void strike_init(Object *self)
{
    bn6_self_object_load_navi_sprite(
        0x01000000u
        | (BN6_SPRITE_GROUP(deathphoenix_strike_sprite) << 8)
        | BN6_SPRITE_ID(deathphoenix_strike_sprite)
    );
    bn6_self_death_sprite_special();
    self->flags |= VISIBLE_FLAG;
    bn6_self_object_set_coords();
    self->z = 0;
    bn6_play_sound(0x144);
    self->state_word = ACTIVE_STATE;
    strike_update(self);
}

static void flame_motion(Object *self)
{
    if (self->phase_timer_low == 0) {
        self->phase_timer_low = 1;
        self->variant_word = 0;
        self->timer = FLAME_MOTION_FRAMES;
        self->animation_state_word = 0x0800;
        self->velocity_x =
            (int32_t)bn6_object_front_direction_for(self) * 0x00028000;
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

    volatile const int16_t *sine =
        (volatile const int16_t *)SINE_TABLE_ADDRESS;
    int32_t height = sine[angle >> 8];
    self->z = (height * 20 << 8) + 0x00040000;
    if (!timer_positive_after_decrement(self)) {
        self->state_word = DESTROY_STATE;
    }
}

static void flame_update(Object *self)
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

static void flame_init(Object *self)
{
    bn6_self_object_set_coords();
    self->y -= 1 << 16;
    bn6_self_sprite_load(
        0x80,
        BN6_SPRITE_GROUP(deathphoenix_strike_sprite),
        BN6_SPRITE_ID(deathphoenix_strike_sprite)
    );
    self->animation = 2;
    bn6_self_sprite_set_animation(2);
    bn6_self_sprite_load_animation_data();
    bn6_self_sprite_no_shadow();
    bn6_self_sprite_set_palette(0);
    self->flags |= VISIBLE_FLAG;
    self->state_word = ACTIVE_STATE;
}

BN6_OBJECT4(deathphoenix_flame_main)
{
    if (self->state == 0) {
        flame_init(self);
    } else if (self->state == ACTIVE_STATE) {
        flame_update(self);
    } else {
        bn6_self_object_free();
    }
    bn6_self_object_update();
}

BN6_OBJECT4(deathphoenix_strike_main)
{
    if (self->state == 0) {
        strike_init(self);
    } else if (self->state == ACTIVE_STATE) {
        strike_update(self);
    } else {
        bn6_self_object_free();
    }
    bn6_self_object_update();
}

BN6_OBJECT1(deathphoenix_actor_main)
{
    if (self->state == 0) {
        actor_init(self);
    } else if (self->state == ACTIVE_STATE) {
        actor_update(self);
    } else {
        bn6_self_object_free();
        return;
    }
    bn6_self_object_update_timestop();
}

BN6_SUMMON_ATTACK(0x134, deathphoenix_attack_main)
{
    Object *actor = bn6_spawn_type1(CUSTOM_TYPE1_ID, spawn_argument);
    if (actor == NULL) {
        return;
    }
    actor->panel_x = (uint8_t)panel_x;
    actor->panel_y = (uint8_t)panel_y;
    actor->parameter = (uint8_t)parameter;
    actor->parent = owner;
    actor->owner_word = owner->owner_word;
    actor->attack = attack;
    actor->completion = completion;
    actor->kind = BN6_OBJECT_KIND(deathphoenix_actor_main);
    *completion = 1;
}
