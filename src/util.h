#ifndef PSLAB_UTIL
#define PSLAB_UTIL

#include <stdint.h>

#define ABS16_MAX (-(INT16_MIN + 1) + 1)
#define ABS32_MAX (-(INT32_MIN + 1) + 1)

static inline uint16_t abs16(int16_t const value)
{
    return (uint16_t)(
        value == INT16_MIN ? ABS16_MAX : value < 0 ? -value : value
    );
}

static inline uint32_t abs32(int32_t const value)
{
    return (uint32_t)(
        value == INT32_MIN ? ABS32_MAX : value < 0 ? -value : value
    );
}

#endif // PSLAB_UTIL
