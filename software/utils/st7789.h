#ifndef __ST7789_H
#define __ST7789_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>

#define ST7789_WIDTH  240
#define ST7789_HEIGHT 240

#define COLOR_WHITE   0xFFFF // 白色
#define COLOR_YELLOW  0xFFE0 // 黄色
#define COLOR_MAGENTA 0xF81F // 品红
#define COLOR_RED     0xF800 // 红色
#define COLOR_LGRAY   0xC618 // 灰白色
#define COLOR_DGRAY   0x7BEF // 深灰色
#define COLOR_OLIVE   0x7BE0 // 橄榄绿
#define COLOR_PURPLE  0x780F // 紫色
#define COLOR_MAROON  0x7800 // 深红色
#define COLOR_CYAN    0x07FF // 青色
#define COLOR_GREEN   0x07E0 // 绿色
#define COLOR_DCYAN   0x03EF // 深青色
#define COLOR_DGREEN  0x03E0 // 深绿色
#define COLOR_BLUE    0x001F // 蓝色
#define COLOR_NAVY    0x000F // 深蓝色
#define COLOR_BLACK   0x0000 // 黑色

int32_t st7789_init(void);
int32_t st7789_exit(void);

void st7789_backlight_on(void);
void st7789_backlight_off(void);

void st7789_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void st7789_draw_point(uint16_t x, uint16_t y, uint16_t color);
void st7789_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t *data);
void st7789_fill(uint16_t color);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __ST7789_H
