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

#include "himpp.h"
#include "camera_config.h"
#include "media.h"


#define LCD_SCREEN_WIDTH  (240)
#define LCD_SCREEN_HEIGHT (240)

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
static uint8_t sg_pv_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];

int i2c_fd2 = 0;
int lcd_fd = 0;
I2CDevice dev_ft6206;

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

    return;
}



static void btn_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        static uint8_t cnt = 0;
        cnt++;

        /*Get the first child of the button which is the label and change its text*/
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Button: %d", cnt);
    }
}

void lv_example_get_started_1(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

    lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
    lv_obj_center(label);
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

void my_button_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    // static int32_t last_btn = 0;   /*Store the last pressed button*/
    // int btn_pr = my_btn_read();     /*Get the ID (0,1,2...) of the pressed button*/

    // if(btn_pr >= 0) {               /*Is there a button press? (E.g. -1 indicated no button was pressed)*/
    //    last_btn = btn_pr;           /*Save the ID of the pressed button*/
    //    data->state = LV_INDEV_STATE_PRESSED;  /*Set the pressed state*/
    // } else {
    //    data->state = LV_INDEV_STATE_RELEASED; /*Set the released state*/
    // }

    // data->btn_id = last_btn;            /*Save the last button*/
}

void my_mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
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

        // ft6206 的有效值为 0-127，这里取4-123映射到0-239，触摸屏的坐标系和显示屏正好相反，这里做翻转
        fix_x = (touch_info.points[0].x - 4) * 2;
        fix_x = fix_x > 239 ? 239 : fix_x;
        fix_x = 239 - fix_x;
        fix_y = (touch_info.points[0].y - 4) * 2;
        fix_y = fix_y > 239 ? 239 : fix_y;
        fix_y = 239 - fix_y;
        data->point.x = fix_x;
        data->point.y = fix_y;
    }
}

void *pth_lvgl_func(void *args)
{
    uint64_t period_ms;
    struct timespec tp = {0};
    struct timespec tp_old = {0};
    lv_obj_t *lv_pv_area = NULL;
    lv_img_dsc_t lv_pv_img;
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

    // 初始化预览图相关
    lv_pv_area = lv_img_create(lv_scr_act());
    lv_obj_remove_style_all(lv_pv_area);
    lv_obj_align(lv_pv_area , LV_ALIGN_TOP_LEFT, 0, 30);

    // 下面这一堆代码的作用和 lv_img_buf_alloc 基本一致
    lv_pv_img.header.always_zero = 0;
    lv_pv_img.header.w = preview_u32Width;
    lv_pv_img.header.h = preview_u32Height;
    lv_pv_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    lv_pv_img.data_size = lv_img_buf_get_img_size(preview_u32Width, preview_u32Height, LV_IMG_CF_RGB565);
    lv_pv_img.data = sg_pv_img_buf;
    lv_img_set_src(lv_pv_area , &lv_pv_img);


    clock_gettime(CLOCK_MONOTONIC, &tp);
    tp_old = tp;

    while (!pth_ctx->exit) {
        loop_count += 1;

        // 获取缩略图，可以适当降低缩略图刷新率
        if (loop_count % 3 == 0) {
            ret = get_preview_img(sg_pv_img_buf);
            if (ret == 0) {
                //刷新缩略图
                lv_obj_invalidate(lv_pv_area);
            }
        }

        clock_gettime(CLOCK_MONOTONIC, &tp);
        period_ms = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000) - (tp_old.tv_sec * 1000 + tp_old.tv_nsec / 1000000);
        tp_old = tp;
        lv_tick_inc((uint32_t)period_ms);
        lv_timer_handler();
        usleep(20 * 1000);
    }

    lv_deinit();

    pth_ctx->running = 0;
    return NULL;
}


int main()
{
    pthread_attr_t attr;

    process_exit = 0;
    signal(SIGTERM, main_exit);
    signal(SIGINT, main_exit);

    memopen();
    set_hi_reg();
    usleep(100 * 1000);

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

    if ((lcd_fd = open("/dev/st7789_lcd", O_RDWR)) == -1) {
        printf("open lcd failed\n");
        return -1;
    }
    ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_ON, NULL);

    cam_media_init();

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256 * 1024);
    pthread_create(&sg_pth_lvgl.pid, &attr, &pth_lvgl_func, &sg_pth_lvgl);
    pthread_attr_destroy(&attr);

    // 海思媒体相关
    hisi_media_init();

    while (!process_exit) {
        sleep(1);
    }

end:
    hisi_media_exit();

    sg_pth_lvgl.exit = 1;
    while(sg_pth_lvgl.running) {usleep(10 * 1000);}

    cam_media_exit();

    ioctl(lcd_fd, SSP_LCD_SET_BACKLIGHT_OFF, NULL);
    close(lcd_fd);

    memclose();

    return 0;
}
