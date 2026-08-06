#include "runtime.h"

static inline BattleContext *runtime_battle_context(void)
{
    Runtime *runtime;
    __asm__("mov %0,r10" : "=l" (runtime));
    return runtime->battle_context;
}

Object *volatile *bn6_battle_units_for_side(uint32_t side)
{
    return runtime_battle_context()->battle_units[side];
}

Object *volatile *bn6_active_units_for_side(uint32_t side)
{
    return runtime_battle_context()->active_units[side];
}

BattleContext *bn6_battle_context(void)
{
    return runtime_battle_context();
}

volatile uint8_t *bn6_battle_state(void)
{
    return (volatile uint8_t *)0x0203CA70u;
}

volatile uint8_t *bn6_chip_queue(void)
{
    return (volatile uint8_t *)0x0203CDB0u;
}
