/* Vita's syscall boundary can return API-9 errors -1..-8 with bit 30
 * cleared. On-device NO_MEMORY (-2) was observed as 0xBFFFFFFE. */
#ifndef VJO_CAPTURE_ERROR_H
#define VJO_CAPTURE_ERROR_H
#include <stdint.h>

static inline int vjo_capture_normalize_result(int result)
{
    uint32_t value = (uint32_t)result;
    if ((value & 0xFFFFFFF8u) == 0xBFFFFFF8u)
        return (int32_t)(value | 0x40000000u);
    return result;
}
#endif
