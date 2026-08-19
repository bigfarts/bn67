#include "runtime.h"

#if FALZAR
#define HEAT_CROSS_FIRE_ARM_RETURN 0x080ECD74
#define HEAT_CROSS_BURN_SPAWN 0x080C8DE0
#else
#define HEAT_CROSS_FIRE_ARM_RETURN 0x080EE0B4
#define HEAT_CROSS_BURN_SPAWN 0x080CA650
#endif

#define HEAT_CROSS_BURN_PARAMETERS 0x00001E04

BN67_PATCH_SECTION(
    HEAT_CROSS_FIRE_ARM_RETURN,
    heat_cross_charge_shot_after_fire_arm
);

/* Fields resolved by Heat Cross's native charged-shot setup and attack. */
struct HeatCrossAttackWork {
    uint8_t reserved_00[2];
    uint8_t element;                     // +0x02
    uint8_t reserved_03[5];
    uint32_t attack;                     // +0x08, including attack bonus
};

_Static_assert(
    offsetof(struct HeatCrossAttackWork, attack) == 0x08,
    "Heat Cross attack offset"
);

/*
 * Convert the ordinary C ABI to BurnSquare's shell 0x26 constructor ABI.
 * Its packed r4 parameters are the native BurnSquare value: type 4, 30 frames.
 */
static NAKED void heat_cross_burn_spawn_native(
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

static void heat_cross_try_spawn_burn(
    Exe6Obj *player,
    const struct HeatCrossAttackWork *work,
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

static USED void heat_cross_spawn_surrounding_burns(
    Exe6Obj *player,
    const struct HeatCrossAttackWork *work
)
{
    uint32_t block_x = player->block_x;
    uint32_t block_y = player->block_y;
    int32_t front = (int32_t)exe6_calc_pl_em_dir_spd_for(player);

    heat_cross_try_spawn_burn(player, work, block_x, block_y - 1u);
    heat_cross_try_spawn_burn(player, work, block_x, block_y + 1u);
    heat_cross_try_spawn_burn(
        player,
        work,
        (uint32_t)((int32_t)block_x - front),
        block_y
    );
}

/*
 * The section patch replaces the native `pop {r7}`, animation value, and
 * animation store immediately after Fire Arm's forward object is spawned.
 * The relay's saved r1 sits above that native r7 value on the stack.
 */
NAKED void heat_cross_charge_shot_after_fire_arm(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "pop {r7}\n"
        "push {r4-r7,lr}\n"
        "adds r0,r5,#0\n"
        "adds r1,r7,#0\n"
        "bl heat_cross_spawn_surrounding_burns\n"
        "movs r0,#0x0a\n"
        "strb r0,[r5,#0x10]\n"
        "pop {r4-r7,pc}\n"
    );
}
