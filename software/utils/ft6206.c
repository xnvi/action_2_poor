#include "ft6206.h"
#include "i2c.h"
#include <stdio.h>

int8_t ft6206_init(void *args)
{
    uint8_t buf[4];
    I2CDevice *i2c_dev = (I2CDevice *)args;

    i2c_ioctl_read(i2c_dev, 0xA8, buf, 1);
    if (buf[0] != 0x11) {
        printf("init ft6206 failed! %#x\n", buf[0]);
        return 1;
    }

	// TODO 配置工作模式
	// 默认参数满足使用需求，这一步略
    // 使用轮询方式，不配置中断

    return 0;
}

void ft6206_exit(void *args)
{
    // do nothing
	return;
}

int8_t ft6206_read(void *args, ft6206_touch_info *info)
{
    I2CDevice *i2c_dev = (I2CDevice *)args;
	uint8_t i = 0;
	uint8_t base = 0;
	uint8_t read_buf[16];
    i2c_ioctl_read(i2c_dev, 0x00, read_buf, 15);

	info->gesture = read_buf[1];
	info->num = read_buf[2] & 0x0F;
	for (i = 0; i < info->num; i++) {
		base = i * 6 + 3;
		info->points[i].event     = (read_buf[base + 0] & 0xC0) >> 6;
		info->points[i].x         = (read_buf[base + 0] & 0x0F) << 8;
		info->points[i].x        += read_buf[base + 1];
		info->points[i].touch_id  = (read_buf[base + 2] & 0xF0) >> 4;
		info->points[i].y         = (read_buf[base + 2] & 0x0F) << 8;
		info->points[i].y        += read_buf[base + 3];
		info->points[i].weight    = read_buf[base + 4];
		info->points[i].area      = (read_buf[base + 5] & 0xF0) >> 4;
	}

	return 0;
}
