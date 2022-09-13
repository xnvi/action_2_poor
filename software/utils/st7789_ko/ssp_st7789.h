#ifndef __HI_SSP_ST7789_H
#define __HI_SSP_ST7789_H

#define SSP_LCD_SET_BRIGHTNESS     0x11
#define SSP_LCD_SET_BACKLIGHT_ON   0x12
#define SSP_LCD_SET_BACKLIGHT_OFF  0x13
#define SSP_LCD_SET_REG            0x14
#define SSP_LCD_SET_DRAW_POINT     0x15
#define SSP_LCD_SET_DRAW_AREA      0x16
#define SSP_LCD_TEST               0x20

typedef union
{
    struct
    {
        unsigned char cmd;
        unsigned char len;
        unsigned char data[4]; // 数据统一按 uint8_t 处理，对于 uint16 等类型的数据注意大小端问题
    } wreg; // 对应 SSP_LCD_SET_REG
    struct
    {
        unsigned short x;
        unsigned short y;
        unsigned short color;
    } point; // 对应 SSP_LCD_SET_DRAW_POINT
    struct
    {
        unsigned short x1;
        unsigned short y1;
        unsigned short x2;
        unsigned short y2;
        unsigned int len;
        unsigned char *data;
    } area; // 对应 SSP_LCD_SET_DRAW_AREA
} ssp_st7789;

/*
示例
if ((lcd_fd = open("/dev/st7789_lcd", O_RDWR)) == -1) {
    printf("open failed\n");
    return 0;
}

// ioctl(lcd_fd, SSP_LCD_TEST, NULL);

ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_ON, NULL);

ssp_st7789 lcd_data;
lcd_data.point.x = 10;
lcd_data.point.y = 10;
lcd_data.point.color = 0x0000;
ioctl(lcd_fd, SSP_LCD_SET_DRAW_POINT, (unsigned long)&lcd_data);

lcd_data.area.x1 = 20;
lcd_data.area.x2 = 39;
lcd_data.area.y1 = 20;
lcd_data.area.y2 = 39;
lcd_data.area.len = 400;
lcd_data.area.data = lcd_buf;
ioctl(lcd_fd, SSP_LCD_SET_DRAW_AREA, (unsigned long)&lcd_data);

ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_OFF, NULL);

close(lcd_fd);
*/

#endif
