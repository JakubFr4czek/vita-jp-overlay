/* Capture buffer, allocated for the requested rectangle and released after
 * the JPEG reader consumes its last row. Adapted from Rubi v0.7.12. */
#include <psp2kern/kernel/sysclib.h>
#include <psp2kern/kernel/sysmem.h>

#include "vjo_kernel.h"

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

/* Raw A8B8G8R8 rows of the largest region. Plain kernel main memory: only
 * the CPU touches it (the display hook writes it, vjoReadRaw copies it out),
 * so it needs neither physical contiguity nor cache maintenance. */
#define RAW_SIZE ALIGN(VJO_MAX_W * VJO_MAX_H * 4, 0x1000)

int buffers_alloc(uint32_t bytes)
{
    void *base = NULL;
    SceUID uid;

    if (!bytes || bytes > RAW_SIZE || g.capture_state != CAPTURE_IDLE)
        return VJO_ERR_ARG;
    bytes = ALIGN(bytes, 0x1000);
    if (g.mem_uid > 0 && g.raw && g.raw_capacity == bytes)
        return 0;
    buffers_free();
    g.raw_valid = 0;
    uid = ksceKernelAllocMemBlock("VjoCapture", SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, bytes, NULL);
    if (uid < 0) {
        klog("alloc capture buffer (%u KiB) failed 0x%08X", bytes >> 10, uid);
        /* v0.7's shell treats ALLOC_FAIL as a permanent preflight failure.
         * Keep NONE so a failed full-frame Anki screenshot cannot prevent
         * the next, smaller OCR request. The request returns NO_MEMORY. */
        return uid;
    }
    if (ksceKernelGetMemBlockBase(uid, &base) < 0 || !base) {
        ksceKernelFreeMemBlock(uid);
        klog("capture buffer base unavailable");
        return VJO_ERR_NO_MEMORY;
    }
    g.mem_uid = uid;
    g.raw = (uint8_t *)base;
    g.raw_capacity = bytes;
    g.alloc_status = VJO_ALLOC_OK;
    klog("capture buffer: %u KiB at %p", bytes >> 10, base);
    return 0;
}

void buffers_free(void)
{
    if (g.mem_uid > 0) {
        ksceKernelFreeMemBlock(g.mem_uid);
        klog("capture buffer freed");
    }
    g.mem_uid = 0;
    g.raw = NULL;
    g.raw_capacity = 0;
    g.raw_valid = 0;
    g.alloc_status = VJO_ALLOC_NONE;
}

void buffers_read_done(uint32_t row, uint32_t rows)
{
    /* v0.7's only raw reader is the JPEG encoder, in increasing 16-row
     * bands. This runs under VJO_LOCK, after a successful user copy. */
    if (g.raw_valid && row < g.crop_h && rows >= g.crop_h - row)
        buffers_free();
}
