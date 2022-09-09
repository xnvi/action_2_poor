// 仅用于测试能否正常读写OV5640的寄存器

#ifndef __OV5640_H
#define __OV5640_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>
#include "i2c.h"

#define OV5640_DEV_ADDR 0x3c
// #define OV5640_DEV_ADDR 0x78

int32_t OV5640_Init(I2CDevice *i2c_dev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
