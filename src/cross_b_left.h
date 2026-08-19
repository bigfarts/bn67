#ifndef BN67_CROSS_B_LEFT_H
#define BN67_CROSS_B_LEFT_H

#include "abi.h"

struct CrossBLeftAttackWork {
    uint8_t action_state;                // +0x00
    uint8_t phase;                       // +0x01
    uint8_t element;                     // +0x02
    uint8_t version;                     // +0x03
    uint8_t marker;                      // +0x04
    uint8_t lockout;                     // +0x05
    uint16_t attack_bonus;               // +0x06
    uint32_t attack;                     // +0x08, damage and hit properties
    uint32_t parameters;                 // +0x0C
    uint16_t timer;                      // +0x10
};

_Static_assert(
    offsetof(struct CrossBLeftAttackWork, attack) == 0x08,
    "Cross B-left attack offset"
);
_Static_assert(
    offsetof(struct CrossBLeftAttackWork, parameters) == 0x0C,
    "Cross B-left parameter offset"
);

uint32_t heat_cross_b_left_init_work(struct CrossBLeftAttackWork *work);

uint32_t slash_cross_b_left_init_work(struct CrossBLeftAttackWork *work);

#endif
