#ifndef __MPU6050_H
#define __MPU6050_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>
#include "gyroflow_log.h"

// MPU6050 内部寄存器地址
#define MPU6050_REG_SELF_TEST_X         0x0D
#define MPU6050_REG_SELF_TEST_Y         0x0E
#define MPU6050_REG_SELF_TEST_Z         0x0F
#define MPU6050_REG_SELF_TEST_A         0x10
#define MPU6050_REG_SMPLRT_DIV          0x19 //陀螺仪采样率，典型值：0x07(125Hz)
#define MPU6050_REG_CONFIG              0x1A //低通滤波频率，典型值：0x06(5Hz)
#define MPU6050_REG_GYRO_CONFIG         0x1B //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define MPU6050_REG_ACCEL_CONFIG        0x1C //加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define MPU6050_REG_MOT_THR             0x1F
#define MPU6050_REG_FIFO_EN             0x23
#define MPU6050_REG_I2C_MST_CTRL        0x24
#define MPU6050_REG_I2C_SLV0_ADDR       0x25
#define MPU6050_REG_I2C_SLV0_REG        0x26
#define MPU6050_REG_I2C_SLV0_CTRL       0x27
#define MPU6050_REG_I2C_SLV1_ADDR       0x28
#define MPU6050_REG_I2C_SLV1_REG        0x29
#define MPU6050_REG_I2C_SLV1_CTRL       0x2A
#define MPU6050_REG_I2C_SLV2_ADDR       0x2B
#define MPU6050_REG_I2C_SLV2_REG        0x2C
#define MPU6050_REG_I2C_SLV2_CTRL       0x2D
#define MPU6050_REG_I2C_SLV3_ADDR       0x2E
#define MPU6050_REG_I2C_SLV3_REG        0x2F
#define MPU6050_REG_I2C_SLV3_CTRL       0x30
#define MPU6050_REG_I2C_SLV4_ADDR       0x31
#define MPU6050_REG_I2C_SLV4_REG        0x32
#define MPU6050_REG_I2C_SLV4_DO         0x33
#define MPU6050_REG_I2C_SLV4_CTRL       0x34
#define MPU6050_REG_I2C_SLV4_DI         0x35
#define MPU6050_REG_I2C_MST_STATUS      0x36
#define MPU6050_REG_INT_PIN_CFG         0x37
#define MPU6050_REG_INT_ENABLE          0x38
#define MPU6050_REG_INT_STATUS          0x3A
#define MPU6050_REG_ACCEL_XOUT_H        0x3B //加速度计
#define MPU6050_REG_ACCEL_XOUT_L        0x3C
#define MPU6050_REG_ACCEL_YOUT_H        0x3D
#define MPU6050_REG_ACCEL_YOUT_L        0x3E
#define MPU6050_REG_ACCEL_ZOUT_H        0x3F
#define MPU6050_REG_ACCEL_ZOUT_L        0x40
#define MPU6050_REG_TEMP_OUT_H          0x41 //温度
#define MPU6050_REG_TEMP_OUT_L          0x42
#define MPU6050_REG_GYRO_XOUT_H         0x43 //陀螺仪
#define MPU6050_REG_GYRO_XOUT_L         0x44
#define MPU6050_REG_GYRO_YOUT_H         0x45
#define MPU6050_REG_GYRO_YOUT_L         0x46
#define MPU6050_REG_GYRO_ZOUT_H         0x47
#define MPU6050_REG_GYRO_ZOUT_L         0x48
#define MPU6050_REG_EXT_SENS_DATA_00    0x49
#define MPU6050_REG_EXT_SENS_DATA_01    0x4A
#define MPU6050_REG_EXT_SENS_DATA_02    0x4B
#define MPU6050_REG_EXT_SENS_DATA_03    0x4C
#define MPU6050_REG_EXT_SENS_DATA_04    0x4D
#define MPU6050_REG_EXT_SENS_DATA_05    0x4E
#define MPU6050_REG_EXT_SENS_DATA_06    0x4F
#define MPU6050_REG_EXT_SENS_DATA_07    0x50
#define MPU6050_REG_EXT_SENS_DATA_08    0x51
#define MPU6050_REG_EXT_SENS_DATA_09    0x52
#define MPU6050_REG_EXT_SENS_DATA_10    0x53
#define MPU6050_REG_EXT_SENS_DATA_11    0x54
#define MPU6050_REG_EXT_SENS_DATA_12    0x55
#define MPU6050_REG_EXT_SENS_DATA_13    0x56
#define MPU6050_REG_EXT_SENS_DATA_14    0x57
#define MPU6050_REG_EXT_SENS_DATA_15    0x58
#define MPU6050_REG_EXT_SENS_DATA_16    0x59
#define MPU6050_REG_EXT_SENS_DATA_17    0x5A
#define MPU6050_REG_EXT_SENS_DATA_18    0x5B
#define MPU6050_REG_EXT_SENS_DATA_19    0x5C
#define MPU6050_REG_EXT_SENS_DATA_20    0x5D
#define MPU6050_REG_EXT_SENS_DATA_21    0x5E
#define MPU6050_REG_EXT_SENS_DATA_22    0x5F
#define MPU6050_REG_EXT_SENS_DATA_23    0x60
#define MPU6050_REG_I2C_SLV0_DO         0x63
#define MPU6050_REG_I2C_SLV1_DO         0x64
#define MPU6050_REG_I2C_SLV2_DO         0x65
#define MPU6050_REG_I2C_SLV3_DO         0x66
#define MPU6050_REG_I2C_MST_DELAAY_CTRL 0x67
#define MPU6050_REG_SIGNAL_PATH_RESET   0x68
#define MPU6050_REG_MOT_DETECE_CTRL     0x69
#define MPU6050_REG_USER_CTRL           0x6A
#define MPU6050_REG_PWR_MGMT_1          0x6B //电源管理，典型值：0x00(正常启用)，设置为0x01使用陀螺仪X轴做参考比默认内部8MHz系统时钟源精确点
#define MPU6050_REG_PWR_MGMT_2          0x6C
#define MPU6050_REG_FIFO_COUNTH         0x72
#define MPU6050_REG_FIFO_COUNTL         0x73
#define MPU6050_REG_FIFO_R_W            0x74
#define MPU6050_REG_WHO_AM_I            0x75 //IIC地址寄存器(默认数值0x68，只读)


// AD0引脚接低电平地址是0x68（含读写位0xD0）
// AD0引脚接高电平地址是0x69（含读写位0xD2）
#define MPU6050_DEV_ADDR 0x68
// #define MPU6050_DEV_ADDR 0x69

gyro_log_err_code mpu6050_init(void *args);
gyro_log_err_code mpu6050_read(void *args, gyro_data *data, int *num);
gyro_log_err_code mpu6050_destroy(void *args);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
