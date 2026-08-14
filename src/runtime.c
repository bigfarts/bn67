#include "runtime.h"

enum Bn67DeployablePlacementResult bn67_deployable_placement_check(
    uint32_t block_x,
    uint32_t block_y
)
{
    if (exe6_block_move_check(
            block_x,
            block_y,
            EXE6_BLOCK_FLAG_SOLID,
            0
        ) == 0) {
        return BN67_DEPLOYABLE_PLACEMENT_INVALID;
    }
    if (exe6_block_move_check(
            block_x,
            block_y,
            EXE6_BLOCK_FLAG_SOLID,
            EXE6_BLOCK_FLAG_SUPPORT_OBJECT
        ) == 0) {
        return BN67_DEPLOYABLE_PLACEMENT_OCCUPIED;
    }
    return BN67_DEPLOYABLE_PLACEMENT_CLEAR;
}
