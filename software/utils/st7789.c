#include "st7789.h"
#include "spi.h"
#include <linux/spi/spidev.h>
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SPI_BUS_NUM   (0)
#define SPI_CS_NUM    (0)
#define DC_GPIO_GROUP (7)
#define DC_GPIO_NUM   (3)
#define BL_GPIO_GROUP (0)
#define BL_GPIO_NUM   (4)

#define GPIO_NUM(group, num) ((group)*8+(num))

typedef struct {
    spi_t *spi_bus;
    gpio_t *st7789_dc;
    gpio_t *st7789_bl; // 背光控制引脚
} st7789_t;

static st7789_t st7789_ctx;


static int32_t st7789_hw_init()
{
    int32_t err = 0;
    char spi_bus_name[32] = {};

    st7789_ctx.spi_bus = spi_new();
    sprintf(spi_bus_name, "/dev/spidev%d.%d", SPI_BUS_NUM, SPI_CS_NUM);
    err = spi_open(st7789_ctx.spi_bus, spi_bus_name, SPI_MODE_0, 50000000);
    if (err) {
        printf("spi %s init err with (%#x)\n", spi_bus_name, err);
        return err;
    }

    st7789_ctx.st7789_dc = gpio_new();
    err = gpio_open_sysfs(st7789_ctx.st7789_dc, GPIO_NUM(DC_GPIO_GROUP, DC_GPIO_NUM), GPIO_DIR_OUT);
    if (err) {
        printf("gpio %d-%d init err with (%#x)\n", BL_GPIO_GROUP, BL_GPIO_NUM, err);
        return err;
    }

    st7789_ctx.st7789_bl = gpio_new();
    err = gpio_open_sysfs(st7789_ctx.st7789_bl, GPIO_NUM(BL_GPIO_GROUP, BL_GPIO_NUM), GPIO_DIR_OUT);
    if (err) {
        printf("gpio %d-%d init err with (%#x)\n", BL_GPIO_GROUP, BL_GPIO_NUM, err);
        return err;
    }

    return 0;
}

static int32_t st7789_hw_exit()
{
    spi_close(st7789_ctx.spi_bus);
    spi_free(st7789_ctx.spi_bus);

    gpio_close(st7789_ctx.st7789_dc);
    gpio_free(st7789_ctx.st7789_dc);

    gpio_close(st7789_ctx.st7789_bl);
    gpio_free(st7789_ctx.st7789_bl);

    memset(&st7789_ctx, 0, sizeof(st7789_ctx));

    return 0;
}

static void st7789_write_cmd(uint8_t cmd)
{
    uint8_t buf[4] = {0};
    buf[0] = cmd;
    gpio_write(st7789_ctx.st7789_dc, 0);
    spi_transfer(st7789_ctx.spi_bus, buf, buf, 1);
}

static void st7789_write_data(uint8_t dat)
{
    uint8_t buf[4] = {0};
    buf[0] = dat;
    gpio_write(st7789_ctx.st7789_dc, 1);
    spi_transfer(st7789_ctx.spi_bus, buf, buf, 1);
}

static void st7789_write_data_len(uint8_t *dat, uint16_t len)
{
    // linux 内核限制，单次读写长度不得超过4KB
    uint8_t rxbuf[4096] = {0};
    int32_t i, page, offset, wlen;
    gpio_write(st7789_ctx.st7789_dc, 1);

    page = (len - 1) / 4096 + 1;
    offset = 0;
    for (i = 0; i < page; i++) {
        wlen = len - offset > 4096 ? 4096 : len - offset;
        spi_transfer(st7789_ctx.spi_bus, &dat[offset], rxbuf, wlen);
        offset += 4096;
    }
}

void st7789_backlight_on(void)
{
    gpio_write(st7789_ctx.st7789_bl, 1);
}

void st7789_backlight_off(void)
{
    gpio_write(st7789_ctx.st7789_bl, 0);
}

int32_t st7789_init()
{
    st7789_hw_init();

    /* Sleep Out */
    st7789_write_cmd(0x11);
    /* wait for power stability */
    usleep(120 * 1000);

    /* Memory Data Access Control */
    st7789_write_cmd(0x36);
    st7789_write_data(0x00);

    /* RGB 5-6-5-bit  */
    st7789_write_cmd(0x3A);
    st7789_write_data(0x55);

    /* Porch Setting */
    st7789_write_cmd(0xB2);
    st7789_write_data(0x0C);
    st7789_write_data(0x0C);
    st7789_write_data(0x00);
    st7789_write_data(0x33);
    st7789_write_data(0x33);

    /*  Gate Control */
    st7789_write_cmd(0xB7);
    st7789_write_data(0x72);

    /* VCOM Setting */
    st7789_write_cmd(0xBB);
    st7789_write_data(0x3D);   //Vcom=1.625V

    /* LCM Control */
    st7789_write_cmd(0xC0);
    st7789_write_data(0x2C);

    /* VDV and VRH Command Enable */
    st7789_write_cmd(0xC2);
    st7789_write_data(0x01);

    /* VRH Set */
    st7789_write_cmd(0xC3);
    st7789_write_data(0x19);

    /* VDV Set */
    st7789_write_cmd(0xC4);
    st7789_write_data(0x20);

    /* Frame Rate Control in Normal Mode */
    st7789_write_cmd(0xC6);
    st7789_write_data(0x0F);	//60MHZ

    /* Power Control 1 */
    st7789_write_cmd(0xD0);
    st7789_write_data(0xA4);
    st7789_write_data(0xA1);

    /* Positive Voltage Gamma Control */
    st7789_write_cmd(0xE0);
    st7789_write_data(0xD0);
    st7789_write_data(0x04);
    st7789_write_data(0x0D);
    st7789_write_data(0x11);
    st7789_write_data(0x13);
    st7789_write_data(0x2B);
    st7789_write_data(0x3F);
    st7789_write_data(0x54);
    st7789_write_data(0x4C);
    st7789_write_data(0x18);
    st7789_write_data(0x0D);
    st7789_write_data(0x0B);
    st7789_write_data(0x1F);
    st7789_write_data(0x23);

    /* Negative Voltage Gamma Control */
    st7789_write_cmd(0xE1);
    st7789_write_data(0xD0);
    st7789_write_data(0x04);
    st7789_write_data(0x0C);
    st7789_write_data(0x11);
    st7789_write_data(0x13);
    st7789_write_data(0x2C);
    st7789_write_data(0x3F);
    st7789_write_data(0x44);
    st7789_write_data(0x51);
    st7789_write_data(0x2F);
    st7789_write_data(0x1F);
    st7789_write_data(0x1F);
    st7789_write_data(0x20);
    st7789_write_data(0x23);

    /* Display Inversion On */
    st7789_write_cmd(0x21);

    /* Display On */
    st7789_write_cmd(0x29);

    return 0;
}

int32_t st7789_exit(void)
{
    st7789_backlight_off();
    st7789_hw_exit();

    return 0;
}

void st7789_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    st7789_write_cmd(0x2a);
    st7789_write_data(x1 >> 8);
    st7789_write_data(x1);
    st7789_write_data(x2 >> 8);
    st7789_write_data(x2);

    st7789_write_cmd(0x2b);
    st7789_write_data(y1 >> 8);
    st7789_write_data(y1);
    st7789_write_data(y2 >> 8);
    st7789_write_data(y2);
}

void st7789_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t buf[4] = {0};

    st7789_set_area(x, y, x, y);
    st7789_write_cmd(0x2C);

    buf[0] = color >> 8;
    buf[1] = color & 0xFF;
    gpio_write(st7789_ctx.st7789_dc, 1);
    spi_transfer(st7789_ctx.spi_bus, buf, buf, 2);
}

void st7789_draw_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t *data)
{
    st7789_set_area(x1, y1, x2, y2);
    st7789_write_cmd(0x2C);
    st7789_write_data_len(data, (x2 - x1 + 1) * (y2 - y1 + 1) * sizeof(uint16_t));
}

void st7789_fill(uint16_t color)
{
    uint16_t i, c;
    uint8_t buf[3840]; // 240 * 8 * 2

    st7789_set_area(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);
    st7789_write_cmd(0x2C);

    //LCD屏幕色彩深度16bit，低八位是颜色数据的高位，高八位是颜色数据的低位
    c = ((color & 0xFF00)>> 8) | ((color & 0x00FF) << 8);
    for (i = 0; i < sizeof(buf) / sizeof(uint16_t); i++) {
        ((uint16_t *)buf)[i] = c;
    }

    for (i = 0; i < 30; i++) {
        st7789_write_data_len(buf, sizeof(buf));
    }
}
