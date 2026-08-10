#include "abi.h"

static USED __attribute__((noinline)) void delete_entire_hand(Exe6Obj *player)
{
    if (
        (player->hit->received_hit_flags
            & EXE6_HIT_TYPE_FLAG_DELETE_ACTIVE_CHIP) == 0
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
