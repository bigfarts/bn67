#include "common.h"
#include "cross_b_left.h"
#include "runtime.h"

#define HEAT_CROSS_DEFAULT_COUNTER 0x0800FDB6
#define HEAT_CROSS_EXIT_ATTACK 0x08011714

#if FALZAR
#define HEAT_CROSS_PERSISTENT_ACTION_POINTER 0x080EAC74
#define HEAT_CROSS_PERSISTENT_ACTION_NATIVE 0x080EBD9C
#define HEAT_CROSS_BURN_SPAWN 0x080C8DE0
#else
#define HEAT_CROSS_PERSISTENT_ACTION_POINTER 0x080EBFB4
#define HEAT_CROSS_PERSISTENT_ACTION_NATIVE 0x080ED0DC
#define HEAT_CROSS_BURN_SPAWN 0x080CA650
#endif

#define HEAT_CROSS_B_LEFT_MARKER 0xB1
#define HEAT_CROSS_B_LEFT_ACTION 0x15
#define HEAT_CROSS_B_LEFT_DAMAGE 150
#define HEAT_CROSS_B_LEFT_HIT_PROPERTIES 0x00940000u
#define HEAT_CROSS_B_LEFT_DURATION 30
#define HEAT_CROSS_FIRE_ELEMENT 1
#define HEAT_CROSS_BURN_PARAMETERS 0x00001E04
#define HEAT_CROSS_MINIBOMB_THROW_POSE 0x06
#define HEAT_CROSS_MINIBOMB_RELEASE_TICK 9
#define HEAT_CROSS_BURNER_SPREAD_DELAY 10
#define HEAT_CROSS_PHASE_WAIT_FOR_THROW_RELEASE 4
#define HEAT_CROSS_PHASE_WAIT_FOR_OUTER_BURNS 8
#define HEAT_CROSS_PHASE_BURNS_ACTIVE 12

BN67_PATCH_THUMB_POINTER(
    HEAT_CROSS_PERSISTENT_ACTION_POINTER,
    heat_cross_persistent_action_dispatch
);

/* Convert the ordinary C ABI to BurnSquare shell 0x26's native ABI. */
static NAKED Exe6Obj *heat_cross_burn_spawn_native(
    uint32_t block_x __attribute__((unused)),
    uint32_t block_y __attribute__((unused)),
    uint32_t element __attribute__((unused)),
    Exe6Obj *owner __attribute__((unused)),
    uint32_t parameters __attribute__((unused)),
    uint32_t attack __attribute__((unused))
)
{
    __asm__(
        ".syntax unified\n"
        "push {r4-r7,lr}\n"
        "adds r5,r3,#0\n"
        "ldr r4,[sp,#20]\n"
        "ldr r6,[sp,#24]\n"
        "ldr r7,=" BN67_STRINGIFY(HEAT_CROSS_BURN_SPAWN) "+1\n"
        "mov r12,r7\n"
        "mov lr,pc\n"
        "bx r12\n"
        "pop {r4-r7,pc}\n"
    );
}

static NAKED void heat_cross_default_counter_native(void)
{
    __asm__(
        ".syntax unified\n"
        "ldr r3,=" BN67_STRINGIFY(HEAT_CROSS_DEFAULT_COUNTER) "+1\n"
        "bx r3\n"
    );
}

static NAKED void heat_cross_exit_attack_native(void)
{
    __asm__(
        ".syntax unified\n"
        "ldr r3,=" BN67_STRINGIFY(HEAT_CROSS_EXIT_ATTACK) "+1\n"
        "bx r3\n"
    );
}

static void heat_cross_try_spawn_burn(
    Exe6Obj *player,
    const struct CrossBLeftAttackWork *work,
    uint32_t block_x,
    uint32_t block_y
)
{
    const uint32_t required =
        EXE6_BLOCK_FLAG_VALID | EXE6_BLOCK_FLAG_SOLID;
    if ((exe6_block_status_get(block_x, block_y) & required) != required) {
        return;
    }

    heat_cross_burn_spawn_native(
        block_x,
        block_y,
        work->element,
        player,
        HEAT_CROSS_BURN_PARAMETERS,
        work->attack
    );
}

static void heat_cross_spawn_center_burn(
    Exe6Obj *player,
    const struct CrossBLeftAttackWork *work
)
{
    heat_cross_try_spawn_burn(
        player,
        work,
        player->block_x,
        player->block_y
    );
}

static void heat_cross_spawn_outer_burns(
    Exe6Obj *player,
    const struct CrossBLeftAttackWork *work
)
{
    uint32_t block_x = player->block_x;
    uint32_t block_y = player->block_y;
    int32_t front = (int32_t)exe6_calc_pl_em_dir_spd_for(player);

    heat_cross_try_spawn_burn(
        player,
        work,
        (uint32_t)((int32_t)block_x + front),
        block_y
    );
    heat_cross_try_spawn_burn(
        player,
        work,
        (uint32_t)((int32_t)block_x - front),
        block_y
    );
    heat_cross_try_spawn_burn(player, work, block_x, block_y - 1u);
    heat_cross_try_spawn_burn(player, work, block_x, block_y + 1u);
}

USED uint32_t heat_cross_b_left_init_work(
    struct CrossBLeftAttackWork *work
)
{
    work->action_state = 0;
    work->phase = 0;
    work->element = HEAT_CROSS_FIRE_ELEMENT;
    work->version = 0;
    work->marker = HEAT_CROSS_B_LEFT_MARKER;
    work->lockout = HEAT_CROSS_B_LEFT_DURATION;
    work->attack_bonus = 0;
    work->attack =
        HEAT_CROSS_B_LEFT_HIT_PROPERTIES | HEAT_CROSS_B_LEFT_DAMAGE;
    work->parameters = HEAT_CROSS_B_LEFT_DURATION;
    work->timer = 0;
    return HEAT_CROSS_B_LEFT_ACTION;
}

static USED void heat_cross_b_left_action_update(
    Exe6Obj *player,
    struct CrossBLeftAttackWork *work
)
{
    if (work->phase == 0) {
        set_animation_immediate(player, HEAT_CROSS_MINIBOMB_THROW_POSE);
        heat_cross_default_counter_native();
        /* Native MiniBomb advances its release timer on this initial tick. */
        work->timer = 1;
        work->phase = HEAT_CROSS_PHASE_WAIT_FOR_THROW_RELEASE;
        return;
    }

    if (work->phase == HEAT_CROSS_PHASE_WAIT_FOR_THROW_RELEASE) {
        if (work->timer < HEAT_CROSS_MINIBOMB_RELEASE_TICK) {
            ++work->timer;
            return;
        }
        heat_cross_spawn_center_burn(player, work);
        work->timer = 0;
        work->phase = HEAT_CROSS_PHASE_WAIT_FOR_OUTER_BURNS;
        return;
    }

    if (work->phase == HEAT_CROSS_PHASE_WAIT_FOR_OUTER_BURNS) {
        ++work->timer;
        if (work->timer < HEAT_CROSS_BURNER_SPREAD_DELAY) {
            return;
        }
        heat_cross_spawn_outer_burns(player, work);
        work->timer = 0;
        work->phase = HEAT_CROSS_PHASE_BURNS_ACTIVE;
        return;
    }

    ++work->timer;
    if (work->timer >= work->parameters) {
        heat_cross_exit_attack_native();
    }
}

/* Action 0x15 remains native except for the marked Heat Cross B-left attack. */
NAKED void heat_cross_persistent_action_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "ldrb r0,[r7,#4]\n"
        "cmp r0,#" BN67_STRINGIFY(HEAT_CROSS_B_LEFT_MARKER) "\n"
        "beq 1f\n"
        "ldr r0,=" BN67_STRINGIFY(HEAT_CROSS_PERSISTENT_ACTION_NATIVE) "+1\n"
        "bx r0\n"
        "1:\n"
        "push {r0,r4-r7,lr}\n"
        "adds r0,r5,#0\n"
        "adds r1,r7,#0\n"
        "bl heat_cross_b_left_action_update\n"
        "pop {r0,r4-r7,pc}\n"
    );
}
