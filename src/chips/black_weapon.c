#include "runtime.h"

BN67_INCBIN(black_weapon_dark_palette, "build/black_weapon_dark_palette.bin");

BN67_ASM_RESOURCE(
    black_weapon_hp_bug_periods,
    ".byte 0,40,35,30,25,20,15,10,6\n"
);
BN67_PATCH_POINTER(0x080102A0, black_weapon_hp_bug_periods);

/* Power-attack IDs 3 and 4 are the two Beast Out rapid-Buster variants. */
BN67_PATCH_THUMB_POINTER(
    0x080117E0,
    black_weapon_beast_buster_id3_dispatch
);
BN67_PATCH_THUMB_POINTER(
    0x080117E4,
    black_weapon_beast_buster_id4_dispatch
);

/*
 * Cross Buster charges and chargeable Cross chips share the native scaler at
 * 0x08012642. Its level helper already supports Attack 1 through 10, but the
 * caller clamps that result to 5. Remove only that redundant clamp so
 * BlackWeapon and each stacked BusterUp level retain the native
 * base-plus-per-level calculation through Attack 10.
 */
BN67_PATCH_SECTION(
    0x08012646,
    black_weapon_attack_level_dispatch
);

#if !FALZAR
BN67_INCBIN(black_weapon_icon, "build/black_weapon_icon.bin");
BN67_INCBIN(black_weapon_image, "build/black_weapon_image.bin");
BN67_INCBIN(black_weapon_palette, "build/black_weapon_palette.bin");
#endif

#if FALZAR
#define EFFECT_FLAGS EXE6_CHIP_EFFECT_FLAG_DIMMING
#define ICON ((const uint8_t *)0x0872C494u)
#define IMAGE ((const uint8_t *)0x087210B4u)
#define PALETTE ((const uint8_t *)0x08725574u)
#else
#define EFFECT_FLAGS                                                    \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING |                                    \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#define ICON black_weapon_icon
#define IMAGE black_weapon_image
#define PALETTE black_weapon_palette
#endif

BN67_CHIP_RECORD(0x12f) {
    .codes = {
        EXE6_CHIP_CODE_B,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
        EXE6_CHIP_CODE_NONE,
    },
    .attack_element = 0,
    .rarity = 4,
    .element = EXE6_CHIP_ELEMENT_NULL,
    .chip_class = EXE6_CHIP_CLASS_GIGA,
    .mb = 64,
    .behavior = {
        .effect_flags = EFFECT_FLAGS,
        .counter_settings = 0,
        .family = BN67_ATTACK_FAMILY(black_weapon_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(black_weapon_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 3,
    .library_flags = 0x14,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x012f,
    .library_gate_usage = 1,
    .dark_chip_id = UINT8_MAX,
    .icon = ICON,
    .image = IMAGE,
    .palette = PALETTE,
};

static const uint8_t BUSTER_ATTACK_LEVEL_10 = 9;
static const uint8_t BUSTER_STAT_MAX = 4;
static const uint8_t HP_BUG = 8;
static const uint16_t FLASH_FRAMES = 60;
static const uint16_t HOLD_FRAMES = 30;

enum VisualPhase {
    VISUAL_PHASE_FLASH,
    VISUAL_PHASE_HOLD = 4,
};

enum EffectStep {
    EFFECT_STEP_INIT,
    EFFECT_STEP_WAIT_FOR_VISUAL = 4,
};

NAKED void black_weapon_beast_attack_level_apply(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        /* Extend only levels above 5; preserve the original result below it. */
        "ldr r3,=0x0801265B\n"
        "mov lr,pc\n"
        "bx r3\n"
        "cmp r0,#5\n"
        "ble 1f\n"
        "strh r0,[r7,#8]\n"
        "1:\n"
        "pop {pc}\n"
        ".pool\n"
    );
}

NAKED void black_weapon_beast_buster_id3_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r3,=0x08011AF3\n"
        "mov lr,pc\n"
        "bx r3\n"
        "push {r0}\n"
        "bl black_weapon_beast_attack_level_apply\n"
        "pop {r0,pc}\n"
        ".pool\n"
    );
}

NAKED void black_weapon_beast_buster_id4_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r3,=0x08011B4B\n"
        "mov lr,pc\n"
        "bx r3\n"
        "push {r0}\n"
        "bl black_weapon_beast_attack_level_apply\n"
        "pop {r0,pc}\n"
        ".pool\n"
    );
}

NAKED void black_weapon_attack_level_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        /* Discard the section patch's saved r1. */
        "pop {r1}\n"
        /* Run BN6's native Attack-level helper, whose own ceiling is 10. */
        "ldr r3,=0x0801265B\n"
        "mov lr,pc\n"
        "bx r3\n"
        "adds r2,r0,#0\n"
        /* Restore the scaler's base and per-level increment arguments. */
        "pop {r0,r1}\n"
        /* Rejoin at base + increment * level, after the old level-5 clamp. */
        "ldr r3,=0x08012655\n"
        "bx r3\n"
        ".pool\n"
    );
}

struct ControllerWork {
    uint8_t visual_active;
};

struct VisualWork {
    uint8_t white_palette[0x20];
};

_Static_assert(
    sizeof(struct VisualWork) == sizeof(((Exe6Obj *)0)->work),
    "BlackWeapon visual work must hold one complete palette"
);

enum VisualFlags {
    VISUAL_PALETTE_SAVED = 1 << 0,
};

#define DARK_PALETTE_BANK 0x0Fu
#define EXE6_SPRITE_PALETTE_BANK_MASK 0xF0u
#define EXE6_SPRITE_PALETTE_BANK_SHIFT 4u
#define EXE6_SPRITE_PALETTE_ATTRIBUTE 0x15u

static uint8_t *palette_bank_address(uint8_t palette_bank)
{
    return (uint8_t *)(uintptr_t)(EXE6_SPRITE_PALETTE_STAGING_00
        + (uintptr_t)palette_bank * 0x20u);
}

static uint8_t *object_sprite(Exe6Obj *object)
{
    return (uint8_t *)object + (object->object_class & 0xF0u);
}

static uint8_t object_palette_bank_get(Exe6Obj *object)
{
    return (uint8_t)(
        object_sprite(object)[EXE6_SPRITE_PALETTE_ATTRIBUTE]
        >> EXE6_SPRITE_PALETTE_BANK_SHIFT
    );
}

static void object_palette_bank_set(Exe6Obj *object, uint8_t palette_bank)
{
    uint8_t *attribute =
        &object_sprite(object)[EXE6_SPRITE_PALETTE_ATTRIBUTE];
    *attribute = (uint8_t)(
        (*attribute & (uint8_t)~EXE6_SPRITE_PALETTE_BANK_MASK)
        | (palette_bank << EXE6_SPRITE_PALETTE_BANK_SHIFT)
    );
}

static void install_dark_palette(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        return;
    }

    uint8_t palette_bank = object_palette_bank_get(owner);
    if (palette_bank != DARK_PALETTE_BANK) {
        return;
    }
    exe6_mem_trans256(
        black_weapon_dark_palette,
        palette_bank_address(palette_bank),
        0x20
    );
}

static void save_white_palette(Exe6Obj *visual)
{
    struct VisualWork *work = (struct VisualWork *)visual->work;
    exe6_mem_trans256(
        palette_bank_address(DARK_PALETTE_BANK),
        work->white_palette,
        sizeof(work->white_palette)
    );
    visual->aux_timer |= VISUAL_PALETTE_SAVED;
}

static void restore_white_palette(Exe6Obj *visual)
{
    if ((visual->aux_timer & VISUAL_PALETTE_SAVED) == 0) {
        return;
    }

    const struct VisualWork *work = (const struct VisualWork *)visual->work;
    exe6_mem_trans256(
        work->white_palette,
        palette_bank_address(DARK_PALETTE_BANK),
        sizeof(work->white_palette)
    );
    visual->aux_timer &= (uint16_t)~VISUAL_PALETTE_SAVED;
}

static void restore_owner_palette(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        return;
    }

    if (object_palette_bank_get(owner) == DARK_PALETTE_BANK) {
        object_palette_bank_set(owner, visual->palette);
    }
}

static void restore_visual(Exe6Obj *visual)
{
    restore_white_palette(visual);
    restore_owner_palette(visual);
}

static void finish_visual(Exe6Obj *visual)
{
    restore_visual(visual);
    if (visual->completion != NULL) {
        *visual->completion = 0;
    }
    visual->state_word = EXE6_OBJECT_STATE_DESTROY;
}

static void visual_flash_update(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        finish_visual(visual);
        return;
    }

    uint8_t palette_bank = object_palette_bank_get(owner);
    if (palette_bank != DARK_PALETTE_BANK) {
        visual->palette = palette_bank;
    }
    uint8_t next_palette_bank = (visual->timer & 2u) != 0
        ? DARK_PALETTE_BANK
        : visual->palette;
    object_palette_bank_set(owner, next_palette_bank);
    exe6_obj_invoke(owner, (uintptr_t)exe6_battle_obj_char_move2);

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        visual->timer = HOLD_FRAMES;
        visual->phase = VISUAL_PHASE_HOLD;
    }
}

static void visual_hold_update(Exe6Obj *visual)
{
    if (visual->parent == NULL) {
        finish_visual(visual);
        return;
    }

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        finish_visual(visual);
    }
}

static void visual_init(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        finish_visual(visual);
        return;
    }

    visual->owner_word = owner->owner_word;
    visual->aux_timer = 0;
    visual->palette = object_palette_bank_get(owner);
    save_white_palette(visual);
    visual->timer = FLASH_FRAMES;
    visual->phase = VISUAL_PHASE_FLASH;
    visual->state_word = EXE6_OBJECT_STATE_ACTIVE;
    visual_flash_update(visual);
}

BN67_EFFECT(black_weapon_visual_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        visual_init(self);
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        if (self->phase == VISUAL_PHASE_FLASH) {
            visual_flash_update(self);
        } else {
            visual_hold_update(self);
        }
        break;
    default:
        restore_visual(self);
        exe6_obj_move_delete();
        return;
    }
    install_dark_palette(self);
}

static void spawn_visual(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    struct ControllerWork *work = (struct ControllerWork *)controller->work;
    work->visual_active = 0;

    Exe6Obj *visual = exe6_efc_open(
        BN67_OBJ_ID(black_weapon_visual_main), spawn_parameters
    );
    if (visual == NULL) {
        return;
    }
    visual->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    visual->parent = controller->parent;
    visual->completion = &work->visual_active;
    work->visual_active = 1;
}

static void apply_black_weapon(Exe6Obj *controller)
{
    Exe6Obj *owner = controller->parent;
    if (owner != NULL) {
        uint32_t side = owner->owner;
        exe6_navi_status_set(side, 1, BUSTER_ATTACK_LEVEL_10);
        exe6_navi_status_set(side, 2, BUSTER_STAT_MAX);
        exe6_navi_status_set(side, 3, BUSTER_STAT_MAX);
        exe6_navi_status_set(side, 0x18, HP_BUG);
    }
    controller->phase = EXE6_EVENT_CHIP_PHASE_OUTRO;
    controller->phase_timer = EFFECT_STEP_INIT;
}

static void effect_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    struct ControllerWork *work = (struct ControllerWork *)controller->work;
    if (controller->substate == EFFECT_STEP_INIT) {
        spawn_visual(controller, spawn_parameters);
        controller->substate = EFFECT_STEP_WAIT_FOR_VISUAL;
    }
    if (work->visual_active == 0) {
        apply_black_weapon(controller);
    }
}

static void controller_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    switch (controller->phase) {
    case EXE6_EVENT_CHIP_PHASE_FADE:
        exe6_event_chip_common_fade();
        break;
    case EXE6_EVENT_CHIP_PHASE_TELOP:
        exe6_event_chip_common_telop();
        break;
    case EXE6_EVENT_CHIP_PHASE_EFFECT:
        effect_update(controller, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

BN67_EFFECT(black_weapon_controller_main)
{
    switch (self->state) {
    case EXE6_OBJECT_STATE_INIT:
        exe6_event_chip_common_init();
        break;
    case EXE6_OBJECT_STATE_ACTIVE:
        controller_update(self, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_PERSISTENT_ATTACK(0x12f, black_weapon_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(black_weapon_controller_main), spawn_parameters
    );
    if (controller == NULL) {
        return NULL;
    }
    controller->block_x = (uint8_t)block_x;
    controller->block_y = (uint8_t)block_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
    return controller;
}
