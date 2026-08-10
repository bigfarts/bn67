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

static USED __attribute__((noinline)) void delete_entire_hand(Exe6Obj *player)
{
    if (
        (player->hit->received_hit_flags
            & EXE6_RECEIVED_HIT_FLAG_DELETE_ACTIVE_CHIP) == 0
    ) {
        return;
    }

    player->active_chip_id = UINT16_MAX;
    const Exe6PlayerRuntime *runtime = player->runtime_data;
    if (runtime == NULL || runtime->type != 2) {
        player->loaded_chip_count = 0;
        return;
    }

    Exe6NaviSelectChipWork *selection =
        exe6_navi_select_chip_work_adrs_get(player->owner);
    uint8_t cursor = selection->active_chip_index;
    while (selection->chip_ids[cursor] != UINT16_MAX) {
        ++cursor;
    }
    selection->active_chip_index = cursor;
}

/* Native player hit processing enters with the player object in r5. */
NAKED void chip_delete_entire_hand_main(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "adds r0,r5,#0\n"
        "bl delete_entire_hand\n"
        "pop {pc}\n"
    );
}
