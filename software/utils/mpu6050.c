#include "mpu6050.h"
#include "i2c.h"
#include <unistd.h>

// gyroflow 官网建议采样率500Hz，低通滤波100Hz或以下，陀螺仪量程 +/- 1000 deg/s，加速度计量程 +/- 4 g
// 海思的i2c有一个内置64x8bit的发送fifo和64x8bit的接收fifo
// 从目前的现象来看，两次通信之间的数据量如果小于64B，那么不占用cpu（推测驱动内部使用select方式或DMA方式）
// 如果大于64B则会阻塞等待，此时会占用大量CPU，直到FIFO为空再收发后面的数据（推测内部使用的是自旋锁）
// 受限于海思I2C的特性，为了降低CPU使用率，只能降低通信的数据量，因此需要将采样率降低至200Hz甚至更低
// 另一种解决办法是重新编译内核，修改时钟树，将i2c速率提高到400kHz，目前我不打算重新编译内核
gyro_log_err_code mpu6050_init(void *args)
{
    uint8_t buf[4];
    I2CDevice *i2c_dev = (I2CDevice *)args;

    i2c_ioctl_read(i2c_dev, MPU6050_REG_WHO_AM_I, buf, 1);
    if (buf[0] != MPU6050_DEV_ADDR) {
        return GYRO_LOG_SENSOR_NOT_CONNECT;
    }
    // printf("i2c buf %d %d %d %d \n", buf[0], buf[1], buf[2], buf[3]);

    // 复位
    buf[0] = 0x80;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_PWR_MGMT_1, buf, 1);

    usleep(10 * 1000);


    // 电源管理，启用并采用陀螺仪X轴做参考比默认内部8MHz系统时钟源精确点
    buf[0] = 0x01;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_PWR_MGMT_1, buf, 1);

    // 陀螺仪采样率，频率=1000/(寄存器值+1)
    buf[0] = 0x04;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_SMPLRT_DIV, buf, 1);

	// 低通滤波
    // buf[0] = 0; // acc 260 Hz, gyro 256 Hz
    // buf[0] = 1; // acc 184 Hz, gyro 188 Hz
    buf[0] = 2; // acc 94 Hz, gyro 98 Hz
    // buf[0] = 3; // acc 44 Hz, gyro 42 Hz
    // buf[0] = 4; // acc 21 Hz, gyro 20 Hz
    // buf[0] = 5; // acc 10 Hz, gyro 10 Hz
    // buf[0] = 6; // acc 5 Hz, gyro 5 Hz
    i2c_ioctl_write(i2c_dev, MPU6050_REG_CONFIG, buf, 1);

    // 加速度计量程
    // buf[0] = 0; // +/- 2 g
    buf[0] = 1; // +/- 4 g
    // buf[0] = 2; // +/- 8 g
    // buf[0] = 3; // +/- 16 g
    i2c_ioctl_write(i2c_dev, MPU6050_REG_ACCEL_CONFIG, buf, 1);

    // 陀螺仪量程
    // buf[0] = 0; // +/- 250 deg/s
    // buf[0] = 1; // +/- 500 deg/s
    buf[0] = 2; // +/- 1000 deg/s
    // buf[0] = 3; // +/- 2000 deg/s
    i2c_ioctl_write(i2c_dev, MPU6050_REG_GYRO_CONFIG, buf, 1);

    // 设置需要写入FIFO的数据
    buf[0] = 0x78;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_FIFO_EN, buf, 1);
    // 开启FIFO
    buf[0] = 0x40;
    i2c_ioctl_write(i2c_dev, MPU6050_REG_USER_CTRL, buf, 1);

    return GYRO_LOG_SENSOR_OK;
}

gyro_log_err_code mpu6050_read(void *args, gyro_data *data, int *num)
{
    I2CDevice *i2c_dev = (I2CDevice *)args;
    int32_t i = 0, j = 0;
    uint16_t tmp;
    uint16_t pack;
    uint8_t buf[16];

    // 读取FIFO内的字节数
    i2c_ioctl_read(i2c_dev, MPU6050_REG_FIFO_COUNTH, (void *)buf, 2);
    tmp = (buf[0] << 8) | buf[1];

    // 读取FIFO数据
    pack = tmp / 12;
    *num = pack > *num ? *num : pack;
    for (i = 0; i < pack; i++) {
        i2c_ioctl_read(i2c_dev, MPU6050_REG_FIFO_R_W, (void *)&buf, 12);
        for (j = 0; j < 6; j++) {
            data[i].full[j] = (buf[j * 2] << 8) | buf[j * 2 + 1];
        }
    }

    return GYRO_LOG_SENSOR_OK;
}

gyro_log_err_code mpu6050_destroy(void *args)
{
    // do nothing
    return GYRO_LOG_SENSOR_OK;
}
