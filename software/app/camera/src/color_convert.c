#include "color_convert.h"
#include <stdio.h>
#include <arm_neon.h>

#define FIXED_POINT_VALUE(value, precision) ((int)(((value)*(1<<precision))+0.5))

typedef struct
{
    uint8_t cb_factor;   // [(255*CbNorm)/CbRange]
    uint8_t cr_factor;   // [(255*CrNorm)/CrRange]
    uint8_t g_cb_factor; // [Bf/Gf*(255*CbNorm)/CbRange]
    uint8_t g_cr_factor; // [Rf/Gf*(255*CrNorm)/CrRange]
    uint8_t y_factor;    // [(YMax-YMin)/255]
    uint8_t y_offset;    // YMin
} YUV2RGBParam;

#define YUV2RGB_PARAM(Rf, Bf, YMin, YMax, CbCrRange) \
{.cb_factor=FIXED_POINT_VALUE(255.0*(2.0*(1-Bf))/CbCrRange, 6), \
.cr_factor=FIXED_POINT_VALUE(255.0*(2.0*(1-Rf))/CbCrRange, 6), \
.g_cb_factor=FIXED_POINT_VALUE(Bf/(1.0-Bf-Rf)*255.0*(2.0*(1-Bf))/CbCrRange, 7), \
.g_cr_factor=FIXED_POINT_VALUE(Rf/(1.0-Bf-Rf)*255.0*(2.0*(1-Rf))/CbCrRange, 7), \
.y_factor=FIXED_POINT_VALUE(255.0/(YMax-YMin), 7), \
.y_offset=YMin}

static const YUV2RGBParam YUV2RGB[3] = {
    // ITU-T T.871 (JPEG)
    YUV2RGB_PARAM(0.299, 0.114, 0.0, 255.0, 255.0),
    // ITU-R BT.601-7
    YUV2RGB_PARAM(0.299, 0.114, 16.0, 235.0, 224.0),
    // ITU-R BT.709-6
    YUV2RGB_PARAM(0.2126, 0.0722, 16.0, 235.0, 224.0)
};

static inline uint8_t clamp(int16_t value)
{
    return value<0 ? 0 : (value>255 ? 255 : value);
}

void nv21_to_rgb888_soft(
    uint32_t width, uint32_t height, 
    const uint8_t *Y, const uint8_t *UV, uint32_t Y_stride, uint32_t UV_stride, 
    uint8_t *RGB, uint32_t RGB_stride, 
    YCbCrType yuv_type)
{
    const YUV2RGBParam *const param = &(YUV2RGB[yuv_type]);
    uint32_t x, y;
    for(y=0; y<(height-1); y+=2)
    {
        const uint8_t *y_ptr1=Y+y*Y_stride,
            *y_ptr2=Y+(y+1)*Y_stride,
            *uv_ptr=UV+(y/2)*UV_stride;
        
        uint8_t *rgb_ptr1=RGB+y*RGB_stride,
            *rgb_ptr2=RGB+(y+1)*RGB_stride;
        
        for(x=0; x<(width-1); x+=2)
        {
            int8_t u_tmp, v_tmp;
            u_tmp = uv_ptr[1]-128;
            v_tmp = uv_ptr[0]-128;
            
            //compute Cb Cr color offsets, common to four pixels
            int16_t b_cb_offset, r_cr_offset, g_cbcr_offset;
            b_cb_offset = (param->cb_factor*u_tmp)>>6;
            r_cr_offset = (param->cr_factor*v_tmp)>>6;
            g_cbcr_offset = (param->g_cb_factor*u_tmp + param->g_cr_factor*v_tmp)>>7;
            
            int16_t y_tmp;
            y_tmp = (param->y_factor*(y_ptr1[0]-param->y_offset))>>7;
            rgb_ptr1[0] = clamp(y_tmp + r_cr_offset);
            rgb_ptr1[1] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr1[2] = clamp(y_tmp + b_cb_offset);
            
            y_tmp = (param->y_factor*(y_ptr1[1]-param->y_offset))>>7;
            rgb_ptr1[3] = clamp(y_tmp + r_cr_offset);
            rgb_ptr1[4] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr1[5] = clamp(y_tmp + b_cb_offset);
            
            y_tmp = (param->y_factor*(y_ptr2[0]-param->y_offset))>>7;
            rgb_ptr2[0] = clamp(y_tmp + r_cr_offset);
            rgb_ptr2[1] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr2[2] = clamp(y_tmp + b_cb_offset);
            
            y_tmp = (param->y_factor*(y_ptr2[1]-param->y_offset))>>7;
            rgb_ptr2[3] = clamp(y_tmp + r_cr_offset);
            rgb_ptr2[4] = clamp(y_tmp - g_cbcr_offset);
            rgb_ptr2[5] = clamp(y_tmp + b_cb_offset);
            
            rgb_ptr1 += 6;
            rgb_ptr2 += 6;
            y_ptr1 += 2;
            y_ptr2 += 2;
            uv_ptr += 2;
        }
    }
}

void rgb888_to_rgb565(uint8_t *in, uint8_t *out, int h, int v)
{
    uint16_t *d        = (uint16_t *)out;
    const uint8_t *s   = in;
    const uint8_t *end = s + h * v * 3;

    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++        = (b >> 3) | ((g & 0xFC) << 3) | ((r & 0xF8) << 8);
    }
}

void rgb888_to_rgb565_swap(uint8_t *in, uint8_t *out, int h, int v)
{
    uint16_t *d        = (uint16_t *)out;
    const uint8_t *s   = in;
    const uint8_t *end = s + h * v * 3;

    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++        = ((b & 0xF8) << 5) | ((g & 0x1C) << 11) | ((g & 0xE0) >> 5) | (r & 0xF8);
    }
}

void rgb888_to_rgb565_neon(uint8_t *in, uint8_t *out, int h, int v)
{
    uint8_t *src = in;
    uint16_t *dst = (uint16_t *)out;
    int count = h * v;

    if (count % 8 != 0) {
        printf("pixel number must align with 8\n");
        return;
    }

    while (count >= 8) {
        uint8x8x3_t vsrc;
        uint16x8_t vdst;

        vsrc = vld3_u8(src);

        vdst = vshll_n_u8(vsrc.val[0], 8);
        vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[1], 8), 5);
        vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[2], 8), 11);

        vst1q_u16(dst, vdst);

        dst += 8;
        src += 8*3;
        count -= 8;
    }
}

void rgb888_to_rgb565_swap_neon(uint8_t *in, uint8_t *out, int h, int v)
{
    uint8_t *src = in;
    uint16_t *dst = (uint16_t *)out;
    int count = h * v;

    if (count % 8 != 0) {
        printf("pixel number must align with 8\n");
        return;
    }

    while (count >= 8) {
        uint8x8x3_t vsrc;
        uint16x8_t vdst;

        vsrc = vld3_u8(src);

        vdst = vshll_n_u8(vsrc.val[0], 8);
        vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[1], 8), 5);
        vdst = vsriq_n_u16(vdst, vshll_n_u8(vsrc.val[2], 8), 11);

        vdst = (uint16x8_t)vrev16q_u8((uint8x16_t)vdst);
        vst1q_u16(dst, vdst);

        dst += 8;
        src += 8*3;
        count -= 8;
    }
}
