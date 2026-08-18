#include "runtime.h"

#define STATUS_GUARD_PROPERTY 0x52u

static USED __attribute__((noinline)) void status_guard_uninstall(
    Exe6Obj *player
)
{
    /* Tomahawk Cross grants immunity through native active-form ID 7, not
     * this base/NaviCust property, so its innate Status Guard remains live. */
    exe6_navi_status_set(player->owner, STATUS_GUARD_PROPERTY, 0);
}

/*
 * Native Uninstall and SunMoon's BlueMoon attack both finish their successful
 * removal path at 0x0801414A. Clear StatGrd there, then tail-call the native
 * refresh routine displaced by the hook.
 */
NAKED void status_guard_uninstall_main(void)
{
    __asm__(
        ".syntax unified\n"
        "push {lr}\n"
        "adds r0,r5,#0\n"
        "bl status_guard_uninstall\n"
        "pop {r3}\n"
        "mov lr,r3\n"
        "ldr r3,=0x0801469D\n"
        "bx r3\n"
    );
}

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
