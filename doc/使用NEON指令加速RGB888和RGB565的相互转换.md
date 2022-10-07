本项目需要将RGB888转换为RGB565，用C语言转换的代码很简单，这是从ffmpeg中摘抄的代码
```
static inline void rgb24to16_c(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d        = (uint16_t *)dst;
    const uint8_t *s   = src;
    const uint8_t *end = s + src_size;

    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++        = (b >> 3) | ((g & 0xFC) << 3) | ((r & 0xF8) << 8);
    }
}
```

项目中需要转换的数据量不多，用C语言进行转换的CPU开销完全可以接受。但我并不满足于此，芯片支持NEON指令加速，所以为什么不用呢？

简单搜索一番发现国很少有相关的文章，最后还是去外面找了一些有用的资料，在学习NEON指令和寻找的过程中发现ARM官网已经提供了例程，那我正好可以偷懒。

这里给出ARM官网的例程链接，防止有人无法无访问，这里也给出源码
RGB888转RGB565
https://developer.arm.com/documentation/den0018/a/NEON-Code-Examples-with-Optimization/Converting-color-depth/Converting-from-RGB888-to-RGB565
```
uint8_t *src = image_src;
uint16_t *dst = image_dst;
int count = PIXEL_NUMBER;

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
```

RGB565转RGB888
https://developer.arm.com/documentation/den0018/a/NEON-Code-Examples-with-Optimization/Converting-color-depth/Converting-from-RGB565-to-RGB888
```
uint16_t *src = image_src;
uint8_t *dst = image_dst;
int count = PIXEL_NUMBER;

while (count >= 8) {
    uint16x8_t vsrc;
    uint8x8x3_t vdst;

    vsrc = vld1q_u16(src);

    vdst.val[0] = vshrn_n_u16(vreinterpretq_u16_u8(vshrq_n_u8(vreinterpretq_u8_u16(vsrc), 3)), 5);
    vdst.val[1] = vshl_n_u8(vshrn_n_u16(vsrc, 5) ,2);
    vdst.val[2] = vmovn_u16(vshlq_n_u16(vsrc, 3));

    vst3_u8(dst, vdst);

    dst += 8*3;
    src += 8;
    count -= 8;
  }
```

NEON的头文件是 `arm_neon.h`，查阅海思SDK内相关文档可知编译需要加 `-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4`。
接下来就是传统项目，性能测试，随机生成数据的 RGB888 图片转换为 RGB565 图片，重复1000次并计时，测试代码如下
```
#include <arm_neon.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

        // 实际应用中还需要将大端转换为小端，所以加了这一句，性能测试中这一句去掉了
        // vdst = (uint16x8_t)vrev16q_u8((uint8x16_t)vdst);
        vst1q_u16(dst, vdst);

        dst += 8;
        src += 8*3;
        count -= 8;
    }
}


#define WIDTH 320
#define HEIGHT 180

// #define WIDTH 640
// #define HEIGHT 360

// #define WIDTH 960
// #define HEIGHT 540

// #define WIDTH 1280
// #define HEIGHT 720

// #define WIDTH 1600
// #define HEIGHT 900

// #define WIDTH 1920
// #define HEIGHT 1080

#define LOOP 1000

uint8_t rgb888[WIDTH * HEIGHT * 3];
uint16_t rgb565[WIDTH * HEIGHT];
int main(int argc, char **argv)
{
    int32_t i;
    for (i = 0; i < WIDTH * HEIGHT * 3; i++) {
        rgb888[i] = rand() & 0xFF;
    }

    struct timeval tv1;
    struct timeval tv2;
    double td = 0; // ms

    printf("size %d x %d, loop %d\n", WIDTH, HEIGHT, LOOP);

    gettimeofday(&tv1, NULL);
    for (i = 0; i < LOOP; i++) {
        rgb888_to_rgb565(rgb888, (uint8_t *)rgb565, WIDTH, HEIGHT);
    }
    gettimeofday(&tv2, NULL);
    td = ((double)tv2.tv_sec * 1000.0 + (double)tv2.tv_usec / 1000.0) - ((double)tv1.tv_sec * 1000.0 + (double)tv1.tv_usec / 1000.0);
    printf("time c: %f ms\n", td);


    gettimeofday(&tv1, NULL);
    for (i = 0; i < LOOP; i++) {
        rgb888_to_rgb565_neon(rgb888, (uint8_t *)rgb565, WIDTH, HEIGHT);
    }
    gettimeofday(&tv2, NULL);
    td = ((double)tv2.tv_sec * 1000.0 + (double)tv2.tv_usec / 1000.0) - ((double)tv1.tv_sec * 1000.0 + (double)tv1.tv_usec / 1000.0);
    printf("time neon: %f ms\n", td);

    return 0;
}
```

来看一下耗时：
```
size 320 x 180, loop 1000
time c: 1049.035889 ms
time neon: 405.596924 ms

size 640 x 360, loop 1000
time c: 3948.885986 ms
time neon: 2150.033203 ms

size 960 x 540, loop 1000
time c: 9026.308838 ms
time neon: 5033.337891 ms

size 1280 x 720, loop 1000
time c: 16550.081055 ms
time neon: 8756.577881 ms

size 1600 x 900, loop 1000
time c: 25366.738037 ms
time neon: 13618.843994 ms

size 1920 x 1080, loop 1000
time c: 37058.665039 ms
time neon: 20064.520996 ms
```
对于RGB888转RGB565来说，NEON指令的性能大约是C语言的两倍。其他NEON指令的性能未做测试，以上测试内容仅供参考。


最后给出ARM官方的参考文档
NEON指令开发指南
https://developer.arm.com/documentation/den0018/a

NEON指令查询
https://developer.arm.com/architectures/instruction-sets/intrinsics/#f:@navigationhierarchiessimdisa=[Neon]

