#include "runtime.h"

BN6_INCBIN(folderback_icon, "build/folderback-icon.bin");
BN6_INCBIN(folderback_image, "build/folderback-image.bin");
BN6_INCBIN(folderback_palette, "build/folderback-palette.bin");
BN6_SONG(
    folderback_rumble_song,
    BN6_PCM(
        folderback_rumble,
        0x40,
        0x08,
        ".byte 0xBC,0x00,0xBB,0x4B,0xBD,0x00,0xBE,0x7F\n"
        ".byte 0xBF,0x40,0xF9,0x3C,0x7F,0xAA,0x81,0xB1\n",
        "build/folderback-rumble-sample.bin"
    )
);

static const uint32_t FULL_GAUGE = 0x4000;
static const uint32_t PALE_FLASH = 0x00004210;
static const uint32_t WHITE_FLASH = 0x00006318;
static const uint16_t IMPACT_FRAMES = 0x46;
static const uint16_t RESTORED_FRAMES = 0x14;

struct FolderbackWork {
    Object *locked_opponents[4];         // +0x60
    uint32_t reserved[3];                // +0x70
};

_Static_assert(sizeof(struct FolderbackWork) == 0x1C, "FolderBack work layout");

NAKED void folderback_type_1_main(void)
{
    __asm__(
        ".syntax unified\n"
        "push {r0,r4,r6,r7,lr}\n"
        "ldr r6,=0x02036870\n"
        "movs r7,#32\n"
        "1:\n"
        "ldrb r0,[r6,#0]\n"
        "movs r1,#" BN6_STRINGIFY(BN6_OBJECT_FLAG_ACTIVE) "\n"
        "tst r0,r1\n"
        "beq 3f\n"
        "ldrb r0,[r6,#1]\n"
        "ldr r1,=__bn6_object_id_folderback_controller_main\n"
        "cmp r0,r1\n"
        "bne 3f\n"
        "movs r4,#96\n"
        "movs r1,#4\n"
        "2:\n"
        "ldr r0,[r6,r4]\n"
        "cmp r0,r5\n"
        "beq 5f\n"
        "adds r4,#4\n"
        "subs r1,#1\n"
        "bne 2b\n"
        "3:\n"
        "movs r0,#200\n"
        "adds r6,r6,r0\n"
        "subs r7,#1\n"
        "bne 1b\n"
        "ldr r4,[sp,#0]\n"
        "adds r0,r4,#0\n"
        "mov lr,pc\n"
        "bx r4\n"
        "4:\n"
        "pop {r0,r4,r6,r7,pc}\n"
        "5:\n"
        "bl bn6_self_object_update_dimming\n"
        "b 4b\n"
    );
}

static void lock_opponent(Object *self)
{
    struct FolderbackWork *work = (struct FolderbackWork *)self->work;
    Object *const *units = bn6_active_units_for_side(self->owner ^ 1u);
    for (size_t index = 0; index < 4; ++index) {
        work->locked_opponents[index] = units[index];
    }
}

static void unlock_opponent(Object *self)
{
    struct FolderbackWork *work = (struct FolderbackWork *)self->work;
    for (size_t index = 0; index < 4; ++index) {
        work->locked_opponents[index] = NULL;
    }
}

static void apply_white_flash(Object *self)
{
    uint32_t color = (self->timer & 4u) != 0 ? PALE_FLASH : WHITE_FLASH;
    bn6_palette_write(0, color, 0x0F, 0x14, BN6_PALETTE_OBJ_OUTPUT_00);
    bn6_palette_write(0, color, 0x0F, 0x15, BN6_PALETTE_BG_OUTPUT_00);
}

static void restore_palette(void)
{
    bn6_palette_restore(0x14);
    bn6_palette_restore(0x15);
}

static void impact(void)
{
    bn6_screen_shake_set(3, IMPACT_FRAMES);
    bn6_play_sound(BN6_SONG_ID(folderback_rumble_song));
}

static void fill_local_custom_gauge(void)
{
    BattleContext *context = bn6_battle_context();
    uint8_t *player = bn6_player_data(context->local_side);
    *(uint16_t *)(player + 0x28) = FULL_GAUGE;
    bn6_set_hud_gauge(FULL_GAUGE);
    bn6_play_sound(0x8F);
}

static void restore_local_folder(Object *self)
{
    if (bn6_compare_local_side(self->owner) != 0) {
        return;
    }

    BattleContext *context = bn6_battle_context();
    uint8_t regular_available = context->regular_available;
    bn6_rebuild_folder();
    if (regular_available == 0) {
        context->regular_available = 0;
        bn6_shuffle_chip_queue(
            (void *)bn6_chip_queue(),
            0,
            context->work_44,
            0x44
        );
    }
    bn6_clear_hand();
}

static bool effect_update(Object *self)
{
    switch (self->phase_timer_low) {
    case 0:
        self->timer = IMPACT_FRAMES;
        self->phase_timer_low = 4;
        impact();
        apply_white_flash(self);
        break;
    case 4:
        if (--self->timer == 0) {
            restore_palette();
            self->phase_timer_low = 8;
        } else {
            apply_white_flash(self);
        }
        break;
    case 8:
        self->timer = RESTORED_FRAMES;
        self->phase_timer_low = 0x0C;
        fill_local_custom_gauge();
        restore_local_folder(self);
        break;
    default:
        if (--self->timer == 0) {
            return true;
        }
        break;
    }
    return false;
}

static void open_custom(uint32_t owner)
{
    uint8_t *battle = bn6_battle_state();
    uint32_t state = *(uint32_t *)battle;
    if (state == 4) {
        battle[5] = (uint8_t)owner;
        bn6_lock_battle_state();
        *(uint32_t *)battle = 0x18;
        bn6_begin_local_custom();
    } else if (state == 8) {
        bn6_lock_battle_state();
        *(uint32_t *)battle = 0x20;
    }
}

BN6_OBJECT4(folderback_controller_main)
{
    if (self->state == 0) {
        self->state_word = 4;
        lock_opponent(self);
    } else if (self->state != 4) {
        bn6_self_object_free();
        return;
    }

    if (bn6_battle_is_over() != 0) {
        restore_palette();
        unlock_opponent(self);
        bn6_self_object_free();
        return;
    }
    if (!effect_update(self)) {
        return;
    }

    uint32_t owner = self->owner;
    unlock_opponent(self);
    bn6_self_object_free();
    open_custom(owner);
}

BN6_PERSISTENT_ATTACK(0x139, folderback_attack_main)
{
    Object *controller = bn6_spawn_type4(
        BN6_OBJECT_ID(folderback_controller_main), spawn_parameters
    );
    if (controller == NULL) {
        return NULL;
    }
    controller->panel_x = (uint8_t)panel_x;
    controller->panel_y = (uint8_t)panel_y;
    controller->parameter = (uint8_t)parameter;
    controller->parent = owner;
    controller->owner_word = owner->owner_word;
    controller->attack = attack;
    controller->chip_data = chip_data;
    struct FolderbackWork *work =
        (struct FolderbackWork *)controller->work;
    for (size_t index = 0; index < 4; ++index) {
        work->locked_opponents[index] = NULL;
    }
    for (size_t index = 0; index < 3; ++index) {
        work->reserved[index] = 0;
    }
    return controller;
}
