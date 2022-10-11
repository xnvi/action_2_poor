#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <unistd.h>
#include "gyroflow_log.h"

/*
传感器坐标系：
相机竖直摆放在桌面上，镜头向前（即正常手持相机拍摄的方式）
向上为正Y轴
向右为正X轴
向前为正Z轴
如果芯片安装位置与此不符，需手动调整

gyroflow官网给出的日志样例
GYROFLOW IMU LOG
version,1.1
id,custom_logger_name
orientation,YxZ
note,development_test
fwversion,FIRMWARE_0.1.0
timestamp,1644159993
vendor,potatocam
videofilename,videofilename.mp4
lensprofile,potatocam_mark1_prime_7_5mm_4k
tscale,0.001
gscale,0.00122173047
ascale,0.00048828125
mscale,0.00059875488
t,gx,gy,gz,ax,ay,az,mx,my,mz
0,39,86,183,-1137,-15689,-2986,123,345,546
1,56,100,202,-1179,-15694,-2887,124,344,560

更多内容请参考gyroflow官网文档
https://docs.gyroflow.xyz/
*/

typedef struct fraction{
    int64_t num; // Numerator
    int64_t den; // Denominator
} fraction;

typedef enum {
    GYRO_LOG_STOP = 0,
    GYRO_LOG_RUNNING,
    GYRO_LOG_PAUSE,
} gyro_log_status;

typedef struct {
    uint8_t in_use;
    uint8_t is_run;
    uint8_t exit;
    pthread_t thread;
} thread_context;

typedef struct {
    uint32_t is_init;
    gyro_log_status status;
    pthread_mutex_t lock;
    FILE *fpw;
    int64_t data_frame_count;
    int64_t base_ms;
    struct timeval time_base;
    thread_context th_ctx;
    gyro_log_init_param init_param;
} gyro_log_param;

static gyro_log_param param_private;

static void *_gyro_log_loop()
{
    char abuf[128] = "";
    char gbuf[128] = "";
    char mbuf[128] = "";
    gyro_data gdata[64] = {0};
    int32_t data_num = 0;
    int32_t i = 0;
    int32_t freq = 0;
    int64_t timestamp = 0;

    prctl(PR_SET_NAME, "pth_gyroflow");

    // 这样做目的是尽可能清空传感器fifo
    while (1) {
        data_num = 64;
        param_private.init_param.sensor_read(param_private.init_param.user_args, gdata, &data_num);
        if (data_num < 10) {
            break;
        }
    }

    freq = param_private.init_param.freq;
    param_private.th_ctx.is_run = 1;
    while(!param_private.th_ctx.exit) {
        // 目前内核tick频率为100Hz，因此这里sleep实际值在10ms至20ms之间波动
        // 所以使用的传感器必有有缓存，为了稳妥一些，传感器最好能缓存30ms以上的数据，等到sleep后一次全部读出
        usleep(1000);

        // 为了避免传感器内缓存溢出，这里不停地读取
        data_num = 64;
        param_private.init_param.sensor_read(param_private.init_param.user_args, gdata, &data_num);

        if (param_private.status != GYRO_LOG_RUNNING) {
            continue;
        }

        for (i = 0; i < data_num; i++) {
            if (param_private.init_param.use_accelerometer) {
                sprintf(abuf, ",%d,%d,%d", gdata[i].ch.ax, gdata[i].ch.ay, gdata[i].ch.az);
            }
            if (param_private.init_param.use_gyroscope) {
                sprintf(gbuf, ",%d,%d,%d", gdata[i].ch.gx, gdata[i].ch.gy, gdata[i].ch.gz);
            }
            if (param_private.init_param.use_magnetometer) {
                sprintf(mbuf, ",%d,%d,%d", gdata[i].ch.mx, gdata[i].ch.my, gdata[i].ch.mz);
            }

            // 绝对时间
            // timestamp = param_private.data_frame_count * 1000 / freq + param_private.base_ms;
            // 相对时间
            timestamp = param_private.data_frame_count * 1000 / freq;

            pthread_mutex_lock(&param_private.lock);
            if (param_private.fpw != NULL) {
                fprintf(param_private.fpw, "%lld%s%s%s\r\n", timestamp, abuf, gbuf, mbuf);
            }
            pthread_mutex_unlock(&param_private.lock);
            param_private.data_frame_count += 1;
        }
    }
    param_private.th_ctx.is_run = 0;

    return NULL;
}

gyro_log_err_code gyro_log_init(gyro_log_init_param *param)
{
    int32_t err = 0;
    pthread_attr_t attr;

    memset(&param_private, 0, sizeof(gyro_log_param));
    param_private.init_param = *param;
    param_private.init_param.sensor_init(param_private.init_param.user_args);

    param_private.th_ctx.is_run = 0;
    param_private.th_ctx.exit = 0;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 16 * 1024); // 节约内存
    err = pthread_create(&param_private.th_ctx.thread, &attr, &_gyro_log_loop, NULL);
    if (err != 0) {
        gyro_log_destroy();
        return GYRO_LOG_INIT_ERR;
    }
    pthread_attr_destroy(&attr);
    param_private.th_ctx.in_use = 1;

    pthread_mutex_init(&param_private.lock, NULL);
    param_private.is_init = 1;
    return GYRO_LOG_OK;
}

gyro_log_err_code gyro_log_start(char *filename)
{
    char full_name[128] = "";
    char log_constants[128] = "";

    sprintf(full_name, "%s%s%s", param_private.init_param.log_path, filename, ".gcsv"); // 懒得对文件名做处理，直接加后缀名
    param_private.fpw = fopen(full_name, "w");
    if (param_private.fpw == NULL) {
        return GYRO_LOG_FILE_NOT_OPEN;
    }

    param_private.data_frame_count = 0;
    gettimeofday(&param_private.time_base, NULL);
    param_private.base_ms = param_private.time_base.tv_sec * 1000 + param_private.time_base.tv_usec / 1000;

    fputs("GYROFLOW IMU LOG\r\n", param_private.fpw);
    fputs("version,1.1\r\n", param_private.fpw);
    fputs("id,Gyroflow_log_create_by_Action_2_Poor\r\n", param_private.fpw);
    fputs("orientation,YxZ\r\n", param_private.fpw);
    if (param_private.init_param.sensor_name) {
        fprintf(param_private.fpw, "note,serner_%s\r\n", param_private.init_param.sensor_name);
    }
    fputs("fwversion,Action_2_Poor_sw_v0.1\r\n", param_private.fpw);
    fprintf(param_private.fpw, "timestamp,%ld\r\n", param_private.time_base.tv_sec);
    fputs("vendor,Action_2_Poor\r\n", param_private.fpw);
    fprintf(param_private.fpw, "videofilename,%s\r\n", filename);
    fputs("lensprofile,OV5640\r\n", param_private.fpw);

    // 定死了就以毫秒为单位，用1000Hz采样率，统一时间
    fprintf(param_private.fpw, "tscale,0.001\r\n");

    if (param_private.init_param.use_accelerometer) {
        fprintf(param_private.fpw, "ascale,%.12f\r\n", param_private.init_param.ascale);
    }
    if (param_private.init_param.use_gyroscope) {
        fprintf(param_private.fpw, "gscale,%.12f\r\n", param_private.init_param.gscale);
    }
    if (param_private.init_param.use_magnetometer) {
        fprintf(param_private.fpw, "mscale,%.12f\r\n", param_private.init_param.mscale);
    }

    // 日志格式 t,gx,gy,gz,ax,ay,az,mx,my,mz
    strcat(log_constants, "t");
    if (param_private.init_param.use_accelerometer) {
        strcat(log_constants, ",ax,ay,az");
    }
    if (param_private.init_param.use_gyroscope) {
        strcat(log_constants, ",gx,gy,gz");
    }
    if (param_private.init_param.use_magnetometer) {
        strcat(log_constants, ",mx,my,mz");
    }
    strcat(log_constants, "\r\n");
    fputs(log_constants, param_private.fpw);

    param_private.status = GYRO_LOG_RUNNING;
    return GYRO_LOG_OK;
}

gyro_log_err_code gyro_log_stop()
{
    param_private.status = GYRO_LOG_STOP;
    pthread_mutex_lock(&param_private.lock);
    fclose(param_private.fpw);
    param_private.fpw = NULL;
    pthread_mutex_unlock(&param_private.lock);

    return GYRO_LOG_OK;
}

gyro_log_err_code gyro_log_pause()
{
    param_private.status = GYRO_LOG_PAUSE;
    return GYRO_LOG_OK;
}

gyro_log_err_code gyro_log_resume()
{
    param_private.status = GYRO_LOG_RUNNING;
    return GYRO_LOG_OK;
}

gyro_log_err_code gyro_log_destroy()
{
    if (param_private.th_ctx.in_use) {
        param_private.th_ctx.exit = 1;
        while(param_private.th_ctx.is_run) {usleep(1 * 1000);}
        param_private.th_ctx.in_use = 0;
    }
    if (param_private.is_init) {
        pthread_mutex_destroy(&param_private.lock);
    }
    param_private.init_param.sensor_destroy(param_private.init_param.user_args);
    memset(&param_private, 0, sizeof(gyro_log_param));
    return GYRO_LOG_OK;
}
