#ifndef __COLOR_CONVERT_H
#define __COLOR_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    YCBCR_JPEG,
    YCBCR_601,
    YCBCR_709
} YCbCrType;

// 纯软件nv21转rgb888
void nv21_to_rgb888_soft(
    uint32_t width, uint32_t height, 
    const uint8_t *Y, const uint8_t *UV, uint32_t Y_stride, uint32_t UV_stride, 
    uint8_t *RGB, uint32_t RGB_stride, 
    YCbCrType yuv_type);

// 纯软件rgb888转rgb565，rgb565按小端方式存储
void rgb888_to_rgb565(uint8_t *in, uint8_t *out, int h, int v);
// 纯软件rgb888转rgb565，rgb565按大端方式存储
void rgb888_to_rgb565_swap(uint8_t *in, uint8_t *out, int h, int v);
// NEON加速rgb888转rgb565，rgb565按小端方式存储
void rgb888_to_rgb565_neon(uint8_t *in, uint8_t *out, int h, int v);
// NEON加速rgb888转rgb565，rgb565按大端方式存储
void rgb888_to_rgb565_swap_neon(uint8_t *in, uint8_t *out, int h, int v);

#ifdef __cplusplus
}
#endif

#endif
