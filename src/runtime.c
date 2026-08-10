#include "runtime.h"

static inline Exe6BattleContext *runtime_battle_context(void)
{
    Exe6Runtime *runtime;
    __asm__("mov %0,r10" : "=l" (runtime));
    return runtime->battle_context;
}

Exe6Obj *const *exe6_battle_units_for_side(uint32_t side)
{
    return runtime_battle_context()->battle_units[side];
}

Exe6Obj *const *exe6_active_units_for_side(uint32_t side)
{
    return runtime_battle_context()->active_units[side];
}

Exe6BattleContext *exe6_battle_context(void)
{
    return runtime_battle_context();
}

uint8_t *exe6_battle_state(void)
{
    return (uint8_t *)0x0203CA70u;
}

uint8_t *exe6_chip_queue(void)
{
    return (uint8_t *)0x0203CDB0u;
}
