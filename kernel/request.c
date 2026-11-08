/* On-demand capture allocation, adapted from Rubi v0.7.12's request path.
 * The exported API remains v9, compatible with the official v0.7 shell. */
#include "vjo_kernel.h"

int capture_request(uint32_t flags)
{
    int ret;
    uint32_t fw, fh, x, y, w, h;
    VJO_LOCK();
    if (flags & ~VJO_CAPTURE_FULL) {
        ret = VJO_ERR_ARG;
    } else if (g.game_pid <= 0 || !g.game_active) {
        ret = VJO_ERR_NO_GAME;
    } else if (g.capture_state != CAPTURE_IDLE) {
        ret = VJO_ERR_BUSY;
    } else {
        fw = g.fb_w ? g.fb_w : VJO_MAX_W;
        fh = g.fb_h ? g.fb_h : VJO_MAX_H;
        if (fw > VJO_MAX_W) fw = VJO_MAX_W;
        if (fh > VJO_MAX_H) fh = VJO_MAX_H;
        if (flags & VJO_CAPTURE_FULL) {
            /* The Anki screenshot is the whole frame at half size (2x2
             * box filter in capture_copy): a full-size buffer (2040 KiB)
             * cannot be allocated on the device (0x80024302). */
            x = y = 0;
            w = fw >> 1;
            h = fh >> 1;
            if (!w || !h) {
                ret = VJO_ERR_ARG;
                goto out;
            }
        } else {
            capture_compute_crop(fw, fh, &x, &y, &w, &h);
        }
        if (buffers_alloc(w * h * 4) < 0) {
            ret = VJO_ERR_NO_MEMORY;
            goto out;
        }
        g.capture_x = x;
        g.capture_y = y;
        g.capture_fb_w = fw;
        g.capture_fb_h = fh;
        g.crop_w = w;
        g.crop_h = h;
        g.capture_seq = (g.capture_seq + 1) & 0x7FFFFFFF;
        if (!g.capture_seq) g.capture_seq = 1;
        ret = (int)g.capture_seq;
        g.raw_valid = 0;
        g.capture_full = (flags & VJO_CAPTURE_FULL) != 0;
        if (!g.capture_full) g.region_seq = 0;
        g.capture_requested_us = ksceKernelGetSystemTimeWide();
        __atomic_store_n(&g.capture_state, CAPTURE_PENDING, __ATOMIC_RELEASE);
    }
out:
    VJO_UNLOCK();
    return ret;
}
