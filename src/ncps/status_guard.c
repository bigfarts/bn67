#include "runtime.h"

#define STATUS_GUARD_PROPERTY 0x52u

#define STATUS_GUARD_UNCOMPRESSED_SHAPE \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,1,0,0,0\n" \
    ".byte 0,0,1,1,1,0,0\n" \
    ".byte 0,0,0,1,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n"

#define STATUS_GUARD_COMPRESSED_SHAPE \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,1,0,0,0\n" \
    ".byte 0,0,1,1,1,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n"

BN67_ASM_RESOURCE(
    status_guard_program_ncp_uncompressed_shape,
    STATUS_GUARD_UNCOMPRESSED_SHAPE
);
BN67_ASM_RESOURCE(
    status_guard_program_ncp_compressed_shape,
    STATUS_GUARD_COMPRESSED_SHAPE
);

/* BodyPack's native slot becomes the pink StatGrd program part. */
BN67_NCP(
    0x1C,
    status_guard_program_ncp,
    status_guard_ncp_main,
    1,
    0,
    3,
    0xFF,
    0xFF,
    0xFF
)
{
    exe6_cur_pet_navi_stats_set(0, STATUS_GUARD_PROPERTY, 1);
}
