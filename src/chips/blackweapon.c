#include "runtime.h"

BN67_INCBIN(blackweapon_dark_palette, "build/blackweapon-dark-palette.bin");

BN67_ASM_RESOURCE(
    blackweapon_hp_bug_periods,
    ".byte 0,40,35,30,25,20,15,10,6\n"
);
BN67_PATCH_POINTER(0x080102A0, blackweapon_hp_bug_periods);

/* Power-attack IDs 3 and 4 are the two Beast Out rapid-Buster variants. */
BN67_PATCH_THUMB_POINTER(
    0x080117E0,
    blackweapon_beast_buster_id3_dispatch
);
BN67_PATCH_THUMB_POINTER(
    0x080117E4,
    blackweapon_beast_buster_id4_dispatch
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
    0x0801264C,
    blackweapon_attack_level_dispatch
);

#if !FALZAR
BN67_INCBIN(blackweapon_icon, "build/blackweapon-icon.bin");
BN67_INCBIN(blackweapon_image, "build/blackweapon-image.bin");
BN67_INCBIN(blackweapon_palette, "build/blackweapon-palette.bin");
#endif

#if FALZAR
#define BLACKWEAPON_EFFECT_FLAGS EXE6_CHIP_EFFECT_FLAG_DIMMING
#define BLACKWEAPON_ICON ((const uint8_t *)0x0872C394u)
#define BLACKWEAPON_IMAGE ((const uint8_t *)0x08720634u)
#define BLACKWEAPON_PALETTE ((const uint8_t *)0x08725534u)
#else
#define BLACKWEAPON_EFFECT_FLAGS                                        \
    (EXE6_CHIP_EFFECT_FLAG_DIMMING |                                    \
     EXE6_CHIP_EFFECT_FLAG_VERSION_AVAILABLE)
#define BLACKWEAPON_ICON blackweapon_icon
#define BLACKWEAPON_IMAGE blackweapon_image
#define BLACKWEAPON_PALETTE blackweapon_palette
#endif

BN67_CHIP_RECORD(0x12d) {
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
        .effect_flags = BLACKWEAPON_EFFECT_FLAGS,
        .counter_settings = 0,
        .family = BN67_ATTACK_FAMILY(blackweapon_attack_main),
        .subfamily = BN67_ATTACK_SUBFAMILY(blackweapon_attack_main),
        .dark_soul_usage = 0x0A,
        .unknown_0e = 0x04,
        .lock_on = 0,
        .object_spawn = {0},
        .delay = 0,
    },
    .library_number = 1,
    .library_flags = 0x14,
    .library_lock_on_type = 0,
    .alphabetical_sort = 0,
    .power = 0,
    .library_sort_order = 0x012d,
    .library_gate_usage = 1,
    .dark_chip_id = UINT8_MAX,
    .icon = BLACKWEAPON_ICON,
    .image = BLACKWEAPON_IMAGE,
    .palette = BLACKWEAPON_PALETTE,
};

static const uint8_t BUSTER_ATTACK_LEVEL_10 = 9;
static const uint8_t BUSTER_STAT_MAX = 4;
static const uint8_t BLACKWEAPON_HP_BUG = 8;
static const uint16_t FLASH_FRAMES = 60;
static const uint16_t HOLD_FRAMES = 30;

NAKED void blackweapon_beast_attack_level_apply(void)
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

NAKED void blackweapon_beast_buster_id3_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r3,=0x08011AF3\n"
        "mov lr,pc\n"
        "bx r3\n"
        "push {r0}\n"
        "bl blackweapon_beast_attack_level_apply\n"
        "pop {r0,pc}\n"
        ".pool\n"
    );
}

NAKED void blackweapon_beast_buster_id4_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "ldr r3,=0x08011B4B\n"
        "mov lr,pc\n"
        "bx r3\n"
        "push {r0}\n"
        "bl blackweapon_beast_attack_level_apply\n"
        "pop {r0,pc}\n"
        ".pool\n"
    );
}

NAKED void blackweapon_attack_level_dispatch(void)
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

struct BlackWeaponControllerWork {
    uint8_t visual_active;
};

struct BlackWeaponVisualWork {
    uint8_t white_palette[0x20];
};

_Static_assert(
    sizeof(struct BlackWeaponVisualWork) == sizeof(((Exe6Obj *)0)->work),
    "BlackWeapon visual work must hold one complete palette"
);

enum BlackWeaponVisualFlags {
    BLACKWEAPON_VISUAL_OWNER_WAS_VISIBLE = 1 << 0,
    BLACKWEAPON_VISUAL_PALETTE_SAVED = 1 << 1,
};

#define BLACKWEAPON_DARK_PALETTE_BANK 0x0Fu

static void *blackweapon_palette_bank(uint32_t palette_bank)
{
    return (void *)(uintptr_t)(EXE6_SPRITE_PALETTE_STAGING_00
        + palette_bank * 0x20u);
}

static void install_dark_palette(Exe6Obj *visual)
{
    const uint8_t *sprite = (const uint8_t *)visual
        + ((uint32_t)(visual->object_class >> 4) << 4);
    uint32_t palette_bank = (uint32_t)(sprite[0x15] >> 4);
    if (palette_bank != BLACKWEAPON_DARK_PALETTE_BANK) {
        return;
    }
    exe6_mem_trans256(
        blackweapon_dark_palette,
        blackweapon_palette_bank(palette_bank),
        0x20
    );
}

static void save_white_palette(Exe6Obj *visual)
{
    struct BlackWeaponVisualWork *work =
        (struct BlackWeaponVisualWork *)visual->work;
    exe6_mem_trans256(
        blackweapon_palette_bank(BLACKWEAPON_DARK_PALETTE_BANK),
        work->white_palette,
        sizeof(work->white_palette)
    );
    visual->aux_timer |= BLACKWEAPON_VISUAL_PALETTE_SAVED;
}

static void restore_white_palette(Exe6Obj *visual)
{
    if ((visual->aux_timer & BLACKWEAPON_VISUAL_PALETTE_SAVED) == 0) {
        return;
    }

    const struct BlackWeaponVisualWork *work =
        (const struct BlackWeaponVisualWork *)visual->work;
    exe6_mem_trans256(
        work->white_palette,
        blackweapon_palette_bank(BLACKWEAPON_DARK_PALETTE_BANK),
        sizeof(work->white_palette)
    );
    visual->aux_timer &= (uint16_t)~BLACKWEAPON_VISUAL_PALETTE_SAVED;
}

static void restore_owner(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner != NULL
        && (visual->aux_timer & BLACKWEAPON_VISUAL_OWNER_WAS_VISIBLE) != 0) {
        owner->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
}

static void restore_visual(Exe6Obj *visual)
{
    restore_white_palette(visual);
    restore_owner(visual);
}

static void finish_visual(Exe6Obj *visual)
{
    restore_visual(visual);
    if (visual->completion != NULL) {
        *visual->completion = 0;
    }
    visual->state_word = 8;
}

static void copy_owner_position(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        finish_visual(visual);
        return;
    }
    visual->x = owner->x;
    visual->y = owner->y;
    visual->z = owner->z;
}

static void visual_flash_update(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    if (owner == NULL) {
        finish_visual(visual);
        return;
    }

    exe6_obj_clt_set(exe6_obj_clt_link_get(owner));

    uint8_t *sprite = (uint8_t *)visual
        + ((uint32_t)(visual->object_class >> 4) << 4);
    if ((visual->timer & 2u) != 0) {
        sprite[0x15] = (uint8_t)((sprite[0x15] & 0x0Fu) | 0xF0u);
    } else {
        sprite[0x15] &= 0x0Fu;
    }
    copy_owner_position(visual);

    uint16_t timer = (uint16_t)(visual->timer - 1u);
    visual->timer = timer;
    if ((int16_t)timer < 0) {
        visual->timer = HOLD_FRAMES;
        visual->phase = 4;
    }
}

static void visual_hold_update(Exe6Obj *visual)
{
    copy_owner_position(visual);
    /* phase shares state_word, so compare only the state byte here. */
    if (visual->state != 4) {
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
    exe6_obj_current_navi_char_init();
    exe6_obj_char_set();
    exe6_obj_shadow_set();
    exe6_obj_clt_set(exe6_obj_clt_link_get(owner));
    visual->animation_word = 0;
    exe6_obj_dma_seq_set(0);
    exe6_obj_char_set();
    exe6_obj_char_move();
    exe6_obj_flip_set(visual->owner);

    visual->aux_timer = 0;
    if ((owner->header_flags & EXE6_OBJ_FLAG_VISIBLE) != 0) {
        visual->aux_timer |= BLACKWEAPON_VISUAL_OWNER_WAS_VISIBLE;
    }
    save_white_palette(visual);
    owner->header_flags &= (uint8_t)~EXE6_OBJ_FLAG_VISIBLE;
    visual->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    visual->timer = FLASH_FRAMES;
    visual->phase = 0;
    visual->state_word = 4;
    visual_flash_update(visual);
}

BN67_EFFECT(blackweapon_visual_main)
{
    switch (self->state) {
    case 0:
        visual_init(self);
        break;
    case 4:
        if (self->phase == 0) {
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
    exe6_battle_obj_char_move2();
    install_dark_palette(self);
}

static void spawn_visual(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    struct BlackWeaponControllerWork *work =
        (struct BlackWeaponControllerWork *)controller->work;
    work->visual_active = 0;

    Exe6Obj *visual = exe6_efc_open(
        BN67_OBJ_ID(blackweapon_visual_main), spawn_parameters
    );
    if (visual == NULL) {
        return;
    }
    visual->header_flags |= EXE6_OBJ_FLAG_UPDATE_DURING_DIMMING;
    visual->parent = controller->parent;
    visual->completion = &work->visual_active;
    work->visual_active = 1;
}

static void apply_blackweapon(Exe6Obj *controller)
{
    Exe6Obj *owner = controller->parent;
    if (owner != NULL) {
        uint32_t side = owner->owner;
        exe6_navi_status_set(side, 1, BUSTER_ATTACK_LEVEL_10);
        exe6_navi_status_set(side, 2, BUSTER_STAT_MAX);
        exe6_navi_status_set(side, 3, BUSTER_STAT_MAX);
        exe6_navi_status_set(side, 0x18, BLACKWEAPON_HP_BUG);
    }
    controller->phase = 0x0C;
    controller->phase_timer = 0;
}

static void effect_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    struct BlackWeaponControllerWork *work =
        (struct BlackWeaponControllerWork *)controller->work;
    if (controller->substate == 0) {
        spawn_visual(controller, spawn_parameters);
        controller->substate = 4;
    }
    if (work->visual_active == 0) {
        apply_blackweapon(controller);
    }
}

static void blackweapon_update(
    Exe6Obj *controller,
    Exe6ObjSpawnParameters spawn_parameters
)
{
    switch (controller->phase) {
    case 0:
        exe6_event_chip_common_fade();
        break;
    case 4:
        exe6_event_chip_common_telop();
        break;
    case 8:
        effect_update(controller, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_end();
        break;
    }
}

BN67_EFFECT(blackweapon_controller_main)
{
    switch (self->state) {
    case 0:
        exe6_event_chip_common_init();
        break;
    case 4:
        blackweapon_update(self, spawn_parameters);
        break;
    default:
        exe6_event_chip_common_exit();
        break;
    }
}

BN67_PERSISTENT_ATTACK(0x12d, blackweapon_attack_main)
{
    Exe6Obj *controller = exe6_efc_open(
        BN67_OBJ_ID(blackweapon_controller_main), spawn_parameters
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
