#include "runtime.h"

#define BEAST_OUT_TURNS_PROPERTY 0x21u
/* Millions owned this byte before BeastT+1 replaced it. NaviCust's normal
 * stat rebuild clears the byte before applying every installed piece, making
 * the whole value a fresh per-piece Beast Out bonus. */
#define BEAST_TIME_BONUS_PROPERTY 0x33u

#define BEAST_OUT_BASE_TURNS 3u
#define BEAST_OUT_MAX_TURNS 9u

/* The game stores both forms as independent 7x7 masks. Keep this plus part a
 * centered 2x2 square in either orientation/compression state. */
#define BEAST_TIME_UNCOMPRESSED_SHAPE \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,1,1,0,0,0\n" \
    ".byte 0,0,1,1,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n"

#define BEAST_TIME_COMPRESSED_SHAPE \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,1,1,0,0,0\n" \
    ".byte 0,0,1,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n" \
    ".byte 0,0,0,0,0,0,0\n"

BN67_ASM_RESOURCE(
    beast_time_program_ncp_uncompressed_shape,
    BEAST_TIME_UNCOMPRESSED_SHAPE
);
BN67_ASM_RESOURCE(
    beast_time_program_ncp_compressed_shape,
    BEAST_TIME_COMPRESSED_SHAPE
);

/* White, pink, and yellow are native NaviCust color selectors 1, 3, and 2.
 * Effect group 2 produces the Emotion Window bug. */
BN67_NCP(
    0x16,
    beast_time_program_ncp,
    beast_time_ncp_main,
    2,
    1,
    1,
    3,
    2,
    0xFF
)
{
    uint32_t bonus =
        exe6_cur_pet_navi_stats_get(0, BEAST_TIME_BONUS_PROPERTY);
    if (bonus < BEAST_OUT_MAX_TURNS - BEAST_OUT_BASE_TURNS) {
        exe6_cur_pet_navi_stats_set(
            0,
            BEAST_TIME_BONUS_PROPERTY,
            bonus + 1
        );
    }
}

/* The native battle initializer overwrites BeastOutCounter with 3 after
 * NaviCust effects have run. Replace that initializer with this dynamic
 * equivalent so each installed BeastT+1 piece survives into the battle. */
USED void beast_time_counter_init(void)
{
    uint32_t navi = exe6_get_cur_pet_navi();
    uint32_t encoded_bonus = exe6_cur_pet_navi_stats_get(
        navi,
        BEAST_TIME_BONUS_PROPERTY
    );
    uint32_t turns = BEAST_OUT_BASE_TURNS + encoded_bonus;
    exe6_cur_pet_navi_stats_set(navi, BEAST_OUT_TURNS_PROPERTY, turns);
}

/* Link setup writes directly into the copied battle Navi-stats record in r6
 * instead of calling the normal initializer. Keep its native flag gate and
 * replace only the hard-coded three-turn store. */
NAKED void beast_time_link_counter_init(void)
{
    __asm__(
        ".syntax unified\n"
        "movs r0,#0x33\n"
        "ldrb r0,[r6,r0]\n"
        "adds r0,#3\n"
        "movs r1,#0x21\n"
        "strb r0,[r6,r1]\n"
        "bx lr\n"
    );
}
