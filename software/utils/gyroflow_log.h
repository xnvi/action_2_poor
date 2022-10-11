#ifndef _GYROFLOW_LOG_H
#define _GYROFLOW_LOG_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>

typedef enum {
    GYRO_LOG_OK = 0,
    GYRO_LOG_PARAM_NULL_PTR,
    GYRO_LOG_PARAM_OVERFLOW,
    GYRO_LOG_PARAM_INVALID,
    GYRO_LOG_MALLOC_FAILED,
    GYRO_LOG_INIT_ERR,
    GYRO_LOG_FILE_NOT_OPEN,
    GYRO_LOG_WRITE_ERR,

    // 传感器相关回调函数使用
    GYRO_LOG_SENSOR_OK,
    GYRO_LOG_SENSOR_ACC_ERR,
    GYRO_LOG_SENSOR_GYRO_ERR,
    GYRO_LOG_SENSOR_MAGNET_ERR,
    GYRO_LOG_SENSOR_INIT_ERR,
    GYRO_LOG_SENSOR_PARAM_ERR,
    GYRO_LOG_SENSOR_NOT_CONNECT,
    GYRO_LOG_SENSOR_POWER_DOWN,
    GYRO_LOG_SENSOR_NO_DATA,
    GYRO_LOG_SENSOR_WAIT,

    GYRO_LOG_OTHER_ERR = 100,
} gyro_log_err_code;

typedef union {
    struct {
        int16_t ax;
        int16_t ay;
        int16_t az;
        int16_t gx;
        int16_t gy;
        int16_t gz;
        int16_t mx;
        int16_t my;
        int16_t mz;
    } ch;
    int16_t full[9];
} gyro_data;

// 初始化传感器
typedef gyro_log_err_code (*func_sensor_init)(void *args);
// 读取数据，其中num做入参表示data数组的长度，做返回值表示实际读取到的gyro_data数据量
typedef gyro_log_err_code (*func_sensor_read)(void *args, gyro_data *data, int *num);
// 销毁传感器
typedef gyro_log_err_code (*func_sensor_destroy)(void *args);

typedef struct {
    uint8_t use_accelerometer;
    uint8_t use_gyroscope;
    uint8_t use_magnetometer;
    uint8_t reserve;
    uint32_t freq; // 这个频率即传感器的采样率
    double ascale; // Multiplying ascale by the raw ax/ay/az values should give the acceleration in g.
    double gscale; // Multiplying gscale by the raw gx/gy/gz values should give the angular rate in rad/s.
    double mscale; // Multiplying mscale by mx/my/mz gives the measurements in gauss (Note that 1 tesla = 10000 gauss).
    char *log_path; // 必须以"/"结尾
    void *user_args; // 回调函数入参
    char *sensor_name;
    func_sensor_init sensor_init;
    func_sensor_read sensor_read;
    func_sensor_destroy sensor_destroy;
} gyro_log_init_param;

gyro_log_err_code gyro_log_init(gyro_log_init_param *param);
gyro_log_err_code gyro_log_start(char *filename); // 对应的视频文件名，仅文件名，不含路径
gyro_log_err_code gyro_log_stop();
gyro_log_err_code gyro_log_pause();
gyro_log_err_code gyro_log_resume();
gyro_log_err_code gyro_log_destroy();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
