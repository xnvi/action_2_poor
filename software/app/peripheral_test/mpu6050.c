// 仅用于测试MPU6050能否正常工作，不是MPU6050的正式驱动程序

#include "mpu6050.h"
#include "i2c.h"
#include <unistd.h>

int mpu6050_init(void *args)
{
    uint8_t buf[4];
    I2CDevice *i2c_dev = (I2CDevice *)args;

    i2c_ioctl_read(i2c_dev, MPU6050_REG_WHO_AM_I, buf, 1);
    if (buf[0] != MPU6050_DEV_ADDR) {
        return 1;
    }

    // 复位
    buf[0] = 0x80;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_PWR_MGMT_1, buf, 1);
    usleep(10 * 1000);

    // 电源
    buf[0] = 0x01;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_PWR_MGMT_1, buf, 1);

    // 陀螺仪采样率
    buf[0] = 0x01;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_SMPLRT_DIV, buf, 1);

	// 低通滤波
    buf[0] = 2; // acc 94 Hz, gyro 98 Hz
    i2c_ioctl_write(i2c_dev, MPU6050_REG_CONFIG, buf, 1);

    // 加速度计量程
    buf[0] = 1 << 3; // +/- 4 g
    i2c_ioctl_write(i2c_dev, MPU6050_REG_ACCEL_CONFIG, buf, 1);

    // 陀螺仪量程
    buf[0] = 2 << 3; // +/- 1000 deg/s
    i2c_ioctl_write(i2c_dev, MPU6050_REG_GYRO_CONFIG, buf, 1);

    return 0;
}

int mpu6050_read(void *args, mpu_data *data)
{
    I2CDevice *i2c_dev = (I2CDevice *)args;
    int32_t i;
    uint16_t tmp;

    i2c_ioctl_read(i2c_dev, MPU6050_REG_ACCEL_XOUT_H, (void *)&data->ch.ax, sizeof(data->ch.ax));
    i2c_ioctl_read(i2c_dev, MPU6050_REG_ACCEL_YOUT_H, (void *)&data->ch.ay, sizeof(data->ch.ay));
    i2c_ioctl_read(i2c_dev, MPU6050_REG_ACCEL_ZOUT_H, (void *)&data->ch.az, sizeof(data->ch.az));
    i2c_ioctl_read(i2c_dev, MPU6050_REG_GYRO_XOUT_H, (void *)&data->ch.gx, sizeof(data->ch.gx));
    i2c_ioctl_read(i2c_dev, MPU6050_REG_GYRO_YOUT_H, (void *)&data->ch.gy, sizeof(data->ch.gy));
    i2c_ioctl_read(i2c_dev, MPU6050_REG_GYRO_ZOUT_H, (void *)&data->ch.gz, sizeof(data->ch.gz));

    for (i = 0; i < 6; i++) {
        tmp = ((data->all[i] & 0x00FF) << 8) | ((data->all[i] & 0xFF00) >> 8);
        data->all[i] = tmp;
    }

    return 0;
}

int mpu6050_destroy(void *args)
{
    // do nothing
    return 0;
}
