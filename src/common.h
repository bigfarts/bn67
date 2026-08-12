#ifndef BN67_COMMON_H
#define BN67_COMMON_H

#include "abi.h"

static inline void set_animation(Exe6Obj *self, uint8_t animation)
{
    self->animation = animation;
    self->palette = UINT8_MAX;
}

static inline void set_animation_immediate(
    Exe6Obj *self,
    uint32_t animation
)
{
    set_animation(self, (uint8_t)animation);
    exe6_obj_dma_seq_set(animation);
    exe6_obj_char_set();
}

static inline void set_phase(Exe6Obj *self, uint8_t phase)
{
    self->phase = phase;
    self->phase_timer = 0;
}

static inline int32_t decrement_timer(uint16_t *timer)
{
    int32_t value = (int32_t)*timer - 1;
    *timer = (uint16_t)value;
    return value;
}

static inline void actor_destroy(Exe6Obj *self)
{
    uint8_t *completion = self->completion;
    if (completion != NULL) {
        *completion = 0;
    }
    exe6_obj_move_delete();
}

#endif
