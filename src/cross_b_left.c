#include "common.h"
#include "cross_b_left.h"
#include "runtime.h"

#define CROSS_B_LEFT_INPUT_GATE 0x08013130
#define CROSS_B_LEFT_INIT_DISPATCH 0x08011796
#define CROSS_B_LEFT_INPUT_SKIP 0x08013176
#define CROSS_B_LEFT_RAPID_INPUT_GATE 0x08013194
#define CROSS_B_LEFT_RAPID_INPUT_CHECK 0x0801319C
#define CROSS_B_LEFT_RAPID_INPUT_SKIP 0x080131D8
#define CROSS_POWER_ATTACK_TABLE 0x080117D4

#define HEAT_CROSS_ACTIVE_FORM 1
#define SLASH_CROSS_ACTIVE_FORM 3
/* Native Cross Beast form IDs are the corresponding Cross IDs plus 12. */
#define HEAT_BEAST_ACTIVE_FORM 13
#define SLASH_BEAST_ACTIVE_FORM 15

typedef uint32_t (*CrossBLeftInitWork)(struct CrossBLeftAttackWork *work);

BN67_PATCH_SECTION(
    CROSS_B_LEFT_INPUT_GATE,
    cross_b_left_input_gate
);
BN67_PATCH_SECTION(
    CROSS_B_LEFT_INIT_DISPATCH,
    cross_b_left_init_dispatch
);
BN67_PATCH_SECTION(
    CROSS_B_LEFT_RAPID_INPUT_GATE,
    cross_b_left_rapid_input_gate
);

static USED CrossBLeftInitWork
cross_b_left_init_work_for_player(const Exe6Obj *player)
{
    if (player == NULL) {
        return NULL;
    }

    const Exe6NaviStatusWork *status =
        exe6_navi_status_work_adrs_get(player->owner);
    if (status == NULL) {
        return NULL;
    }

    switch (status->active_form) {
    case HEAT_CROSS_ACTIVE_FORM:
    case HEAT_BEAST_ACTIVE_FORM:
        return heat_cross_b_left_init_work;
    case SLASH_CROSS_ACTIVE_FORM:
    case SLASH_BEAST_ACTIVE_FORM:
        return slash_cross_b_left_init_work;
    default:
        return NULL;
    }
}

/* Release a short B tap before the native parser decrements the command window.
 * This includes the final window tick, which would otherwise look expired by
 * the time the rapid-fire gate runs. */
static USED CrossBLeftInitWork cross_b_left_prepare_input(Exe6Obj *player)
{
    CrossBLeftInitWork init_work = cross_b_left_init_work_for_player(player);
    if (init_work == NULL) {
        return NULL;
    }
    const Exe6NaviStatusWork *status =
        exe6_navi_status_work_adrs_get(player->owner);
    Exe6PlayerRuntime *runtime = player->runtime_data;
    if ((status->active_form == HEAT_BEAST_ACTIVE_FORM
            || status->active_form == SLASH_BEAST_ACTIVE_FORM)
        && runtime->b_left_window != 0
        && (runtime->released_input & 2u) != 0) {
        runtime->b_left_window = 0;
        runtime->action_flags |= 1u;
    }
    return init_work;
}

/* Action zero is reserved here to mean that the native dispatcher should run. */
static USED uint32_t cross_b_left_init_for_player(
    const Exe6Obj *player,
    struct CrossBLeftAttackWork *work
)
{
    CrossBLeftInitWork init_work = cross_b_left_init_work_for_player(player);
    if (init_work == NULL) {
        return 0;
    }
    return init_work(work);
}

/*
 * The native input gate skips B-left when no NaviCust B-left is installed.
 * Crosses with standalone B-left attacks always fall through; every other form
 * keeps the native check.
 */
NAKED void cross_b_left_input_gate(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "push {r4,lr}\n"
        "adds r0,r5,#0\n"
        "bl cross_b_left_prepare_input\n"
        "cmp r0,#0\n"
        "pop {r4}\n"
        "pop {r1}\n"
        "mov lr,r1\n"
        "bne 1f\n"
        "ldrb r0,[r4,#8]\n"
        "cmp r0,#0xff\n"
        "bne 1f\n"
        "ldr r0,=" BN67_STRINGIFY(CROSS_B_LEFT_INPUT_SKIP) "+1\n"
        "bx r0\n"
        "1:\n"
        "bx lr\n"
    );
}

/*
 * Held-B rapid fire is queued after B-left input and wins action dispatch.
 * Extend the native TenguBeast/DustBeast exception to our Cross Beasts so a
 * pending B-left suppresses the competing shot. Also let the native eight-frame
 * B-left window finish before starting rapid fire: otherwise pressing B starts
 * a shot whose cleanup discards a subsequent Left input during that window.
 * Releasing B during the window fires once, preserving short B taps.
 * r0 already holds the active form; the final comparison supplies the flags
 * for the displaced native conditional branch at 0x0801319A.
 */
NAKED void cross_b_left_rapid_input_gate(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "cmp r0,#" BN67_STRINGIFY(HEAT_BEAST_ACTIVE_FORM) "\n"
        "beq 2f\n"
        "cmp r0,#" BN67_STRINGIFY(SLASH_BEAST_ACTIVE_FORM) "\n"
        "beq 2f\n"
        "cmp r0,#20\n"
        "beq 1f\n"
        "cmp r0,#22\n"
        "bx lr\n"
        "1:\n"
        "ldr r0,=" BN67_STRINGIFY(CROSS_B_LEFT_RAPID_INPUT_CHECK) "+1\n"
        "bx r0\n"
        "2:\n"
        "ldrb r0,[r4,#0x13]\n"
        "cmp r0,#0\n"
        "beq 1b\n"
        "ldr r0,=" BN67_STRINGIFY(CROSS_B_LEFT_RAPID_INPUT_SKIP) "+1\n"
        "bx r0\n"
    );
}

/*
 * The native B-left dispatcher has already saved r6/r7/lr and prepared r7 as
 * attack work. A supported Cross initializes its standalone action; every
 * other form resumes the displaced native power-attack table lookup.
 */
NAKED void cross_b_left_init_dispatch(void)
{
    __asm__(
        ".syntax unified\n"
        "pop {r1}\n"
        "push {r2,r3,lr}\n"
        "adds r0,r5,#0\n"
        "adds r1,r7,#0\n"
        "bl cross_b_left_init_for_player\n"
        "cmp r0,#0\n"
        "pop {r2,r3}\n"
        "pop {r1}\n"
        "mov lr,r1\n"
        "bne 2f\n"
        "1:\n"
        "ldr r1,=" BN67_STRINGIFY(CROSS_POWER_ATTACK_TABLE) "\n"
        "ldrb r0,[r6,#8]\n"
        "lsls r0,r0,#2\n"
        "bx lr\n"
        "2:\n"
        "pop {r6,r7,pc}\n"
    );
}
