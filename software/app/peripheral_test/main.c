#include <signal.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <time.h>

#include "himm.h"
#include "i2c.h"
#include <linux/spi/spidev.h>
#include "spi.h"
#include "gpio.h"
#include "ft6206.h"
#include "ov5640.h"
#include "st7789.h"

// 读取内部寄存器并计算温度
float read_temperature()
{
    float temp = 0.0;
    uint32_t reg = 0;
    reg = himd(0x120280BC);
    reg = reg & 0x00000FFF;
    temp = ((reg-117)/798.0)*165-40;
    return temp;
}

int set_reg()
{
    // I2C0 设置
    // I2C0_SCL I2C0_SDA 设置为 I2C
    himm(0x112c0030, 0x00001c01);
    himm(0x112c0034, 0x00001c01);
    // LCD_DATA6 LCD_DATA7 设置为 GPIO
    himm(0x112c0060, 0x00001170);
    himm(0x112c0064, 0x00001170);

    // I2C2 设置
    // I2C2_SCL I2C2_SDA 设置为 GPIO
    himm(0x112c0038, 0x00001c00);
    himm(0x112c003c, 0x00001c00);
    // LCD_DATA4 LCD_DATA5 设置为 I2C2
    himm(0x112c0058, 0x00001172);
    himm(0x112c005c, 0x00001172);


    // SPI0 设置
    // SPI0_SCLK
    himm(0x112C003C, 0x00001c00);
    himm(0x112C0074, 0x00001177);
    // SPI0_SDO
    himm(0x112C0038, 0x00001c00);
    himm(0x112C006C, 0x00001137);
    // SPI0_SDI
    himm(0x112C0070, 0x00001137);
    // SPI0_CSN
    himm(0x112C0040, 0x00001000);
    himm(0x112C0068, 0x00001137);


    // GPIO_0_0 设置
    himm(0x100C0008, 0x00001D30); // GPIO
    // himm(0x100C0008, 0x00001D31); // UPDATE_MODE


    // PWM 设置
	// TODO

    // ADC 设置
	// TODO


    // 音视频等相关设置
    // 时钟引脚
    himm(0x112c0028, 0x00001001);
    himm(0x112c0054, 0x00001A00);
    // 开启24MHz时钟
    himm(0x120100f0, 0x0000000D);

    // MIPI 接口设置
    // MIPI_RX_CK0N
    himm(0x112c000, 0x00001000);
    // MIPI_RX_CK0P
    himm(0x112c004, 0x00001000);
    // MIPI_RX_D0N
    himm(0x112c008, 0x00001000);
    // MIPI_RX_D0P
    himm(0x112c00c, 0x00001000);
    // MIPI_RX_D2N
    himm(0x112c010, 0x00001000);
    // MIPI_RX_D2P
    himm(0x112c014, 0x00001000);

    return 0;
}

extern void hi_ssp_demo(void);
int main(int argc, char **argv)
{
    int32_t i = 0;
    bool io_val = 0;
    int i2c_fd0 = 0;
    int i2c_fd2 = 0;
    I2CDevice dev_ft6206;
    I2CDevice dev_ov5640;

    gpio_t *gpio_0_0;

    printf("hello world\n");

    memopen();
    set_reg();

    printf("core temperature %.2f\n", read_temperature());


    // GPIO_0_0 测试
    gpio_0_0 = gpio_new();
    gpio_open_sysfs(gpio_0_0, 0, GPIO_DIR_OUT);
    printf("==== GPIO write test ====\n");
    printf("  Test with voltmeter to check output voltage.\n");
    for (i = 0; i < 3; i++) {
        printf("GPIO_0_0 write 1\n");
        gpio_write(gpio_0_0, 1);
        sleep(1);
        printf("GPIO_0_0 write 0\n");
        gpio_write(gpio_0_0, 0);
        sleep(1);
    }
    printf("==== GPIO read test ====\n");
    printf("  Press the button to check input value.\n");
    gpio_set_direction(gpio_0_0, GPIO_DIR_IN);
    for (i = 0; i < 50; i++) {
        gpio_read(gpio_0_0, &io_val);
        printf("count %d, gpio_0_0 read %d\n", i, io_val);
        usleep(100 * 1000);
    }
    gpio_close(gpio_0_0);
    gpio_free(gpio_0_0);
    
    i2c_fd0 = i2c_open("/dev/i2c-0");
    if (i2c_fd0 < 0) {
        printf("open /dev/i2c-0 error");
        return -1;
    }
    i2c_fd2 = i2c_open("/dev/i2c-2");
    if (i2c_fd2 < 0) {
        printf("open /dev/i2c-2 error");
        return -1;
    }

	// 触摸屏测试
    printf("==== FT6206 touch test ====\n");
    printf("  Touch the screen to check touch coordinate.\n");
    dev_ft6206.bus = i2c_fd2;
    dev_ft6206.addr = FT6202_I2C_ADDR;
    dev_ft6206.tenbit = 0;
    dev_ft6206.delay = 1;
    dev_ft6206.flags = 0;
    dev_ft6206.page_bytes = 256;
    dev_ft6206.iaddr_bytes = 1; /* Set this to zero, and using i2c_ioctl_xxxx API will ignore chip internal address */
    ft6206_init((void *)&dev_ft6206);
    ft6206_touch_info touch_info;
    memset(&touch_info, 0, sizeof(touch_info));
    for(i = 0; i < 100; i++) {
        ft6206_read((void *)&dev_ft6206, &touch_info);
        if (touch_info.num > 0) {
            printf("num %d, pos0 %d %d, pos1 %d %d \n", touch_info.num,
                    touch_info.points[0].x, touch_info.points[0].y,
                    touch_info.points[1].x, touch_info.points[1].y);
        }
        usleep(50 * 1000);
    }


	// OV5640 寄存器读写测试
    printf("==== OV5640 read write test ====\n");
    dev_ov5640.bus = i2c_fd0;
    dev_ov5640.addr = OV5640_DEV_ADDR;
    dev_ov5640.tenbit = 0;
    dev_ov5640.delay = 1;
    dev_ov5640.flags = 0;
    dev_ov5640.page_bytes = 256;
    dev_ov5640.iaddr_bytes = 2; /* Set this to zero, and using i2c_ioctl_xxxx API will ignore chip internal address */
    OV5640_Init((void *)&dev_ov5640);

    i2c_close(i2c_fd0);
    i2c_close(i2c_fd2);


    // 液晶屏测试
    printf("==== ST7789 screen test ====\n");
    printf("  Check whether the screen backlight is work.\n");
    printf("  Check whether the screen color is normal.\n");
    st7789_init();
    st7789_backlight_on();
    st7789_fill(COLOR_WHITE);
    sleep(1);
    for (i = 0; i < 2; i++) {
        st7789_fill(COLOR_YELLOW);
        st7789_fill(COLOR_MAGENTA);
        st7789_fill(COLOR_RED);
        st7789_fill(COLOR_LGRAY);
        st7789_fill(COLOR_DGRAY);
        st7789_fill(COLOR_OLIVE);
        st7789_fill(COLOR_PURPLE);
        st7789_fill(COLOR_MAROON);
        st7789_fill(COLOR_CYAN);
        st7789_fill(COLOR_GREEN);
        st7789_fill(COLOR_DCYAN);
        st7789_fill(COLOR_DGREEN);
        st7789_fill(COLOR_BLUE);
        st7789_fill(COLOR_NAVY);
        st7789_fill(COLOR_BLACK);
        st7789_fill(COLOR_WHITE);
    }
    sleep(1);
    st7789_backlight_off();
    st7789_exit();


    memclose();

    printf("==== test finish ====\n");
    
    return 0;
}
