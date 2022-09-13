/*
 * Copyright (c) 2006 Hisilicon Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 * History:
 *      21-April-2006 create this file
 */

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/poll.h>

#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>

#include <linux/moduleparam.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/gpio.h>

#include "ssp_st7789.h"


#ifdef __HuaweiLite__
#include "asm/io.h"
#endif


#ifdef __HuaweiLite__
#else
#include <mach/io.h>/* for IO_ADDRESS */
#endif

#define SSP_BASE    0x12070000
#define SSP_SIZE    0x1000          // 4KB


#define DEFAULT_MD_LEN (128)

#ifdef __HuaweiLite__

#define IO_ADDRESS_VERIFY(x) (x)

#else

#define CRG_BASE       0x12010000
#define GPIO_AHB_BASE  0x100C0000
#define GPIO_VIVO_BASE 0x112C0000

void __iomem *reg_ssp_base_va;
void __iomem *reg_crg_base_va;
void __iomem *reg_gpio_ahb_base_va;
void __iomem *reg_gpio_vivo_base_va;
#define IO_ADDRESS_VERIFY(x) (reg_ssp_base_va + ((x)-(SSP_BASE)))

#endif

/* SSP register definition .*/
#define SSP_CR0              IO_ADDRESS_VERIFY(SSP_BASE + 0x00)
#define SSP_CR1              IO_ADDRESS_VERIFY(SSP_BASE + 0x04)
#define SSP_DR               IO_ADDRESS_VERIFY(SSP_BASE + 0x08)
#define SSP_SR               IO_ADDRESS_VERIFY(SSP_BASE + 0x0C)
#define SSP_CPSR             IO_ADDRESS_VERIFY(SSP_BASE + 0x10)
#define SSP_IMSC             IO_ADDRESS_VERIFY(SSP_BASE + 0x14)
#define SSP_RIS              IO_ADDRESS_VERIFY(SSP_BASE + 0x18)
#define SSP_MIS              IO_ADDRESS_VERIFY(SSP_BASE + 0x1C)
#define SSP_ICR              IO_ADDRESS_VERIFY(SSP_BASE + 0x20)
#define SSP_DMACR            IO_ADDRESS_VERIFY(SSP_BASE + 0x24)

#define SPI_SR_BSY        (0x1 << 4)/* spi busy flag */
#define SPI_SR_TFE        (0x1 << 0)/* Whether to send fifo is empty */
#define SPI_DATA_WIDTH    (8)
#define SPI_SPO           (0)
#define SPI_SPH           (0)
#define SPI_SCR           (0)
#define SPI_CPSDVSR       (2)
#define SPI_FRAMEMODE     (0)

#define MAX_WAIT 10000

#define  ssp_readw(addr,ret)       (ret =(*(volatile unsigned int *)(addr)))
#define  ssp_writew(addr,value)    ((*(volatile unsigned int *)(addr)) = (value))

#define ST7789_DC_PIN     (7*8+3) // gpio7-3
#define LCD_BACKLIGHT_PIN (0*8+4) // gpio0-4

#define KBUF_SIZE (4096)

static int ssp_set_reg(unsigned int Addr, unsigned int Value)
{
#ifdef __HuaweiLite__
    (*(volatile unsigned int *)(Addr)) = Value;
#else
    void* pmem = ioremap_nocache(Addr, DEFAULT_MD_LEN);
    if (pmem == NULL)
    {
        return -1;
    }

    *(unsigned int*)pmem = Value;
    iounmap(pmem);
#endif
    return 0;
}

static inline void hi_spi_delay(void)
{
    volatile unsigned int tmp = 0;
    while (tmp++ < 32);
    // 35 可以保证在50MHz时钟下，每两字节之间间隔 100 ns，
    // 32 可以保证在50MHz时钟下，每两字节之间间隔 80 ns，
    // 30 可以保证在50MHz时钟下，每两字节之间间隔 65 ns，
    // 不加延时的间隔是60 ns，可能存在丢数据风险，待确认
    // 延时的时长一定要根据SPI的时钟修改并实际测试，确保每字节之间有一定间隔
}

static int hi_spi_check_timeout(void)
{
    unsigned int value =  0;
    unsigned int tmp = 0;

    while (1)
    {
        ssp_readw(SSP_SR,value);
        if ((value & SPI_SR_TFE) && (!(value & SPI_SR_BSY)))
        {
            break;
        }

        if (tmp++ > MAX_WAIT)
        {
            printk("spi transfer wait timeout!\n");
            return -1;
        }
        udelay(1);
        // 低速通信可以加这个延迟
        // 为保证写入下一个字节时上一个字节已经发送完，发送完一个字节的时间应当小于1us，
        // 所以这里对高低、低速的定义为：写入一个字节的时间小于1us即为高速，也就是spi时钟大于8MHz为高速
        // 这个延迟一定程度上会拖慢spi的通信速度，LCD屏的数据量比较大，为了提高速率可以不加延迟
    }
    return 0;
}

static void hi_ssp_writeOnly(int bWriteOnly)
{
    unsigned int  ret = 0;
    
    ssp_readw(SSP_CR1,ret);

    if (bWriteOnly)
    {
        ret = ret | (0x1 << 5);
    }
    else
    {
        ret = ret & (~(0x1 << 5));
    }

    ssp_writew(SSP_CR1,ret);
}


static void hi_ssp_enable(void)
{
    unsigned int ret = 0;
    ssp_readw(SSP_CR1,ret);
    ret = (ret & 0xFFFD) | 0x2;

    ret = ret | (0x1 << 4); /* big/little end, 1: little, 0: big */

    ret = ret | (0x1 << 15); /* wait en */

    ssp_writew(SSP_CR1,ret);

    hi_ssp_writeOnly(0);
}


static void hi_ssp_disable(void)
{
    unsigned int ret = 0;
    ssp_readw(SSP_CR1,ret);
    ret = ret & (~(0x1 << 1));
    ssp_writew(SSP_CR1,ret);
}

static int hi_ssp_set_frameform(unsigned char framemode,unsigned char spo,unsigned char sph,unsigned char datawidth)
{
    unsigned int ret = 0;
    ssp_readw(SSP_CR0,ret);
    if(framemode > 3)
    {
        printk("set frame parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFCF) | (framemode << 4);
    if((ret & 0x30) == 0)
    {
        if(spo > 1)
        {
            printk("set spo parameter err.\n");
            return -1;
        }
        if(sph > 1)
        {
            printk("set sph parameter err.\n");
            return -1;
        }
        ret = (ret & 0xFF3F) | (sph << 7) | (spo << 6);
    }
    if((datawidth > 16) || (datawidth < 4))
    {
        printk("set datawidth parameter err.\n");
        return -1;
    }
    ret = (ret & 0xFFF0) | (datawidth -1);
    ssp_writew(SSP_CR0,ret);
    return 0;
}

static int hi_ssp_set_serialclock(unsigned char scr,unsigned char cpsdvsr)
{
    unsigned int ret = 0;
    ssp_readw(SSP_CR0,ret);
    ret = (ret & 0xFF) | (scr << 8);
    ssp_writew(SSP_CR0,ret);
    if((cpsdvsr & 0x1))
    {
        printk("set cpsdvsr parameter err.\n");
        return -1;
    }
    ssp_writew(SSP_CPSR,cpsdvsr);
    return 0;
}

static int hi_ssp_alt_mode_set(int enable)
{
    unsigned int ret = 0;

    ssp_readw(SSP_CR1,ret);
    if (enable)
    {
        ret = ret & (~0x40);
    }
    else
    {
        ret = (ret & 0xFF) | 0x40;
    }
    ssp_writew(SSP_CR1,ret);

    return 0;
}

static void spi_enable(void)
{
    ssp_writew(SSP_CR1, 0x42);
}

static void spi_disable(void)
{
    ssp_writew(SSP_CR1, 0x40);
}

static inline void write_data_mode(void)
{
    gpio_set_value(ST7789_DC_PIN, 1);
}

static inline void write_cmd_mode(void)
{
    gpio_set_value(ST7789_DC_PIN, 0);
}

static void spi_write_byte(unsigned char dat)
{
    unsigned short spi_data = 0;

    spi_data = dat;
    ssp_writew(SSP_DR, spi_data);

    // 这个延迟函数一定要根据CPU主频、SPI时钟频率实际测量后进行调整
    hi_spi_delay();
}

// 一次性写入不大于255字节的数据
static void spi_write_len(unsigned char *dat, unsigned char len)
{
    int i, ret = 0;

    for (i = 0; i < len; i++) {
        ssp_writew(SSP_DR, dat[i]);
    }
    ret = hi_spi_check_timeout();
    if(ret != 0) {
        printk("spi write timeout\n");
    }
}

static void ssp_write_dat(unsigned char dat)
{
    write_data_mode();
    spi_write_byte(dat);
}

static void ssp_write_cmd(unsigned char dat)
{
    write_cmd_mode();
    spi_write_byte(dat);
}

static void st7789_set_area(unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2)
{
    ssp_write_cmd(0x2a);
    ssp_write_dat(x1 >> 8);
    ssp_write_dat(x1);
    ssp_write_dat(x2 >> 8);
    ssp_write_dat(x2);

    ssp_write_cmd(0x2b);
    ssp_write_dat(y1 >> 8);
    ssp_write_dat(y1);
    ssp_write_dat(y2 >> 8);
    ssp_write_dat(y2);
}

static void st7789_draw_point(unsigned short x, unsigned short y, unsigned short color)
{
    spi_enable();
    st7789_set_area(x, y, x, y);
    ssp_write_cmd(0x2C);
    ssp_write_dat(color >> 8);
    ssp_write_dat(color);
    spi_disable();
}

static int st7789_draw_area(unsigned short x1, unsigned short y1,
                            unsigned short x2, unsigned short y2,
                            unsigned int len, unsigned char *data)
{
    int i, j;
    unsigned char *kbuf;
    int page, offset, wlen;

    kbuf = kmalloc(KBUF_SIZE, GFP_KERNEL);
    if (kbuf == NULL) {
        return -ENOMEM;
    }

    spi_enable();
    st7789_set_area(x1, y1, x2, y2);
    ssp_write_cmd(0x2C);
    write_data_mode();

    page = (len + 4095) / 4096;
    offset = 0;
    for (i = 0; i < page; i++) {
        wlen = len - offset > 4096 ? 4096 : len - offset;
        copy_from_user(kbuf, &data[offset], wlen);
        for (j = 0; j < wlen; j++) {
            spi_write_byte(kbuf[j]);
            // TODO 用这个批量写入应该更好
            // spi_write_len();
        }
        offset += 4096;
    }

    spi_disable();
    kfree(kbuf);
    return 0;
}

static void st7789_fill(unsigned short color)
{
    int i;

    st7789_set_area(0, 0, 239, 239);
    ssp_write_cmd(0x2C);

    write_data_mode();
    for (i = 0; i < 240 * 240; i++) {
        spi_write_byte(color >> 8);
        spi_write_byte(color);
    }
}

static long ssp_lcd_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int *argp = (unsigned int *)arg;
    ssp_st7789 st7789_ctl;
    if((cmd&0xf000 )== 0xf000)
    {
        printk(KERN_EMERG "KERN_EMERG cmd = 0x%x,0x%x,0x%x\n",cmd,((cmd&(0x1<<8))>>8),cmd&0xff);
        // spi_write_a9byte(((cmd&(0x1<<8))>>8),cmd&0xff);
        return 0;
    }

    if (argp != NULL)
    {
        if (copy_from_user(&st7789_ctl, argp, sizeof(ssp_st7789)))
        {
            return -EFAULT;
        }
    }

    switch(cmd)
    {
        case SSP_LCD_SET_BRIGHTNESS:
            // TODO 暂未实现
        break;

        case SSP_LCD_SET_BACKLIGHT_ON:
            gpio_set_value(LCD_BACKLIGHT_PIN, 1);
        break;

        case SSP_LCD_SET_BACKLIGHT_OFF:
            gpio_set_value(LCD_BACKLIGHT_PIN, 0);
        break;

        case SSP_LCD_SET_REG:
            // TODO 暂未实现
        break;

        case SSP_LCD_SET_DRAW_POINT:
            st7789_draw_point(st7789_ctl.point.x, st7789_ctl.point.y, st7789_ctl.point.color);
        break;

        case SSP_LCD_SET_DRAW_AREA:
            return st7789_draw_area(st7789_ctl.area.x1, st7789_ctl.area.y1,
                                    st7789_ctl.area.x2, st7789_ctl.area.y2,
                                    st7789_ctl.area.len, st7789_ctl.area.data);
        break;

        case SSP_LCD_TEST:
            gpio_set_value(LCD_BACKLIGHT_PIN, 1);
            msleep(1000);
            spi_enable();
            st7789_fill(0xFFFF);
            msleep(1000);
            st7789_fill(0x0000);
            msleep(1000);

            st7789_fill(0xF800);
            msleep(1000);
            st7789_fill(0x07E0);
            msleep(1000);
            st7789_fill(0x001F);
            msleep(1000);
            st7789_fill(0xFFE0);
            msleep(1000);
            st7789_fill(0x07FF);
            msleep(1000);
            st7789_fill(0xF81F);
            msleep(1000);

            st7789_fill(0xFFFF);
            spi_disable();
            msleep(1000);
            gpio_set_value(LCD_BACKLIGHT_PIN, 0);
        break;

        break;
        default:
        {
            printk("Kernel: No such ssp command %#x!\n", cmd);
            break;
        }
    }

    return 0;
}

static int ssp_lcd_open(struct inode * inode, struct file * file)
{
    return 0;
}

static int ssp_lcd_close(struct inode * inode, struct file * file)
{
    return 0;
}

static void ssp_set(void)
{
    unsigned int ret = 0;

    // 这部分寄存器的读写参考 IO_ADDRESS_VERIFY
    // 开启SPI0软复位
    ssp_readw(reg_crg_base_va + 0x01BC, ret);
    ret |= (1 << 15);
    ssp_writew(reg_crg_base_va + 0x01BC, ret);

    // 停止SPI0软复位
    ssp_readw(reg_crg_base_va + 0x01BC, ret);
    ret = ret & (~(1 << 15));
    ssp_writew(reg_crg_base_va + 0x01BC, ret);

    // 开启SPI0时钟门控寄存器
    ssp_readw(reg_crg_base_va + 0x01BC, ret);
    ret |= (1 << 12);
    ssp_writew(reg_crg_base_va + 0x01BC, ret);

    spi_disable();
    hi_ssp_set_frameform(SPI_FRAMEMODE, SPI_SPO, SPI_SPH, SPI_DATA_WIDTH);
    hi_ssp_set_serialclock(SPI_SCR, SPI_CPSDVSR);
    hi_ssp_alt_mode_set(1);
    hi_ssp_enable();
}

// 目前 LCD 复位由 SYS_RSTN_OUT 控制，无需硬件复位
static void lcd_reset(void)
{
    //-----------------------------------------------LED RESET----------------------------------------------//
    // ssp_set_reg(0x100C009C, 0x1000);
    // ssp_set_reg(0x120b9400, 0x01);
    // ssp_set_reg(0x120b9004, 0x0);
    // msleep(10);
    // ssp_set_reg(0x120b9004, 0x01);
    // msleep(120);
    //----------------------------------------ST7789S SleepOut Cmd-------------------------------------------//
    //msleep(120);

    // 按实际情况看要不要加软复位
    // ssp_write_cmd(0x01);
}

static void lcd_st7789_init_vertical(void)
{
    // 这部分寄存器的读写参考 IO_ADDRESS_VERIFY

    // SPI GPIO 复用设置
    // SPI0_SCLK
    // himm(0x112C003C, 0x00001c00);
    // himm(0x112C0074, 0x00001167);
    ssp_writew(reg_gpio_vivo_base_va + 0x003C, 0x00001c00);
    ssp_writew(reg_gpio_vivo_base_va + 0x0074, 0x00001167);
    // SPI0_SDO
    // himm(0x112C0038, 0x00001c00);
    // himm(0x112C006C, 0x00001127);
    ssp_writew(reg_gpio_vivo_base_va + 0x0038, 0x00001c00);
    ssp_writew(reg_gpio_vivo_base_va + 0x006C, 0x00001127);
    // SPI0_SDI
    // himm(0x112C0070, 0x00001127);
    ssp_writew(reg_gpio_vivo_base_va + 0x0070, 0x00001127);
    // SPI0_CSN
    // himm(0x112C0040, 0x00001000);
    // himm(0x112C0068, 0x00001127);
    ssp_writew(reg_gpio_vivo_base_va + 0x0040, 0x00001000);
    ssp_writew(reg_gpio_vivo_base_va + 0x0068, 0x00001127);
    
    // LCD 背光控制引脚复用设置
    // himm(0x100C0010, 0x00001120);
    ssp_writew(reg_gpio_ahb_base_va + 0x0010, 0x00001120);

    // ST7789 DC引脚复用设置
    // himm(0x112C0064, 0x00001120);
    ssp_writew(reg_gpio_vivo_base_va + 0x0064, 0x00001120);


    // 初始化引脚
    gpio_request(ST7789_DC_PIN, NULL);
    gpio_direction_output(ST7789_DC_PIN, 0);
    gpio_request(LCD_BACKLIGHT_PIN, NULL);
    gpio_direction_output(LCD_BACKLIGHT_PIN, 0);


    /*spi_9bit_setting*/
    ssp_set();

    lcd_reset();

    spi_enable();

    /* Sleep Out */
    ssp_write_cmd(0x11);
    /* wait for power stability */
    msleep(120);

    /* Memory Data Access Control */
    ssp_write_cmd(0x36);
    ssp_write_dat(0x00);

    /* RGB 5-6-5-bit  */
    ssp_write_cmd(0x3A);
    ssp_write_dat(0x55);

    /* Porch Setting */
    ssp_write_cmd(0xB2);
    ssp_write_dat(0x0C);
    ssp_write_dat(0x0C);
    ssp_write_dat(0x00);
    ssp_write_dat(0x33);
    ssp_write_dat(0x33);

    /*  Gate Control */
    ssp_write_cmd(0xB7);
    ssp_write_dat(0x72);

    /* VCOM Setting */
    ssp_write_cmd(0xBB);
    ssp_write_dat(0x3D);   //Vcom=1.625V

    /* LCM Control */
    ssp_write_cmd(0xC0);
    ssp_write_dat(0x2C);

    /* VDV and VRH Command Enable */
    ssp_write_cmd(0xC2);
    ssp_write_dat(0x01);

    /* VRH Set */
    ssp_write_cmd(0xC3);
    ssp_write_dat(0x19);

    /* VDV Set */
    ssp_write_cmd(0xC4);
    ssp_write_dat(0x20);

    /* Frame Rate Control in Normal Mode */
    ssp_write_cmd(0xC6);
    ssp_write_dat(0x0F);	//60MHZ

    /* Power Control 1 */
    ssp_write_cmd(0xD0);
    ssp_write_dat(0xA4);
    ssp_write_dat(0xA1);

    /* Positive Voltage Gamma Control */
    ssp_write_cmd(0xE0);
    ssp_write_dat(0xD0);
    ssp_write_dat(0x04);
    ssp_write_dat(0x0D);
    ssp_write_dat(0x11);
    ssp_write_dat(0x13);
    ssp_write_dat(0x2B);
    ssp_write_dat(0x3F);
    ssp_write_dat(0x54);
    ssp_write_dat(0x4C);
    ssp_write_dat(0x18);
    ssp_write_dat(0x0D);
    ssp_write_dat(0x0B);
    ssp_write_dat(0x1F);
    ssp_write_dat(0x23);

    /* Negative Voltage Gamma Control */
    ssp_write_cmd(0xE1);
    ssp_write_dat(0xD0);
    ssp_write_dat(0x04);
    ssp_write_dat(0x0C);
    ssp_write_dat(0x11);
    ssp_write_dat(0x13);
    ssp_write_dat(0x2C);
    ssp_write_dat(0x3F);
    ssp_write_dat(0x44);
    ssp_write_dat(0x51);
    ssp_write_dat(0x2F);
    ssp_write_dat(0x1F);
    ssp_write_dat(0x1F);
    ssp_write_dat(0x20);
    ssp_write_dat(0x23);

    /* Display Inversion On */
    ssp_write_cmd(0x21);

    /* Display On */
    ssp_write_cmd(0x29);


// 海思源码的初始化参数，暂不使用
#if 0
    ssp_write_cmd(0x11);
    msleep(120);

    //--------------------------------ST7789S Memory Data Access Control setting----------------------------------//
    //-----------------------Decides RGB/BGR or refresh Left to Right/Bottom to Top and so on---------------------- ---//
#if 0
    ssp_write_cmd(0x36);
    ssp_write_dat(0x00);

    //----------------------------------ST7789S Interface Pixel Format setting--------------------------------------//
    //----------------------------------18bit/Pixel and 262K of RGB interface----------------------------------------//
    ssp_write_cmd(0x3A);
    ssp_write_dat(0x66);
    //----------------------------------------ST7789S Display InversionOn Cmd-------------------------------------//
    ssp_write_cmd(0x21);

    //------------------------------------------ST7789S RAM Control Setting---------------------------------------//
    ssp_write_cmd(0xB0);
    ssp_write_dat(0x11);
    ssp_write_dat(0x04);

    //---------------------------------------ST7789S RAM Interface Control Setting----------------------------------//
    //----------------------------------------RGB Mode:Shift Register----------------------------------------------//
    ssp_write_cmd(0xB1);
    ssp_write_dat(0xC0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x14);

    //-----------------------------------------ST7789S Porch Setting---------------------------------------------//
    ssp_write_cmd(0xB2);
    ssp_write_dat(0x05);
    ssp_write_dat(0x05);
    ssp_write_dat(0x00);
    ssp_write_dat(0x33);
    ssp_write_dat(0x33);

    //-----------------------------------------ST7789S Gate Control Setting----------------------------------------//
    //--------------------------------------------VGL and VGH number--------------------------------------------//
    ssp_write_cmd(0xB7);
    ssp_write_dat(0x64);

    //-----------------------------------------ST7789S VCOM  setting---------------------------------------------//
    ssp_write_cmd(0xBB);
    ssp_write_dat(0x25);

    //---------------------------------------ST7789S LCM Control setting-------------------------------------------//
    ssp_write_cmd(0xC0);
    ssp_write_dat(0x2C);

    //-----------------------------------ST7789S VDV and VRH Command Enable setting--------------------------------//
    ssp_write_cmd(0xC2);
    ssp_write_dat(0x01);

    ssp_write_cmd(0xC3);
    //ssp_write_dat(0x13);
    ssp_write_dat(0x20);

    ssp_write_cmd(0xC4);
    //ssp_write_dat(0x20);
    ssp_write_dat(0x3);
    //----------------------------------ST7789S Normal mode Frame Rate setting--------------------------------------//
    ssp_write_cmd(0xC6);
    ssp_write_dat(0x11);

    //-------------------------------------ST7789S Power Control setting--------------------------------------------//
    ssp_write_cmd(0xD0);
    ssp_write_dat(0xA4);
    ssp_write_dat(0xA1);

    ssp_write_cmd(0xD6);
    ssp_write_dat(0xA1);

    //---------------------------------ST7789S Positive Gamma setting--------------------------------------//
#if 0
    ssp_write_cmd(0xE0);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x08);
    ssp_write_dat(0x11);
    ssp_write_dat(0x1a);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x26);
    ssp_write_dat(0x12);
    ssp_write_dat(0x21);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x11);

    //---------------------------------ST7789S Negative Gamma setting--------------------------------------//
    ssp_write_cmd(0xE1);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x09);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x27);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x17);
    ssp_write_dat(0x12);
    ssp_write_dat(0x11);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x31);
#endif
    //-----------------------------------ST7789S BrightNess Setting------------------------------------------//
    //ssp_write_cmd(0x51);
    //ssp_write_dat(0xFF);

    //-----------------------------------ST7789S Display ON Cmd--------------------------------------------//
    ssp_write_cmd(0x29);

    //-----------------------------------ST7789S Memory Write Cmd-----------------------------------------//
    //--------------------------------Transfer data from MCU to Frame memory---------------------------------//
    ssp_write_cmd(0x2c);

#else
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    ssp_write_cmd(0x36);
    ssp_write_dat(0x00);
    //--------------------------------ST7789S Frame rate setting----------------------------------//
    ssp_write_cmd(0xb2);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x33);
    ssp_write_dat(0x33);

    ssp_write_cmd(0xb7);
    ssp_write_dat(0x35);
    //---------------------------------ST7789S Power setting--------------------------------------//
    ssp_write_cmd(0xb8);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x2f);

    ssp_write_cmd(0xbb);
    ssp_write_dat(0x24);//vcom

    ssp_write_cmd(0xc0);
    ssp_write_dat(0x2C);

    ssp_write_cmd(0xc3);
    ssp_write_dat(0x20);

    ssp_write_cmd(0xc4);
    ssp_write_dat(0x3);

    ssp_write_cmd(0xc6);
    ssp_write_dat(0x11);

    ssp_write_cmd(0xd0);
    ssp_write_dat(0xa4);
    ssp_write_dat(0xa1);

    ssp_write_cmd(0xe8);
    ssp_write_dat(0x03);

    ssp_write_cmd(0xe9);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x12);
    ssp_write_dat(0x00);
    //--------------------------------ST7789S gamma setting---------------------------------------//
#if 0
    ssp_write_cmd(0xe0);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x00);
    ssp_write_dat(0x00);
    ssp_write_dat(0x08);
    ssp_write_dat(0x11);
    ssp_write_dat(0x1a);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x26);
    ssp_write_dat(0x12);
    ssp_write_dat(0x21);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x11);

    ssp_write_cmd(0xe1);
    ssp_write_dat(0xd0);
    ssp_write_dat(0x02);
    ssp_write_dat(0x09);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x0d);
    ssp_write_dat(0x27);
    ssp_write_dat(0x2b);
    ssp_write_dat(0x33);
    ssp_write_dat(0x42);
    ssp_write_dat(0x17);
    ssp_write_dat(0x12);
    ssp_write_dat(0x11);
    ssp_write_dat(0x2f);
    ssp_write_dat(0x31);
#endif
    ssp_write_cmd(0x21);

    //*********SET RGB Interfae***************
    ssp_write_cmd(0xB0);
    ssp_write_dat(0x11); //set RGB interface and DE mode.
    ssp_write_dat(0x04);
    ssp_write_dat(0x00);

    ssp_write_cmd(0xB1);
    //ssp_write_dat(0x40); //set DE mode ; SET Hs,Vs,DE,DOTCLK signal polarity
    ssp_write_dat(0xC0);
    ssp_write_dat(0x2);  //ori 2
    ssp_write_dat(0x14); //ori 14
    ssp_write_cmd(0x3a);
    ssp_write_dat(0x55); //18 RGB ,55-16BIT RGB
    // ssp_write_cmd(0x11); //Exit Sleep
    ssp_write_cmd(0x29); //display on
    ssp_write_cmd(0x2c);

#endif
#endif // 海思源码的初始化参数，暂不使用

    spi_disable();
    return;
}

// static void lcd_st7789_backlight_on(void)
// {
//     gpio_set_value(LCD_BACKLIGHT_PIN, 1);
// }

// static void lcd_st7789_backlight_off(void)
// {
//     gpio_set_value(LCD_BACKLIGHT_PIN, 0);
// }


#ifdef __HuaweiLite__
static const struct file_operations_vfs ssp_lcd_fops =
{
    ssp_lcd_open,   /* open */
    ssp_lcd_close,  /* close */
    0,              /* read */
    0,              /* write */
    0,              /* seek */
    ssp_lcd_ioctl   /* ioctl */
#ifndef CONFIG_DISABLE_POLL
    , 0             /* poll */
#endif
};

int lcd_dev_register(void)
{
    return register_driver("/dev/ssp_lcd", &ssp_lcd_fops, 0666, 0);
}

int __init hi_ssp_lcd_st7789_init(void)
{
    int ret;

    ret = lcd_dev_register();
    if(0 != ret)
    {
        printk("Kernel: lcd_dev_register failed!\n");
        return -1;
    }

    lcd_st7789_init_vertical();

    lcd_st7789_backlighton();

    printk("Kernel: ssp_lcd initial ok!\n");

    return 0;
}

void __exit hi_ssp_lcd_st7789_exit(void)
{
    hi_ssp_disable();
}

#else

static struct file_operations ssp_lcd_fops = {
    .owner      = THIS_MODULE,
    .unlocked_ioctl = ssp_lcd_ioctl,
    .open       = ssp_lcd_open,
    .release    = ssp_lcd_close
};


static struct miscdevice ssp_lcd_dev = {
   .minor       = MISC_DYNAMIC_MINOR,
   .name        = "st7789_lcd",
   .fops        = &ssp_lcd_fops,
};

static int __init hi_ssp_lcd_st7789_init(void)
{
    int ret;

    reg_ssp_base_va = ioremap_nocache((unsigned long)SSP_BASE, (unsigned long)SSP_SIZE);
    if (!reg_ssp_base_va)
    {
        printk("Kernel: ioremap ssp base failed!\n");
        return -ENOMEM;
    }

    reg_crg_base_va = ioremap_nocache((unsigned long)CRG_BASE, (unsigned long)SSP_SIZE);
    if (!reg_crg_base_va)
    {
        printk("Kernel: ioremap crg base failed!\n");
        return -ENOMEM;
    }

    reg_gpio_ahb_base_va = ioremap_nocache((unsigned long)GPIO_AHB_BASE, (unsigned long)SSP_SIZE);
    if (!reg_gpio_ahb_base_va)
    {
        printk("Kernel: ioremap ahb base failed!\n");
        return -ENOMEM;
    }

    reg_gpio_vivo_base_va = ioremap_nocache((unsigned long)GPIO_VIVO_BASE, (unsigned long)SSP_SIZE);
    if (!reg_gpio_vivo_base_va)
    {
        printk("Kernel: ioremap vivo base failed!\n");
        return -ENOMEM;
    }

    ret = misc_register(&ssp_lcd_dev);
    if(0 != ret)
    {
        printk("Kernel: register ssp_0 device failed!\n");
        return -1;
    }

    lcd_st7789_init_vertical();
    st7789_fill(0xFFFF);

    printk("Kernel: ssp_lcd initial ok!\n");
    return 0;
}

static void __exit hi_ssp_lcd_st7789_exit(void)
{
    hi_ssp_disable();
    gpio_set_value(LCD_BACKLIGHT_PIN, 0);
    gpio_free(ST7789_DC_PIN);
    gpio_free(LCD_BACKLIGHT_PIN);
    iounmap((void*)reg_ssp_base_va);
    iounmap((void*)reg_crg_base_va);
    iounmap((void*)reg_gpio_ahb_base_va);
    iounmap((void*)reg_gpio_vivo_base_va);
    misc_deregister(&ssp_lcd_dev);
}
#endif


module_init(hi_ssp_lcd_st7789_init);
module_exit(hi_ssp_lcd_st7789_exit);

MODULE_LICENSE("GPL");

