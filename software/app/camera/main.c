#include <signal.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "himm.h"
#include "i2c.h"
#include "ft6206.h"
#include "ssp_st7789.h"
#include "lvgl.h"
#include "cam_gui.h"
#include "key.h"
#include "mpu6050.h"
#include "gyroflow_log.h"
#include "file_opt.h"
#include "hisi_adc.h"
#include "cJSON.h"
#include "himpp.h"
#include "camera_himpp_config.h"
#include "camera_base_config.h"
#include "media.h"

#define LCD_SCREEN_WIDTH  (240)
#define LCD_SCREEN_HEIGHT (240)

#define JSON_CONFIG_FILE_NAME "./config.json"

#define JSON_ITEM_VFORMAT  "format"
#define JSON_ITEM_VSIZE    "size"
#define JSON_ITEM_VQUALITY "quality"

char *vformat_list[] = {
    "H.264",
    "H.265",
};
char *vsize_list[] = {
    "720P",
    "1080P",
};
char *vquality_list[] = {
    "low",
    "mid",
    "high",
};

typedef struct {
    pthread_t pid;
    // int stack_size;
    // char name[16];
    char running;
    char exit;
} pthread_manger_t;

static pthread_manger_t sg_pth_lvgl = {0};

static lv_color_t sg_lcd_buf1[LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT / 2];
static lv_color_t sg_lcd_buf2[LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT / 2];

int i2c_fd2 = 0;
int lcd_fd = 0;
I2CDevice dev_ft6206;
I2CDevice dev_mpu6050;
gyro_log_init_param gyro_cfg;

static uint32_t process_exit = 0;
void main_exit(int sig)
{
    printf("vIpcExit by signal:%d\n", sig);
    process_exit = 1;
}

static void set_hi_reg(void)
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


    // GPIO 设置
    himm(0x100C0008, 0x00001D30); // GPIO
    // himm(0x100C0008, 0x00001D31); // UPDATE_MODE


    // PWM 设置
    // TODO


    // ADC 引脚复用设置
    himm(0x120C0000, 0x00000001); // ch0
    // himm(0x120C0004, 0x00000001); // ch1


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

    return;
}

float get_cpu_temp(void)
{
#ifdef ENABLE_SIMULATE_SYSTEM_CALL
    return 56.78
#else
    float temp = 0.0;
    uint32_t reg = 0;
    reg = himd(0x120280BC);
    reg = reg & 0x00000FFF;
    temp = ((reg-117)/798.0)*165-40;
    return temp;
#endif
}

float get_battery_voltage(void)
{
#ifdef ENABLE_SIMULATE_SYSTEM_CALL
    static float voltage = 4.20;
    voltage -= 0.05;
    if (voltage < 3.3) {
        voltage = 4.20;
    }
    return voltage;
#else
    uint32_t adc = 0;
    float voltage = 0.0;
    adc = hiadc_get_ch0();
    voltage = 3.3 / (10.0 / (10.0 + 4.7)) * (float)adc / 1024.0;
    return voltage;
#endif
}

int save_json_cfg(video_format_e format, video_size_e size, video_quality_e quality)
{
    char json_buf[1024];
    cJSON *root_obj;

    memset(json_buf, 0, sizeof(json_buf));

    root_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(root_obj, JSON_ITEM_VFORMAT, vformat_list[format]);
    cJSON_AddStringToObject(root_obj, JSON_ITEM_VSIZE, vsize_list[size]);
    cJSON_AddStringToObject(root_obj, JSON_ITEM_VQUALITY, vquality_list[quality]);
    cJSON_PrintPreallocated(root_obj, json_buf, sizeof(json_buf), 1);
    cJSON_Delete(root_obj);

    FILE *fp = fopen(JSON_CONFIG_FILE_NAME, "w");
    fwrite(json_buf, 1, strlen(json_buf), fp);
    fclose(fp);

    return 0;
}

int read_json_cfg(video_format_e *format, video_size_e *size, video_quality_e *quality)
{
    char json_buf[1024];
    cJSON *root_obj;
    cJSON *tmp_obj;
    char *val;
    int i;

    memset(json_buf, 0, sizeof(json_buf));

    if (access(JSON_CONFIG_FILE_NAME, F_OK) != 0) {
        // 创建默认设置
        *format = VIDEO_FORMAT_H264;
        *size = VIDEO_SIZE_1920_1080;
        *quality = VIDEO_QUALITY_MID;
        save_json_cfg(*format, *size, *quality);
        return 0;
    }

    FILE *fp = fopen(JSON_CONFIG_FILE_NAME, "r");
    fread(json_buf, 1, sizeof(json_buf), fp);
    fclose(fp);

    root_obj = cJSON_Parse(json_buf);
    tmp_obj = cJSON_GetObjectItem(root_obj, JSON_ITEM_VFORMAT);
    val = cJSON_GetStringValue(tmp_obj);
    for (i = 0; i < sizeof(vformat_list)/sizeof(char *); i++) {
        if (strcmp(vformat_list[i], val) == 0) {
            *format = i;
            break;
        }
    }
    tmp_obj = cJSON_GetObjectItem(root_obj, JSON_ITEM_VSIZE);
    val = cJSON_GetStringValue(tmp_obj);
    for (i = 0; i < sizeof(vsize_list)/sizeof(char *); i++) {
        if (strcmp(vsize_list[i], val) == 0) {
            *size = i;
            break;
        }
    }
    tmp_obj = cJSON_GetObjectItem(root_obj, JSON_ITEM_VQUALITY);
    val = cJSON_GetStringValue(tmp_obj);
    for (i = 0; i < sizeof(vquality_list)/sizeof(char *); i++) {
        if (strcmp(vquality_list[i], val) == 0) {
            *quality = i;
            break;
        }
    }
    cJSON_Delete(root_obj);

    return 0;
}

int get_video_format(video_format_e *format, video_size_e *size, video_quality_e *quality)
{
    read_json_cfg(format, size, quality);
    return 0;
}

int set_hisi_video_cfg(video_format_e format, video_size_e size, video_quality_e quality)
{
    if (format == VIDEO_FORMAT_H264) {
        g_enPayLoad[0] = PT_H264;
    }
    else if (format == VIDEO_FORMAT_H265) {
        g_enPayLoad[0] = PT_H265;
    }

    if (size == VIDEO_SIZE_1280_720) {
        g_vpssSize[3] = PIC_720P;
        g_vencSize[0] = PIC_720P;
    }
    else if (size == VIDEO_SIZE_1920_1080) {
        g_vpssSize[3] = PIC_1080P;
        g_vencSize[0] = PIC_1080P;
    }

    g_br_index = quality;

    return 0;
}

int set_video_format(video_format_e format, video_size_e size, video_quality_e quality)
{
    save_json_cfg(format, size, quality);
    hisi_media_exit();
    set_hisi_video_cfg(format, size, quality);
    hisi_media_init();
    return 0;
}

void rec_key_job()
{
    static int32_t count = 0;
    static int32_t triggered = 0; // 标记按键长按是否已经触发
    key_data key_dat;
    rec_state_e rec_state = REC_STATE_IDLE;
    char path[64];
    char name[64];

    key_scan();

    key_dat = key_read();
    if (KEY_IS_FALLING(key_dat, 1)) {
        count += 1;
    }
    else if (KEY_IS_RISING(key_dat, 1)) {
        // DO NOTHING
    }
    else if (KEY_IS_PUSH(key_dat, 1)) {
        count += 1;
        // printf("key push count %d\n", count);
    }
    else {
        count = 0;
        triggered = 0;
    }

    // count 计数识别长按和短按
    if (count > 20 && triggered == 0) {
        triggered = 1;

        // 处理逻辑
        rec_state = rec_get_state();
        if (rec_state == REC_STATE_IDLE || rec_state == REC_STATE_STOP_OK) {
            memset(path, 0, sizeof(path));
            memset(name, 0, sizeof(name));

            strcpy(path, FULL_REC_PATH);
            // strcpy(name, "video"); // 简单粗暴的方案，直接使用"video+序号"，适合调试
            // 优雅一点的方案，使用系统时间
            struct tm *tm_ptr;
            time_t time_now;
            time(&time_now);
            tm_ptr = localtime(&time_now);
            strftime(name, sizeof(name), "%y%m%d_%H%M%S", tm_ptr);
            if (make_file_name(path, sizeof(path), name, sizeof(name))) {
                printf("generate filename error\n");
                return;
            }
            save_pv_img(name, pv_img_buf, sizeof(pv_img_buf));
            rec_set_filename(name); // 设置录像文件名，不含后缀名
            rec_set_state(REC_STATE_DO_START);
            gyro_log_start(name); // 设置gyroflow文件名，不含后缀名
            disable_gui_btn();
            set_rec_icon_start();
            printf("record start(%s)\n", name);
        }
        else if (rec_state == REC_STATE_START_OK) {
            rec_set_state(REC_STATE_DO_STOP);
            gyro_log_stop();
            enable_gui_btn();
            refresh_rec_time(0);
            set_rec_icon_stop();
            printf("record stop\n");
        }
    }
}

void rec_gui_job()
{
    struct timeval time_now;
    uint64_t time_now_ms;
    uint64_t time_delta_ms;
    rec_state_e rec_state = REC_STATE_IDLE;

    rec_state = rec_get_state();
    if (rec_state == REC_STATE_START_OK) {
        gettimeofday(&time_now, NULL);
        time_now_ms = time_now.tv_sec * 1000 + time_now.tv_usec / 1000;
        time_delta_ms = time_now_ms - rec_get_start_time_ms();
        refresh_rec_time(time_delta_ms);
    }
}

void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{
    // printf("disp area %d %d %d %d \n", area->x1, area->x2, area->y1, area->y2);
    ssp_st7789 lcd_data;
    lcd_data.area.x1 = area->x1;
    lcd_data.area.x2 = area->x2;
    lcd_data.area.y1 = area->y1;
    lcd_data.area.y2 = area->y2;
    lcd_data.area.len = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1) * sizeof(lv_color_t);
    lcd_data.area.data = (uint8_t *)color_p;
    ioctl(lcd_fd, SSP_LCD_SET_DRAW_AREA, (unsigned long)&lcd_data);

    lv_disp_flush_ready(disp);         /* Indicate you are ready with the flushing*/
}

void my_button_read(lv_indev_drv_t * drv, lv_indev_data_t *data)
{
    // 物理按键独立处理，不使用lvgl处理
}

// 不同批次的触摸屏坐标系方向、都有可能不同，需要在这里自行做修正
// 有可能翻转x、翻转y、翻转xy，触摸坐标范围可能为0-99、0-128、0-200、0-240等
// 以我目前使用的触摸屏为例，它的触摸坐标系和显示屏完全相反，需要做xy翻转
// 触摸范围为 0-127，去掉屏幕四周的黑边，这里取12-116映射到0-239
#define TOUCH_FLIP_X 1
#define TOUCH_FLIP_Y 1
#define TOUCH_MIN_COOR (0)
#define TOUCH_MAX_COOR (127)
#define TOUCH_CUT_SIZE (12)
#define TOUCH_VALID_SIZE (TOUCH_MAX_COOR - TOUCH_MIN_COOR - TOUCH_CUT_SIZE * 2)

void my_mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    // 临时禁用触摸屏
    // data->state = LV_INDEV_STATE_RELEASED;
    // return;


    (void) indev_drv; /*Unused*/

    int fix_x, fix_y;
    ft6206_touch_info touch_info;
    memset(&touch_info, 0, sizeof(touch_info));
    ft6206_read((void *)&dev_ft6206, &touch_info);
    if (touch_info.num > 0) {
        // printf("num %d, pos0 %d %d, pos1 %d %d \n", touch_info.num,
        //         touch_info.points[0].x, touch_info.points[0].y,
        //         touch_info.points[1].x, touch_info.points[1].y);

        if (touch_info.points[0].event == FT6206_EVENT_PRESS_DOWN ||
            touch_info.points[0].event == FT6206_EVENT_CONTACT ) {
            data->state = LV_INDEV_STATE_PRESSED;
        }
        else if (touch_info.points[0].event == FT6206_EVENT_LIFT_UP) {
            data->state = LV_INDEV_STATE_RELEASED;
        }
        else {
            data->state = LV_INDEV_STATE_RELEASED;
        }

#if TOUCH_FLIP_X == 1
        fix_x = TOUCH_MAX_COOR - touch_info.points[0].x + TOUCH_MIN_COOR;
#else
        fix_x = touch_info.points[0].x;
#endif

#if TOUCH_FLIP_Y == 1
        fix_y = TOUCH_MAX_COOR - touch_info.points[0].y + TOUCH_MIN_COOR;
#else
        fix_y = touch_info.points[0].y;
#endif

        fix_x = fix_x - TOUCH_CUT_SIZE;
        fix_x = fix_x > TOUCH_VALID_SIZE ? TOUCH_VALID_SIZE : fix_x;
        fix_x = fix_x * LCD_SCREEN_WIDTH / TOUCH_VALID_SIZE;

        fix_y = fix_y - TOUCH_CUT_SIZE;
        fix_y = fix_y > TOUCH_VALID_SIZE ? TOUCH_VALID_SIZE : fix_y;
        fix_y = fix_y * LCD_SCREEN_HEIGHT / TOUCH_VALID_SIZE;

        data->point.x = fix_x;
        data->point.y = fix_y;
    }
}

// LVGL的接口不是线程安全的，如需多线程操作必须加锁
// 我采用无锁方案，因此所有调用LVGL接口的操作只放在这个线程中进行
void *pth_lvgl_func(void *args)
{
    uint64_t period_ms;
    struct timespec tp = {0};
    struct timespec tp_old = {0};
    int32_t ret = 0;
    uint32_t loop_count = 0;

    pthread_manger_t *pth_ctx = (pthread_manger_t *)args;
    prctl(PR_SET_NAME, "pth_lvgl");
    pth_ctx->running = 1;
    pth_ctx->exit = 0;

    // lvgl初始化
    lv_init();

    // 显存初始化
    static lv_disp_draw_buf_t lvgl_disp_buf;
    memset(sg_lcd_buf1, 0, sizeof(sg_lcd_buf1));
    memset(sg_lcd_buf2, 0, sizeof(sg_lcd_buf2));
    lv_disp_draw_buf_init(&lvgl_disp_buf, sg_lcd_buf1, sg_lcd_buf2, LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT / 2);

    // 显示设备初始化
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &lvgl_disp_buf;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.hor_res = LCD_SCREEN_WIDTH;
    disp_drv.ver_res = LCD_SCREEN_HEIGHT;
    disp_drv.antialiasing = 1;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    // 物理按键初始化
    // static lv_indev_drv_t indev_drv_1;
    // lv_indev_drv_init(&indev_drv_1);
    // indev_drv_1.type = LV_INDEV_TYPE_BUTTON;
    // indev_drv_1.read_cb = my_button_read;
    // lv_indev_t *bt_indev = lv_indev_drv_register(&indev_drv_1);
    // lv_point_t button_points[1] = {{0,0}};
    // lv_indev_set_button_points(bt_indev, button_points);

    // 触摸屏初始化
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2);
    indev_drv_2.type = LV_INDEV_TYPE_POINTER;
    indev_drv_2.read_cb = my_mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_2);

    cam_gui_init_param param;
    param.get_battery_voltage = get_battery_voltage;
    param.get_cpu_temp = get_cpu_temp;
    param.get_video_format = get_video_format;
    param.set_video_format = set_video_format;
    cam_gui_init(&param);

    if (access(DEV_DISK_NAME, F_OK) == 0) {
        set_sd_card_icon(SD_CARD_OK);
    }
    else {
        set_sd_card_icon(SD_CARD_NOT_DETECT);
    }
    set_rec_icon_stop();

    clock_gettime(CLOCK_MONOTONIC, &tp);
    tp_old = tp;

    while (!pth_ctx->exit) {
        loop_count += 1;

        // 获取缩略图，可以适当降低缩略图刷新率
        if (loop_count % 2 == 0) {
            ret = get_preview_img(pv_img_buf);
            if (ret == 0) {
                //刷新缩略图
                lv_obj_invalidate(video_preview_obj);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &tp);
        period_ms = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000) - (tp_old.tv_sec * 1000 + tp_old.tv_nsec / 1000000);
        tp_old = tp;
        lv_tick_inc((uint32_t)period_ms);
        lv_timer_handler();

        rec_key_job();
        rec_gui_job();
        other_gui_job();

        usleep(25 * 1000);
    }

    lv_deinit();

    pth_ctx->running = 0;
    return NULL;
}


int main()
{
    char cmd[128] = {0};
    pthread_attr_t attr;
    time_t time_now;
    struct timeval tv;
    video_format_e format;
	video_size_e size;
	video_quality_e quality;

    process_exit = 0;
    signal(SIGTERM, main_exit);
    signal(SIGINT, main_exit);

    // 对于初次上电的的情况，设置系统默认时间，2022-01-01 00:00:00
    time(&time_now);
    if (time_now < 1640995200) {
        tv.tv_sec = 1640995200;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        system("hwclock -w");
    }

    // 挂载SD卡
    if (access(DEV_DISK_NAME, F_OK) == 0) {
        // 避免重复挂载
        if (read_df_cmd()) {
            sprintf(cmd, "mkdir -p %s", MOUNT_NODE);
            system(cmd);
            sprintf(cmd, "mount -t vfat %s %s", DEV_DISK_NAME, MOUNT_NODE);
            system(cmd);
            // sprintf(cmd, "mkdir -p %s", FULL_REC_PATH);
            sprintf(cmd, "mkdir -p /tmp/sd/video");
            system(cmd);
        }
    }

    // 读取配置文件
    read_json_cfg(&format, &size, &quality);
    // 设置媒体相关
    set_hisi_video_cfg(format, size, quality);

    memopen();
    set_hi_reg();
    usleep(100 * 1000);

    key_init();

    hiadc_init();
    hiadc_enable_ch0(1);

    i2c_fd2 = i2c_open("/dev/i2c-2");
    if (i2c_fd2 < 0) {
        printf("open /dev/i2c-2 error");
        return -1;
    }

    dev_ft6206.bus = i2c_fd2;
    dev_ft6206.addr = FT6202_I2C_ADDR;
    dev_ft6206.tenbit = 0;
    dev_ft6206.delay = 1;
    dev_ft6206.flags = 0;
    dev_ft6206.page_bytes = 256;
    dev_ft6206.iaddr_bytes = 1;
    ft6206_init((void *)&dev_ft6206);


    dev_mpu6050.bus = i2c_fd2;
    dev_mpu6050.addr = MPU6050_DEV_ADDR;
    dev_mpu6050.tenbit = 0;
    dev_mpu6050.delay = 1;
    dev_mpu6050.flags = 0;
    dev_mpu6050.page_bytes = 256;
    dev_mpu6050.iaddr_bytes = 1;
    // mpu6050_init((void *)&dev_mpu6050);

    gyro_log_init_param gyro_cfg;
    gyro_cfg.use_accelerometer = 1;
    gyro_cfg.use_gyroscope     = 1;
    gyro_cfg.use_magnetometer  = 0;
    gyro_cfg.reserve           = 0;
    gyro_cfg.freq              = 200;
    gyro_cfg.ascale            = 0.00012207031f; // ascale = 4/2^15 = 0.00012207031
    gyro_cfg.gscale            = 0.00053263221f; // gscale = (1000 * pi / 180)/2^15 = 0.00053263221
    gyro_cfg.mscale            = 0.0f;
    gyro_cfg.log_path          = FULL_REC_PATH;
    gyro_cfg.user_args         = (void *)&dev_mpu6050;
    gyro_cfg.sensor_name       = "mpu6050";
    gyro_cfg.sensor_init       = mpu6050_init;
    gyro_cfg.sensor_read       = mpu6050_read;
    gyro_cfg.sensor_destroy    = mpu6050_destroy;
    gyro_log_init(&gyro_cfg);

    cam_media_init(FULL_REC_PATH);
    
    if ((lcd_fd = open("/dev/st7789_lcd", O_RDWR)) == -1) {
        printf("open lcd failed\n");
        return -1;
    }
    ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_ON, NULL);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    pthread_create(&sg_pth_lvgl.pid, &attr, &pth_lvgl_func, &sg_pth_lvgl);
    pthread_attr_destroy(&attr);

    // 海思媒体相关
    hisi_media_init();

    while (!process_exit) {
        // 主线程暂时没用，如果内存特别紧张可以将LVGL线程放在这里，可以节约出一个线程栈的内存
        sleep(1);
    }

end:
    hisi_media_exit();

    sg_pth_lvgl.exit = 1;
    while(sg_pth_lvgl.running) {usleep(10 * 1000);}

    cam_media_exit();

    ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_OFF, NULL);
    close(lcd_fd);

    gyro_log_destroy();

    i2c_close(i2c_fd2);

    hiadc_deinit();

    key_exit();

    memclose();

    return 0;
}
