#include "runtime.h"

BN67_INCBIN(blackweapon_dark_palette, "build/blackweapon-dark-palette.bin");

BN67_ASM_RESOURCE(
    blackweapon_hp_bug_periods,
    ".byte 0,40,35,30,25,20,15,10,6\n"
);
BN67_PATCH_POINTER(0x080102A0, blackweapon_hp_bug_periods);

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

struct BlackWeaponControllerWork {
    uint8_t visual_active;
};

struct BlackWeaponVisualWork {
    uint8_t owner_was_visible;
    uint8_t dark_palette;
};

static void install_dark_palette(Exe6Obj *visual)
{
    const struct BlackWeaponVisualWork *work =
        (const struct BlackWeaponVisualWork *)visual->work;
    if (work->dark_palette == 0) {
        return;
    }

    const uint8_t *sprite = (const uint8_t *)visual
        + ((uint32_t)(visual->object_class >> 4) << 4);
    uint32_t palette_bank = (uint32_t)(sprite[0x15] >> 4);
    exe6_mem_trans256(
        blackweapon_dark_palette,
        (void *)(uintptr_t)(EXE6_SPRITE_PALETTE_STAGING_00
            + palette_bank * 0x20u),
        0x20
    );
}

static void restore_owner(Exe6Obj *visual)
{
    Exe6Obj *owner = visual->parent;
    struct BlackWeaponVisualWork *work =
        (struct BlackWeaponVisualWork *)visual->work;
    if (owner != NULL && work->owner_was_visible != 0) {
        owner->header_flags |= EXE6_OBJ_FLAG_VISIBLE;
    }
}

static void finish_visual(Exe6Obj *visual)
{
    restore_owner(visual);
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

    struct BlackWeaponVisualWork *work =
        (struct BlackWeaponVisualWork *)visual->work;
    work->dark_palette = (uint8_t)((visual->timer & 2u) != 0);
    exe6_obj_clt_set(exe6_obj_clt_link_get(owner));

    uint8_t *sprite = (uint8_t *)visual
        + ((uint32_t)(visual->object_class >> 4) << 4);
    if (work->dark_palette != 0) {
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

    struct BlackWeaponVisualWork *work =
        (struct BlackWeaponVisualWork *)visual->work;
    work->owner_was_visible =
        (uint8_t)(owner->header_flags & EXE6_OBJ_FLAG_VISIBLE);
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
        restore_owner(self);
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
