#include "common.h"
#include "cross_b_left.h"

#define SLASH_CROSS_B_LEFT_ACTION 0x40
#define SLASH_CROSS_B_LEFT_DAMAGE 130
#define SLASH_CROSS_MOON_BLADE_COUNTER_FRAMES 0x1E

USED uint32_t slash_cross_b_left_init_work(
    struct CrossBLeftAttackWork *work
)
{
    work->action_state = 0;
    work->phase = 0;
    work->element = 0;
    work->version = 0;
    work->marker = 0;
    work->lockout = 0;
    work->attack_bonus = 0;
    work->attack =
        ((uint32_t)SLASH_CROSS_MOON_BLADE_COUNTER_FRAMES << 16)
        | SLASH_CROSS_B_LEFT_DAMAGE;
    work->parameters = 0;
    work->timer = 0;
    return SLASH_CROSS_B_LEFT_ACTION;
}
