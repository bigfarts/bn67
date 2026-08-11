#include "abi.h"

static inline bool timer_expired(Exe6Obj *self)
{
    int32_t timer = (int32_t)self->timer - 1;
    self->timer = (uint16_t)timer;
    return timer < 0;
}
