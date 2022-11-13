#include "lvgl.h"
#include "cam_gui.h"
#include "file_opt.h"
#include "lv_stb_truetype.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "camera_base_config.h"

// 存储路径
#define MOUNT_DEV_PATH "/dev/mmcblk0p1"
#define FILE_BASE_PATH "/tmp/sd/video/"
#define UPDATE_PACK_NAME "update_pack.tar"

// 视频拍摄实时显示区域
uint8_t pv_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];
lv_obj_t *video_preview_obj = NULL;
static lv_img_dsc_t sg_video_preview_img;

// 回放显示区域
uint8_t pb_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];
static lv_img_dsc_t sg_playback_img;

#define TOP_BOT_BOX_WIDTH  LCD_SCREEN_WIDTH
#define TOP_BOT_BOX_HEIGHT ((LCD_SCREEN_HEIGHT - PREVIEW_HEIGHT) / 2)


static cam_gui_init_param sg_init_param = {0};

// 设置视频格式用
static video_format_e sg_video_format = VIDEO_FORMAT_H264;
static video_size_e sg_video_size = VIDEO_SIZE_1920_1080;
static video_quality_e sg_video_quality = VIDEO_QUALITY_MID;

// 设置时间用
static struct tm sg_tm_info;


#define TITLE_BOX_WIDTH_SIZE 240
#define TITLE_BOX_HEIGHT_SIZE 40

// 录像图标小红点的尺寸
#define REC_TIME_ICON_SIZE 12

// GUI主界面根节点
static lv_obj_t *base_view;

// 屏幕顶部和底部的区域
static lv_obj_t *top_box;
static lv_obj_t *bottom_box;

// 主界面设置菜单对象
static lv_obj_t *menu_obj;

// 主界面4个角的GUI对象
static lv_obj_t *rec_time_obj;
static lv_obj_t *rec_time_icon;
static lv_obj_t *rec_time_lb;
static char rec_time_str[32];
static lv_obj_t *battery_obj;
static lv_obj_t *battery_icon;
static lv_obj_t *battery_lb;
static char battery_str[32];
static lv_obj_t *video_fmt_obj;
static lv_obj_t *video_fmt_icon;
static lv_obj_t *video_fmt_lb;
static char video_fmt_str[32];
static lv_obj_t *storage_obj;
static lv_obj_t *storage_icon;
static lv_obj_t *storage_lb;
static char storage_str[32];


// 字体样式，小中大三种规格
static lv_style_t style_text_s;
static lv_font_t my_font_s;
static lv_style_t style_text_m;
static lv_font_t my_font_m;
static lv_style_t style_text_l;
static lv_font_t my_font_l;

// 该字体样式仅用于显示图标
static lv_style_t style_text_icon;

// 标题栏样式
static lv_style_t style_title;
// 标题栏底部的分割线
static lv_point_t title_line_points[] = {{0, 0}, {236, 0}};
static lv_style_t style_title_line;

// 基础样式，目前用在主界面顶栏和底栏，和各种白色界面
static lv_style_t style_base;

// 软件升级相关
typedef enum {
    UPDATE_STATE_IDLE,
    UPDATE_STATE_START,
    UPDATE_STATE_RUNNING,
    UPDATE_STATE_FINISH,
} update_state_e;
static update_state_e sg_update_state = UPDATE_STATE_IDLE;

// 设备状态相关
typedef enum {
    REFRESH_DEV_STATE_STOP,
    REFRESH_DEV_STATE_RUNNING,
} refresh_dev_state_e;
static refresh_dev_state_e sg_refresh_dev_state = REFRESH_DEV_STATE_STOP;
static lv_obj_t *dev_state_text_label = NULL;
static char dev_state_text[256] = {0};
static sd_card_state_e sg_sd_card_state = SD_CARD_NOT_DETECT;

static lv_obj_t *make_title_box(lv_obj_t *parent, char *title, int32_t add_back_btn);
static lv_obj_t *settings_gui(lv_obj_t *parent);
static lv_obj_t *video_fmt_setting_gui(lv_obj_t *parent);
static lv_obj_t *file_manager_gui(lv_obj_t *parent);
static lv_obj_t *set_time_gui(lv_obj_t *parent);
static lv_obj_t *disk_usage_gui(lv_obj_t *parent);
static lv_obj_t *dev_state_gui(lv_obj_t *parent);
static lv_obj_t *help_gui(lv_obj_t *parent);
static lv_obj_t *update_gui(lv_obj_t *parent);
static lv_obj_t *about_gui(lv_obj_t *parent);

static void refresh_dev_state();
static void refresh_update_gui();
static void refresh_battery(float voltage);


typedef uint64_t mini_timer;

static void mini_timer_init(mini_timer *timer, uint64_t timeout_ms)
{
    struct timespec tp = {0};
    uint64_t now_ms;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    now_ms = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    *timer = now_ms + timeout_ms;
}

static int32_t mini_timer_is_timeout(mini_timer *timer)
{
    struct timespec tp = {0};
    uint64_t now_ms;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    now_ms = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    return now_ms > *timer ? 1 : 0;
}

#define BATTERY_VOLTAGE_CORRECTION (-0.35f)
static const float battery_voltage_level[5] = {
    // 我设想的理想情况下，电池电量和电压的关系差不多是这样的
    // 3.85, // 满电
    // 3.77, // 75%
    // 3.70, // 50%
    // 3.60, // 25%
    // 3.50, // 0%

    // 实际由于电池内阻、导线电阻等因素，电压适当下调一点
    3.85 + BATTERY_VOLTAGE_CORRECTION, // 满电
    3.77 + BATTERY_VOLTAGE_CORRECTION, // 75%
    3.70 + BATTERY_VOLTAGE_CORRECTION, // 50%
    3.60 + BATTERY_VOLTAGE_CORRECTION, // 25%
    3.50 + BATTERY_VOLTAGE_CORRECTION, // 0%
};

void other_gui_job()
{
    static mini_timer timer = 0;
    float val = 0.0;

    refresh_dev_state();
    refresh_update_gui(NULL);

    if (mini_timer_is_timeout(&timer)) {
        mini_timer_init(&timer, 10000);
        val = sg_init_param.get_battery_voltage();
        refresh_battery(val);
    }
    return;
}

void set_sd_card_icon(sd_card_state_e state)
{
    sg_sd_card_state = state;
    if (sg_sd_card_state == SD_CARD_OK) {
        lv_label_set_text(storage_icon, LV_SYMBOL_SD_CARD);
        lv_obj_set_style_text_color(storage_icon, lv_color_hex(0x000000), 0);

        lv_obj_set_style_text_color(storage_lb, lv_color_hex(0x000000), 0);
        lv_label_set_text(storage_lb, "正常");
    }
    else {
        lv_label_set_text(storage_icon, LV_SYMBOL_SD_CARD);
        lv_obj_set_style_text_color(storage_icon, lv_color_hex(0xED1C24), 0);

        lv_obj_set_style_text_color(storage_lb, lv_color_hex(0xED1C24), 0);
        lv_label_set_text(storage_lb, "异常");
    }
}

static void refresh_battery(float voltage)
{
    // 暂未使用，以后可以考虑现实精确电压或精确电量百分比
    // strcpy(battery_str, "75%");
    // lv_label_set_text(battery_lb, battery_str);

    lv_obj_set_style_text_color(battery_icon, lv_color_hex(0x000000), 0);
    if (voltage > battery_voltage_level[0]) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    }
    else if (voltage > battery_voltage_level[1]) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_3);
    }
    else if (voltage > battery_voltage_level[2]) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    }
    else if (voltage > battery_voltage_level[3]) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_1);
    }
    else if (voltage > battery_voltage_level[4]) {
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    }
    else {
        // 红色图标
        lv_obj_set_style_text_color(battery_icon, lv_color_hex(0xED1C24), 0);
        lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_EMPTY);
    }
    return;
}

void refresh_rec_time(uint64_t time_ms)
{
    int32_t min = 0;
    int32_t sec = 0;
    static int32_t sec_old = 0;
    
    min = time_ms / 1000 / 60;
    sec = time_ms / 1000 % 60;

    if (sec == sec_old) {
        return;
    }
    sec_old = sec;

    if ((sec & 0x01) == 1) {
        lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xED1C24), 0);
    }
    else {
        lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xFFFFFF), 0);
    }

    memset(rec_time_str, 0, sizeof(rec_time_str));
    sprintf(rec_time_str, "%02d:%02d", min, sec);
    lv_label_set_text(rec_time_lb, rec_time_str);
    return;
}

void set_rec_icon_start()
{
    lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xED1C24), 0);
    return;
}

void set_rec_icon_stop()
{
    // 录像停止用其他颜色，加以区分
    lv_obj_set_style_bg_color(rec_time_icon, lv_palette_main(LV_PALETTE_LIGHT_BLUE), 0);
    return;
}

void enable_gui_btn()
{
    lv_obj_add_flag(menu_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(video_fmt_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(storage_obj, LV_OBJ_FLAG_CLICKABLE);
    return;
}

void disable_gui_btn()
{
    lv_obj_clear_flag(menu_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(video_fmt_obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(storage_obj, LV_OBJ_FLAG_CLICKABLE);
    return;
}

// -------- event_handler --------

static void btn_rec_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        LV_LOG_USER("btn rec Clicked");
    }
}

static void btn_file_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        file_manager_gui(lv_scr_act());
    }
}

static void btn_setting_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        settings_gui(lv_scr_act());
    }
}

static void btn_video_setting_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        video_fmt_setting_gui(lv_scr_act());
    }
}

static void btn_disk_info_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        disk_usage_gui(lv_scr_act());
    }
}

static void btn_set_time_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        set_time_gui(lv_scr_act());
    }
}

static void btn_dev_state_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        dev_state_gui(lv_scr_act());
    }
}

static void btn_help_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        help_gui(lv_scr_act());
    }
}

static void btn_update_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        update_gui(lv_scr_act());
    }
}

static void btn_about_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        about_gui(lv_scr_act());
    }
}

// -------- create gui --------

static void btn_title_box_close_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_t * title_box = lv_obj_get_parent(btn);
        lv_obj_t * parent_box = lv_obj_get_parent(title_box);
        // lv_obj_del(title_box);
        lv_obj_del(parent_box);
    }
}

static lv_obj_t *make_title_box(lv_obj_t *parent, char *title, int32_t add_back_btn)
{
    lv_obj_t *title_box;

    title_box = lv_obj_create(parent);
    lv_obj_remove_style_all(title_box);
    lv_obj_add_style(title_box, &style_title, 0);
    lv_obj_set_size(title_box, TITLE_BOX_WIDTH_SIZE, TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(title_box, LV_ALIGN_TOP_LEFT, 0, 0);
    // lv_obj_set_flex_flow(title_box, LV_FLEX_FLOW_ROW);

    if (add_back_btn) {
        lv_obj_t *btn_back = lv_obj_create(title_box);
        lv_obj_remove_style_all(btn_back);
        lv_obj_add_style(btn_back, &style_base, 0);
        lv_obj_add_event_cb(btn_back, btn_title_box_close_handler, LV_EVENT_CLICKED, NULL);
        lv_obj_add_flag(btn_back, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(btn_back, 36, 36);
        lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 0, 0);

        lv_obj_t *btn_icon = lv_label_create(btn_back);
        lv_obj_add_style(btn_icon, &style_text_icon, 0);
        lv_label_set_text(btn_icon, LV_SYMBOL_LEFT);
        lv_obj_align(btn_icon, LV_ALIGN_CENTER, 0, 0);
    }

    // 标题
    lv_obj_t *title_label = lv_label_create(title_box);
    lv_obj_add_style(title_label, &style_text_m, 0);
    lv_label_set_text(title_label, title);
    // lv_obj_set_size(title_label, 160, 30);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -4);
    // lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);

    // 底部横线，做分割
    lv_obj_t *lv_tmp_obj = lv_line_create(title_box);
    lv_line_set_points(lv_tmp_obj, title_line_points, sizeof(title_line_points) / sizeof(lv_point_t));
    lv_obj_add_style(lv_tmp_obj, &style_title_line, 0);
    // lv_obj_center(lv_tmp_obj);
    lv_obj_set_align(lv_tmp_obj, LV_ALIGN_BOTTOM_MID);

    return title_box;
}

static void btn_event_handler_del_file_yes(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *user_data = lv_event_get_user_data(e);
    char *file_name = lv_obj_get_user_data(user_data);
    char cmd[128] = {0};

    if (code == LV_EVENT_CLICKED) {
        printf("yes delete file: %s \n", file_name);
        
#ifdef ENABLE_SIMULATE_SYSTEM_CALL
        printf("simulate do \"rm %s\"\n", file_name);
        printf("simulate do \"rm %s*\"\n", file_name);
#else
        memset(cmd, 0, sizeof(cmd));
        strcat(cmd, "rm ");
        strcat(cmd, FULL_REC_PATH);
        strcat(cmd, file_name);
        system(cmd);
        del_same_name_file(file_name);
#endif
        
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
        lv_obj_del(user_data);
    }
}

static void btn_event_handler_del_file_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

static void del_file_gui(lv_obj_t *parent, void *file_obj)
{
    lv_obj_t *del_file_view;
    lv_obj_t *text_label = NULL;
    lv_obj_t *btn;
    char *file_name = lv_obj_get_user_data(file_obj);
    char text[128];

    del_file_view = lv_obj_create(parent);
    lv_obj_remove_style_all(del_file_view);
    lv_obj_add_style(del_file_view, &style_base, 0);
    lv_obj_set_size(del_file_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(del_file_view, LV_ALIGN_TOP_LEFT, 0, 0);
    // lv_obj_set_flex_flow(del_file_view, LV_FLEX_FLOW_COLUMN);


    text_label = lv_label_create(del_file_view);
    lv_obj_set_size(text_label, 180, 100);
    lv_obj_set_pos(text_label, 30, 40);
    sprintf(text, "确定删除文件?\n\n%s", file_name);
    lv_label_set_text(text_label, text);
    lv_obj_add_style(text_label, &style_text_m, 0);


    // 确定键
    btn = lv_btn_create(del_file_view);
    lv_obj_add_event_cb(btn, btn_event_handler_del_file_yes, LV_EVENT_CLICKED, file_obj);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 60, 180);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0XED1C24), 0);
    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_OK);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);

    // 取消键
    btn = lv_btn_create(del_file_view);
    lv_obj_add_event_cb(btn, btn_event_handler_del_file_no, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 150, 180);
    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_CLOSE);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);

    return;
}

static void btn_del_file_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *user_data = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        del_file_gui(lv_scr_act(), user_data);
    }
}

static void btn_event_handler_set_video_format_yes(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // printf("set video format: %d %d %d \n", sg_video_format, sg_video_size, sg_video_quality);
        sg_init_param.set_video_format(sg_video_format, sg_video_size, sg_video_quality);

        if (sg_video_size == VIDEO_SIZE_1280_720) {
            lv_label_set_text(video_fmt_lb, "720P");
        }
        else if (sg_video_size == VIDEO_SIZE_1920_1080) {
            lv_label_set_text(video_fmt_lb, "1080P");
        }

        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(lv_obj_get_parent(parent));
    }
}

static void btn_event_handler_set_video_format_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(lv_obj_get_parent(parent));
    }
}

// 这个顺序和 video_format_e, video_size_e, video_quality_e 保持一致，方便进行设置
static char video_encoder_list[] = "H.264\nH.265";
static char video_size_list[] = "720p\n1080p";
static char video_quality_list[] = "低\n中\n高";

#define TAG_STR_VENC     "venc"
#define TAG_STR_VSIZE    "vsize"
#define TAG_STR_VQUALITY "vquality"
static void video_format_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    char *param_type = lv_event_get_user_data(e);
    int32_t id = 0;

    printf("param: %s \n", param_type);

    if (code == LV_EVENT_VALUE_CHANGED) {
        // char buf[32];
        // lv_roller_get_selected_str(obj, buf, sizeof(buf));
        // printf("Selected %s\n", buf);
        id = lv_roller_get_selected(obj);

        if (0 == strcmp(param_type, TAG_STR_VENC)) {
            sg_video_format = id;
        }
        else if (0 == strcmp(param_type, TAG_STR_VSIZE)) {
            sg_video_size = id;
        }
        else if (0 == strcmp(param_type, TAG_STR_VQUALITY)) {
            sg_video_quality = id;
        }
    }
}

#define VFMT_VISIBLE_ROW_COUNT 3
static lv_obj_t *video_fmt_setting_gui(lv_obj_t *parent)
{    
    lv_obj_t *vfmt_set_view = NULL;
    lv_obj_t *setting_view = NULL;
    lv_obj_t *tmp_obj = NULL;
    lv_obj_t *text_label = NULL;
    lv_obj_t *btn = NULL;

    int16_t label_pos_y  = 15;
    int16_t roller_pos_y = 45;


    vfmt_set_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(vfmt_set_view);
    lv_obj_add_style(vfmt_set_view, &style_base, 0);
    lv_obj_set_size(vfmt_set_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(vfmt_set_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(vfmt_set_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(vfmt_set_view, "录像规格", 1);

    setting_view = lv_obj_create(vfmt_set_view);
    lv_obj_remove_style_all(setting_view);
    lv_obj_add_style(setting_view, &style_base, 0);
    lv_obj_set_size(setting_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(setting_view, LV_ALIGN_TOP_LEFT, 0, 0);

    sg_init_param.get_video_format(&sg_video_format, &sg_video_size, &sg_video_quality);

    tmp_obj = lv_roller_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 60);
    lv_obj_set_pos(tmp_obj, 15, roller_pos_y);
    lv_obj_set_style_border_width(tmp_obj, 2, 0);
    lv_roller_set_options(tmp_obj, video_encoder_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VFMT_VISIBLE_ROW_COUNT);
    lv_roller_set_selected(tmp_obj, sg_video_format, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_VENC);

    tmp_obj = lv_label_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 30);
    lv_obj_set_pos(tmp_obj, 25, label_pos_y);
    lv_label_set_text(tmp_obj, "格式");



    tmp_obj = lv_roller_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 60);
    lv_obj_set_pos(tmp_obj, 90, roller_pos_y);
    lv_obj_set_style_border_width(tmp_obj, 2, 0);
    lv_roller_set_options(tmp_obj, video_size_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VFMT_VISIBLE_ROW_COUNT);
    lv_roller_set_selected(tmp_obj, sg_video_size, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_VSIZE);

    tmp_obj = lv_label_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 30);
    lv_obj_set_pos(tmp_obj, 100, label_pos_y);
    lv_label_set_text(tmp_obj, "尺寸");



    tmp_obj = lv_roller_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 60);
    lv_obj_set_pos(tmp_obj, 165, roller_pos_y);
    lv_obj_set_style_border_width(tmp_obj, 2, 0);
    lv_roller_set_options(tmp_obj, video_quality_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VFMT_VISIBLE_ROW_COUNT);
    lv_roller_set_selected(tmp_obj, sg_video_quality, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_VQUALITY);

    tmp_obj = lv_label_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 30);
    lv_obj_set_pos(tmp_obj, 175, label_pos_y);
    lv_label_set_text(tmp_obj, "质量");



    btn = lv_btn_create(setting_view);
    lv_obj_add_event_cb(btn, btn_event_handler_set_video_format_yes, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 50, 155);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0XED1C24), 0);

    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_OK);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);


    btn = lv_btn_create(setting_view);
    lv_obj_add_event_cb(btn, btn_event_handler_set_video_format_no, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 160, 155);

    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_CLOSE);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);

    return vfmt_set_view;
}


void *disk_gui_anim_obj[2];
static void disk_usage_anim_cb(void * obj, int32_t v)
{
    char usage_text[64] = "";
    lv_arc_set_value(disk_gui_anim_obj[0], v);
    sprintf(usage_text, "%d%%", v); // lv_label_set_text 不能处理 %% 转义符
    lv_label_set_text(disk_gui_anim_obj[1], usage_text);
}

static lv_obj_t *disk_usage_gui(lv_obj_t *parent)
{
    char usage_text[64] = "";
    lv_obj_t *disk_usage_view = NULL;
    lv_obj_t *disk_usage_box = NULL;
    lv_obj_t *text_label = NULL;

    read_df_cmd();

    disk_usage_view = lv_obj_create(parent);
    lv_obj_remove_style_all(disk_usage_view);
    lv_obj_add_style(disk_usage_view, &style_base, 0);
    lv_obj_set_size(disk_usage_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(disk_usage_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(disk_usage_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(disk_usage_view, "存储空间", 1);


    // 存储状态绘制区域
    disk_usage_box = lv_obj_create(disk_usage_view);
    lv_obj_set_size(disk_usage_box, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_set_style_pad_all(disk_usage_box, 0, 0);
    lv_obj_set_style_pad_gap(disk_usage_box, 0, 0);
    lv_obj_set_style_border_width(disk_usage_box, 0, 0);
    lv_obj_set_style_outline_width(disk_usage_box, 0, 0);


    // 存储百分比图标
    lv_obj_t *usage_arc = lv_arc_create(disk_usage_box);
    lv_obj_align(usage_arc, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_size(usage_arc, 130, 130);
    lv_arc_set_rotation(usage_arc, 135);
    lv_arc_set_bg_angles(usage_arc, 0, 270);
    lv_arc_set_range(usage_arc, 0, 100);
    lv_arc_set_value(usage_arc, 0);
    lv_obj_set_style_arc_width(usage_arc, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_width(usage_arc, 12, LV_PART_INDICATOR);
    lv_obj_remove_style(usage_arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(usage_arc, LV_OBJ_FLAG_CLICKABLE);

    // 存储百分比文字
    text_label = lv_label_create(disk_usage_box);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -20);
    sprintf(usage_text, "%d%%", 0); // lv_label_set_text 不能处理 %% 转义符
    lv_label_set_text(text_label, usage_text);
    lv_obj_add_style(text_label, &style_text_l, 0);

    disk_gui_anim_obj[0] = usage_arc;
    disk_gui_anim_obj[1] = text_label;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, disk_usage_anim_cb);
    lv_anim_set_time(&a, 800);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_set_values(&a, 0, sg_disk_info.percent);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);



    // 存储容量
    text_label = lv_label_create(disk_usage_box);
    lv_obj_set_pos(text_label, 30, 140);
    sprintf(usage_text, "已用%s 剩余%s\n共%s", sg_disk_info.used, sg_disk_info.free, sg_disk_info.size);
    lv_label_set_text(text_label, usage_text);
    lv_obj_add_style(text_label, &style_text_m, 0);

    return NULL;
}


static int month_day_table[13] = {0, 31, 28, 31, 30,
                                     31, 30, 31, 31,
                                     30, 31, 30, 31};

static char year_list[] =
          "2022\n2023\n2024\n2025\n"
    "2026\n2027\n2028\n2029\n2030\n"
    "2031\n2032";

static char month_list[] =
    "01\n02\n03\n04\n05\n06\n"
    "07\n08\n09\n10\n11\n12";

static char days_list[] =
    "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n"
    "11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n"
    "21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31";

static char hour_list[] =
    "00\n01\n02\n03\n04\n05\n"
    "06\n07\n08\n09\n10\n11\n"
    "12\n13\n14\n15\n16\n17\n"
    "18\n19\n20\n21\n22\n23";

static char minute_list[] =
    "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
    "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
    "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
    "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
    "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
    "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

#define TAG_STR_YEAR   "year"
#define TAG_STR_MONTH  "month"
#define TAG_STR_DAY    "day"
#define TAG_STR_HOUR   "hour"
#define TAG_STR_MINUTE "minute"
static void set_time_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    char *param_type = lv_event_get_user_data(e);
    int32_t id = 0;

    if (code == LV_EVENT_VALUE_CHANGED) {
        id = lv_roller_get_selected(obj);
        // printf("param: %s, id %d \n", param_type, id);

        if (0 == strcmp(param_type, TAG_STR_YEAR)) {
            sg_tm_info.tm_year = 2022 + id - 1900;
        }
        else if (0 == strcmp(param_type, TAG_STR_MONTH)) {
            sg_tm_info.tm_mon = id;
        }
        else if (0 == strcmp(param_type, TAG_STR_DAY)) {
            sg_tm_info.tm_mday = id + 1;
        }
        else if (0 == strcmp(param_type, TAG_STR_HOUR)) {
            sg_tm_info.tm_hour = id;
        }
        else if (0 == strcmp(param_type, TAG_STR_MINUTE)) {
            sg_tm_info.tm_min = id;
        }
    }
}

static void btn_event_handler_set_time_yes(lv_event_t *e)
{
    time_t tv_sec;
    struct timeval tv;
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        tv_sec = mktime(&sg_tm_info);
        tv.tv_sec = tv_sec;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        system("hwclock -w");
        
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

static void btn_event_handler_set_time_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

#define VISIBLE_ROW_COUNT 3
static lv_obj_t *set_time_gui(lv_obj_t *parent)
{
    int32_t id = 0;
    lv_obj_t *set_time_view = NULL;
    lv_obj_t *tmp_obj = NULL;
    lv_obj_t *text_label = NULL;
    lv_obj_t *btn = NULL;

    int16_t roller_y1 = 30, roller_y2 = 145;
    int16_t label_y1  = 5,  label_y2  = 120;

    time_t time_now;
    struct tm *tm_ptr;

    time(&time_now);
    // 对于初次上电的的情况，系统时间不得早于 2022-01-01 00:00:00
    if (time_now < 1640995200) {
        time_now = 1640995200;
    }
    tm_ptr = localtime(&time_now);
    sg_tm_info = *tm_ptr;


    set_time_view = lv_obj_create(parent);
    lv_obj_remove_style_all(set_time_view);
    lv_obj_add_style(set_time_view, &style_base, 0);
    lv_obj_set_size(set_time_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(set_time_view, LV_ALIGN_TOP_LEFT, 0, 0);



    tmp_obj = lv_label_create(set_time_view);
    lv_obj_remove_style_all(tmp_obj);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_pos(tmp_obj, 35, label_y1);
    lv_label_set_text(tmp_obj, "年");

    tmp_obj = lv_roller_create(set_time_view);
    lv_obj_add_style(tmp_obj, &style_text_s, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 50, 50);
    lv_obj_set_pos(tmp_obj, 20, roller_y1);
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_YEAR);
    lv_roller_set_options(tmp_obj, year_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = sg_tm_info.tm_year + 1900 - 2022;
    lv_roller_set_selected(tmp_obj, id, LV_ANIM_OFF);



    tmp_obj = lv_label_create(set_time_view);
    lv_obj_remove_style_all(tmp_obj);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_pos(tmp_obj, 110, label_y1);
    lv_label_set_text(tmp_obj, "月");

    tmp_obj = lv_roller_create(set_time_view);
    lv_obj_add_style(tmp_obj, &style_text_s, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 50, 50);
    lv_obj_set_pos(tmp_obj, 95, roller_y1);
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_MONTH);
    lv_roller_set_options(tmp_obj, month_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = sg_tm_info.tm_mon;
    lv_roller_set_selected(tmp_obj, id, LV_ANIM_OFF);



    tmp_obj = lv_label_create(set_time_view);
    lv_obj_remove_style_all(tmp_obj);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_pos(tmp_obj, 185, label_y1);
    lv_label_set_text(tmp_obj, "日");

    tmp_obj = lv_roller_create(set_time_view);
    lv_obj_add_style(tmp_obj, &style_text_s, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 50, 50);
    lv_obj_set_pos(tmp_obj, 170, roller_y1);
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_DAY);
    lv_roller_set_options(tmp_obj, days_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = sg_tm_info.tm_mday - 1;
    lv_roller_set_selected(tmp_obj, id, LV_ANIM_OFF);



    tmp_obj = lv_label_create(set_time_view);
    lv_obj_remove_style_all(tmp_obj);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_pos(tmp_obj, 35, label_y2);
    lv_label_set_text(tmp_obj, "时");

    tmp_obj = lv_roller_create(set_time_view);
    lv_obj_add_style(tmp_obj, &style_text_s, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 50, 50);
    lv_obj_set_pos(tmp_obj, 20, roller_y2);
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_HOUR);
    lv_roller_set_options(tmp_obj, hour_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = sg_tm_info.tm_hour;
    lv_roller_set_selected(tmp_obj, id, LV_ANIM_OFF);
    


    tmp_obj = lv_label_create(set_time_view);
    lv_obj_remove_style_all(tmp_obj);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_pos(tmp_obj, 110, label_y2);
    lv_label_set_text(tmp_obj, "分");

    tmp_obj = lv_roller_create(set_time_view);
    lv_obj_add_style(tmp_obj, &style_text_s, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 50, 50);
    lv_obj_set_pos(tmp_obj, 95, roller_y2);
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, TAG_STR_MINUTE);
    lv_roller_set_options(tmp_obj, minute_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = sg_tm_info.tm_min;
    lv_roller_set_selected(tmp_obj, id, LV_ANIM_OFF);


    // 确定键
    btn = lv_btn_create(set_time_view);
    lv_obj_add_event_cb(btn, btn_event_handler_set_time_yes, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 180, 150);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0XED1C24), 0);
    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_OK);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);

    // 取消键
    btn = lv_btn_create(set_time_view);
    lv_obj_add_event_cb(btn, btn_event_handler_set_time_no, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 30, 30);
    lv_obj_set_pos(btn, 180, 195);
    text_label = lv_label_create(btn);
    lv_obj_add_style(text_label, &style_text_icon, 0);
    lv_label_set_text(text_label, LV_SYMBOL_CLOSE);
    lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);

    return set_time_view;
}

static lv_obj_t *show_pv_img_gui(char *file_name)
{    
    lv_obj_t *base_view = NULL;
    lv_obj_t *pv_view = NULL;
    lv_obj_t *tmp_obj = NULL;
    lv_img_dsc_t *img_obj = NULL;
    int32_t img_is_exist = 0;

    base_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(base_view);
    lv_obj_add_style(base_view, &style_base, 0);
    lv_obj_set_size(base_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(base_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(base_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(base_view, "缩略图", 1);

    pv_view = lv_obj_create(base_view);
    lv_obj_remove_style_all(pv_view);
    lv_obj_add_style(pv_view, &style_base, 0);
    lv_obj_set_size(pv_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(pv_view, LV_ALIGN_TOP_LEFT, 0, 0);

    img_is_exist = search_pv_img(file_name);
    if (img_is_exist) {
        sg_playback_img.header.always_zero = 0;
        sg_playback_img.header.w = PREVIEW_WIDTH;
        sg_playback_img.header.h = PREVIEW_HEIGHT;
        sg_playback_img.header.cf = LV_IMG_CF_TRUE_COLOR;
        sg_playback_img.data_size = lv_img_buf_get_img_size(PREVIEW_WIDTH, PREVIEW_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        sg_playback_img.data = pb_img_buf;

        load_pv_img(file_name, sg_playback_img.data, sg_playback_img.data_size);
        tmp_obj = lv_img_create(pv_view);
        lv_obj_align(tmp_obj, LV_ALIGN_CENTER, 0, 0);
        lv_img_set_src(tmp_obj, &sg_playback_img);

        // 背景设置为黑色
        lv_obj_set_style_bg_color(pv_view, lv_color_hex(0x000000), 0);
    }
    else {
        tmp_obj = lv_label_create(pv_view);
        lv_obj_remove_style_all(tmp_obj);
        lv_obj_add_style(tmp_obj, &style_text_m, 0);
        lv_obj_align(tmp_obj, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(tmp_obj, "未找到缩略图");
    }

    return base_view;
}

static void btn_event_handler_show_pv_img(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *user_data = lv_event_get_user_data(e);

    if (code == LV_EVENT_CLICKED) {
        // printf("pv file name %s\n", user_data);
        show_pv_img_gui((char *)user_data);
    }
}

#define ITEM_BOX_HEIGHT 50
static lv_obj_t *add_file_item(lv_obj_t *parent, file_info *info)
{
    lv_obj_t *file_item_box = NULL;
    lv_obj_t *text_label = NULL;
    lv_obj_t *icon = NULL;

    file_item_box = lv_obj_create(parent);
    lv_obj_set_size(file_item_box, LCD_SCREEN_WIDTH, 60);
    lv_obj_set_style_pad_all(file_item_box, 0, 0);
    lv_obj_set_style_pad_gap(file_item_box, 0, 0);

    // 这里可以使用grid布局，但会额外多消耗几十字节，文件数量多的时候这个内存开销就比较客观可了

    // 文件图标
    icon = lv_label_create(file_item_box);
    lv_obj_add_style(icon, &style_text_icon, 0);
    lv_label_set_text(icon, LV_SYMBOL_FILE);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);

    // 文件名
    text_label = lv_label_create(file_item_box);
    lv_obj_remove_style_all(text_label);
    lv_obj_set_size(text_label, 160, 20);
    lv_label_set_long_mode(text_label, LV_LABEL_LONG_DOT);
    lv_label_set_text(text_label, info->name);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 40, 5);
    lv_obj_add_event_cb(text_label, btn_event_handler_show_pv_img, LV_EVENT_CLICKED, info->name);
    lv_obj_add_flag(text_label, LV_OBJ_FLAG_CLICKABLE);

    // 文件大小
    text_label = lv_label_create(file_item_box);
    lv_obj_remove_style_all(text_label);
    lv_obj_set_size(text_label, 160, 20);
    lv_label_set_text(text_label, info->size);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 40, 30);

    // 删除按钮
    icon = lv_label_create(file_item_box);
    lv_obj_add_style(icon, &style_text_icon, 0);
    lv_label_set_text(icon, LV_SYMBOL_TRASH);
    lv_obj_align(icon, LV_ALIGN_RIGHT_MID, -10, 0);
    // 这里注意，文件名设置到file_item_box中，后面的删除逻辑先删除文件，再删除file_item_box对象
    lv_obj_set_user_data(file_item_box, info->name);
    lv_obj_add_event_cb(icon, btn_del_file_handler, LV_EVENT_CLICKED, file_item_box);
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    return file_item_box;
}

static lv_obj_t *file_manager_gui(lv_obj_t *parent)
{
    lv_obj_t *file_manager_view;
    lv_obj_t *file_list_view;

    file_manager_view = lv_obj_create(parent);
    lv_obj_remove_style_all(file_manager_view);
    lv_obj_add_style(file_manager_view, &style_base, 0);
    lv_obj_set_size(file_manager_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(file_manager_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(file_manager_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(file_manager_view, "录像管理", 1);

    file_list_view = lv_obj_create(file_manager_view);
    lv_obj_remove_style_all(file_list_view);
    lv_obj_add_style(file_list_view, &style_base, 0);
    lv_obj_set_size(file_list_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(file_list_view, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_flex_flow(file_list_view, LV_FLEX_FLOW_COLUMN);

    int32_t i, n;
    n = read_ls_cmd();
    for (i = 0; i < n; i++) {
        // printf("%s %s %s\n", sg_file_list[i].type, sg_file_list[i].size, sg_file_list[i].name);
        add_file_item(file_list_view, &sg_file_list[i]);
    }
    if (n == MAX_FILE_NUM - 1) {
        lv_obj_t *text_label = lv_label_create(file_list_view);
        lv_obj_remove_style_all(text_label);
        lv_label_set_text(text_label, "已达到显示上限！\n更多文件请在电脑上查看");
        lv_obj_add_style(text_label, &style_text_m, 0);
        lv_obj_set_style_pad_all(text_label, 15, 0);
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
    }

    return file_manager_view;
}

static void dev_state_gui_close_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_DELETE) {
        LV_LOG_USER("close event");
        sg_refresh_dev_state = REFRESH_DEV_STATE_STOP;
    }
}

static void refresh_dev_state()
{
    static mini_timer timer = 0;
    int32_t slen = 0;
    float val = 0.0f;
    if (sg_refresh_dev_state == REFRESH_DEV_STATE_RUNNING) {
        if (mini_timer_is_timeout(&timer)) {
            mini_timer_init(&timer, 3000);
            memset(dev_state_text, 0 ,sizeof(dev_state_text));
            read_top_cmd(dev_state_text);

            slen = strlen(dev_state_text);
            val = sg_init_param.get_cpu_temp();
            sprintf(&dev_state_text[slen], "\ncpu temp: %3.2f", val);
            slen = strlen(dev_state_text);
            val = sg_init_param.get_battery_voltage();
            sprintf(&dev_state_text[slen], "\nbattery voltage: %1.3f", val);

            lv_label_set_text(dev_state_text_label, dev_state_text);
        }
    }
    return;
}

// 随便写的，没有优化显示，主要是为了方便调试，查看设备运行状态
static lv_obj_t *dev_state_gui(lv_obj_t *parent)
{
    lv_obj_t *dev_state_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;

    dev_state_view = lv_obj_create(parent);
    lv_obj_remove_style_all(dev_state_view);
    lv_obj_add_style(dev_state_view, &style_base, 0);
    lv_obj_set_size(dev_state_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(dev_state_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(dev_state_view, dev_state_gui_close_event_handler, LV_EVENT_DELETE, NULL);


    title_box = make_title_box(dev_state_view, "设备状态", 1);

    text_view = lv_obj_create(dev_state_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    dev_state_text_label = lv_label_create(text_view);
    lv_obj_set_size(dev_state_text_label, LCD_SCREEN_WIDTH - 20, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE -20);
    lv_obj_add_style(dev_state_text_label, &style_text_s, 0);
    lv_label_set_text(dev_state_text_label, dev_state_text);
    lv_obj_align(dev_state_text_label, LV_ALIGN_TOP_LEFT, 10, 10);

    sg_refresh_dev_state = REFRESH_DEV_STATE_RUNNING;
    
    return dev_state_view;
}

static lv_obj_t *help_gui(lv_obj_t *parent)
{
    lv_obj_t *help_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;

    help_view = lv_obj_create(parent);
    lv_obj_remove_style_all(help_view);
    lv_obj_add_style(help_view, &style_base, 0);
    lv_obj_set_size(help_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(help_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(help_view, "使用帮助", 1);

    text_view = lv_obj_create(help_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_remove_style_all(text_label);
    lv_obj_set_width(text_label, LCD_SCREEN_WIDTH);
    lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_pad_all(text_label, 10, 0);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_label_set_text(text_label,
    "●长按机身顶部按键开始（红点闪烁）/停止（蓝点常亮）录像\n"
    "●目前无拍照功能，录像过程中所有菜单不可操作\n"
    "●左下角设置录像规格，设置时有轻微卡顿属正常现象\n"
    "●右下角为录像管理，点击可查看缩略图，最多显示前63个文件，更多文件请在电脑上进行管理\n"
    "●软件升级：将升级文件（update_pack.tar）放入SD卡根目录，系统检测到后即可升级\n"
    "●其他：\n"
    "设置时间日期是一个使用频率很低，但是开发却较麻烦的功能，月份天数不对的问题使用过程中注意就行，暂不修复\n"
    );
    // 这里的文字不要使用"1.2.3."类似这样的英文序号，lvgl对中文的自动换行很难看

    return help_view;
}

static pthread_t pid_update;

static void *pth_update_func(void *args)
{
    sleep(1);
    sg_update_state = UPDATE_STATE_RUNNING;

#ifdef ENABLE_SIMULATE_SYSTEM_CALL
    printf("simulate do \"rm -rf /home/app/*\"\n");
    printf("simulate do \"mkdir -p /home/app/\"\n");
    printf("simulate do \"tar -xf /tmp/sd/" UPDATE_PACK_NAME " -C /home/app/\"\n");
#else
    system("rm -rf /home/app/*");
    system("mkdir -p /home/app/");
    system("tar -xf /tmp/sd/" UPDATE_PACK_NAME " -C /home/app/");
#endif

    sleep(1);
    sg_update_state = UPDATE_STATE_FINISH;
    sleep(1);
    
    return NULL;
}

// 第一次调用需要传递父指针，其他时候为空
static void refresh_update_gui(lv_obj_t *parent)
{
    static mini_timer timer = 0;
    static int32_t countdown = -1;
    static lv_obj_t *sl_parent = NULL;
    static lv_obj_t *text_label = NULL;
    char tmp_str[64];

    if (parent != NULL) {
        sl_parent = parent;
    }

    if (sg_update_state == UPDATE_STATE_IDLE) {
        return;
    }
    else if (sg_update_state == UPDATE_STATE_START) {
        // DO NOTHING
        return;
    }
    else if (sg_update_state == UPDATE_STATE_RUNNING) {
        // DO NOTHING
        return;
    }
    else if (sg_update_state == UPDATE_STATE_FINISH){
        if (countdown == -1) {
            countdown = 6;

            lv_obj_clean(sl_parent);
            text_label = lv_label_create(sl_parent);
            lv_obj_set_size(text_label, LCD_SCREEN_WIDTH - 20, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE - 20);
            lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
            lv_obj_add_style(text_label, &style_text_m, 0);
        }
        else {
            if (mini_timer_is_timeout(&timer)) {
                countdown -= 1;
                mini_timer_init(&timer, 1000);
                memset(tmp_str, 0, sizeof(tmp_str));
                sprintf(tmp_str, "\n\n升级完成\n\n%d秒后自动重启", countdown);
                lv_label_set_text(text_label, tmp_str);
            }
            if (countdown == 0) {
                printf("update finish, system reboot\n");
                
#ifdef ENABLE_SIMULATE_SYSTEM_CALL
                printf("simulate do \"reboot\"\n");
#else
                system("reboot");
#endif
                while(1){sleep(1);};
            }
        }
    }
}

static void btn_event_handler_update_yes(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btn);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_clean(parent);

        lv_obj_t *text_label;
        text_label = lv_label_create(parent);
        lv_obj_set_size(text_label, LCD_SCREEN_WIDTH - 20, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE - 20);
        lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
        lv_obj_add_style(text_label, &style_text_m, 0);
        lv_label_set_text(text_label, "\n正在升级，请稍等");

        lv_obj_t *tmp;
        tmp = lv_spinner_create(parent, 2500, 50);
        lv_obj_set_size(tmp, 80, 80);
        lv_obj_align(tmp, LV_ALIGN_CENTER, 0, 20);

        // 这个线程只做升级，不做任何GUI处理
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 256 * 1024);
        pthread_create(&pid_update, &attr, &pth_update_func, NULL);
        pthread_attr_destroy(&attr);
        sg_update_state = UPDATE_STATE_START;

        refresh_update_gui(parent);
    }
}

static void btn_event_handler_update_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(lv_obj_get_parent(parent));
    }
}

static lv_obj_t *update_gui(lv_obj_t *parent)
{
    lv_obj_t *update_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;
    lv_obj_t *btn;

    update_view = lv_obj_create(parent);
    lv_obj_remove_style_all(update_view);
    lv_obj_add_style(update_view, &style_base, 0);
    lv_obj_set_size(update_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(update_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(update_view, "软件升级", 0);

    text_view = lv_obj_create(update_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_set_size(text_label, LCD_SCREEN_WIDTH - 20, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE - 20);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);

    if (access("/tmp/sd/" UPDATE_PACK_NAME, F_OK) == 0) {
        lv_label_set_text(text_label,
            "请保证电量充足\n"
            "升级过程中请勿操作\n"
            "升级完成将自动重启\n"
            "\n已发现升级文件\n"
            "是否开始升级？\n"
        );
        // 文字按钮
        btn = lv_btn_create(text_view);
        lv_obj_add_event_cb(btn, btn_event_handler_update_yes, LV_EVENT_CLICKED, NULL);
        lv_obj_set_size(btn, 30, 30);
        lv_obj_set_pos(btn, 60, 150);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0XED1C24), 0);

        text_label = lv_label_create(btn);
        lv_obj_add_style(text_label, &style_text_icon, 0);
        lv_label_set_text(text_label, LV_SYMBOL_OK);
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);


        btn = lv_btn_create(text_view);
        lv_obj_add_event_cb(btn, btn_event_handler_update_no, LV_EVENT_CLICKED, NULL);
        lv_obj_set_size(btn, 30, 30);
        lv_obj_set_pos(btn, 150, 150);

        text_label = lv_label_create(btn);
        lv_obj_add_style(text_label, &style_text_icon, 0);
        lv_label_set_text(text_label, LV_SYMBOL_CLOSE);
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
    }
    else {
        lv_label_set_text(text_label,
            "请保证电量充足\n"
            "升级过程中请勿操作\n"
            "升级完成将自动重启\n"
            "\n未找到升级文件\n"
        );

        btn = lv_btn_create(text_view);
        lv_obj_add_event_cb(btn, btn_event_handler_update_no, LV_EVENT_CLICKED, NULL);
        lv_obj_set_size(btn, 30, 30);
        lv_obj_set_pos(btn, 150, 150);

        text_label = lv_label_create(btn);
        lv_obj_add_style(text_label, &style_text_icon, 0);
        lv_label_set_text(text_label, LV_SYMBOL_CLOSE);
        lv_obj_align(text_label, LV_ALIGN_CENTER, 0, 0);
    }

    return update_view;
}

static lv_obj_t *about_gui(lv_obj_t *parent)
{
    lv_obj_t *about_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;

    about_view = lv_obj_create(parent);
    lv_obj_remove_style_all(about_view);
    lv_obj_add_style(about_view, &style_base, 0);
    lv_obj_set_size(about_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(about_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(about_view, "关于", 1);

    text_view = lv_obj_create(about_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_set_size(text_label, LCD_SCREEN_WIDTH - 20, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE - 20);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_label_set_text(text_label,
        "软件版本 " SOFTWARE_VERSION "\n" GIT_BRANCH_NAME ":" GIT_COMMIT_HASH "\n"
        "项目主页 https://gitee.com/dma/action_2_poor\n"
        "\n感谢“嘉立创”对本项目的支持！"
    );

    return about_view;
}

static lv_obj_t *settings_gui(lv_obj_t *parent)
{
    lv_obj_t *settings_view;
    lv_obj_t *title_box;
    lv_obj_t *settings_list_view;

    settings_view = lv_obj_create(parent);
    lv_obj_remove_style_all(settings_view);
    lv_obj_add_style(settings_view, &style_base, 0);
    lv_obj_set_size(settings_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(settings_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(settings_view, "设置", 1);

    settings_list_view = lv_obj_create(settings_view);
    lv_obj_remove_style_all(settings_list_view);
    lv_obj_add_style(settings_list_view, &style_base, 0);
    lv_obj_set_size(settings_list_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(settings_list_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);
    lv_obj_set_flex_flow(settings_list_view, LV_FLEX_FLOW_COLUMN);


    lv_obj_t *setting_item;
    lv_obj_t *setting_label;


    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_set_time_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "时间日期");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_disk_info_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "存储空间");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_dev_state_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "设备状态");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_help_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "使用帮助");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_update_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "软件升级");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, LCD_SCREEN_WIDTH, 50);
    lv_obj_add_event_cb(setting_item, btn_about_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "关于");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    return settings_view;
}

static void cam_base_gui(void)
{
    lv_obj_t *lv_tmp_obj;

    base_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(base_view);
    lv_obj_add_style(base_view, &style_base, 0);
    lv_obj_set_size(base_view, LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_obj_align(base_view, LV_ALIGN_TOP_LEFT, 0, 0);


    // 顶边栏
    top_box = lv_obj_create(base_view);
    lv_obj_remove_style_all(top_box);
    lv_obj_add_style(top_box, &style_base, 0);
    lv_obj_set_size(top_box, TOP_BOT_BOX_WIDTH, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(top_box, LV_ALIGN_TOP_LEFT, 0, 0);

    // 录像时间
    rec_time_obj = lv_obj_create(top_box);
    lv_obj_remove_style_all(rec_time_obj);
    lv_obj_set_size(rec_time_obj, 80, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(rec_time_obj, LV_ALIGN_TOP_LEFT, 0, 0);
#ifdef ENABLE_SIMULATE_SYSTEM_CALL
    // 调试期间暂时代替录像物理按键
    lv_obj_add_event_cb(rec_time_obj, btn_rec_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(rec_time_obj, LV_OBJ_FLAG_CLICKABLE);
#endif
    rec_time_icon = lv_obj_create(rec_time_obj);
    lv_obj_remove_style_all(rec_time_icon);
    lv_obj_set_size(rec_time_icon, REC_TIME_ICON_SIZE, REC_TIME_ICON_SIZE);
    lv_obj_set_style_radius(rec_time_icon, REC_TIME_ICON_SIZE, 0);
    lv_obj_set_style_bg_opa(rec_time_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(rec_time_icon, 0, 0);
    // lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xED1C24), 0);
    lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(rec_time_icon, LV_ALIGN_LEFT_MID, 5, 0);
    rec_time_lb = lv_label_create(rec_time_obj);
    lv_obj_add_style(rec_time_lb, &style_text_m, 0);
    lv_obj_align(rec_time_lb, LV_ALIGN_LEFT_MID, REC_TIME_ICON_SIZE + 10, 0);
    strcpy(rec_time_str, "00:00");
    lv_label_set_text(rec_time_lb, rec_time_str);

    // 菜单
    menu_obj = lv_obj_create(top_box);
    lv_obj_remove_style_all(menu_obj);
    lv_obj_set_size(menu_obj, 60, 28);
    lv_obj_align(menu_obj, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(menu_obj, btn_setting_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(menu_obj, LV_OBJ_FLAG_CLICKABLE);

    // 使用线条来装饰菜单按钮（使用图标有点丑）
    static lv_point_t line_points[] = {{0, 0}, {40, 0}};
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 8);
    lv_style_set_line_color(&style_line, lv_color_hex(0xD0D0D0));
    lv_style_set_line_rounded(&style_line, true);

    lv_tmp_obj = lv_line_create(menu_obj);
    lv_line_set_points(lv_tmp_obj, line_points, 2);
    lv_obj_add_style(lv_tmp_obj, &style_line, 0);
    lv_obj_align(lv_tmp_obj, LV_ALIGN_CENTER, 4, 4); // 位置根据线宽调整


    // 电池
    battery_obj = lv_obj_create(top_box);
    lv_obj_remove_style_all(battery_obj);
    lv_obj_set_size(battery_obj, 80, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(battery_obj, LV_ALIGN_TOP_LEFT, 160, 0);
    battery_icon = lv_label_create(battery_obj);
    lv_obj_add_style(battery_icon, &style_text_icon, 0);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_2);
    lv_obj_align(battery_icon, LV_ALIGN_RIGHT_MID, -5, 0);
    battery_lb = lv_label_create(battery_obj);
    lv_obj_add_style(battery_lb, &style_text_m, 0);
    lv_obj_align(battery_lb, LV_ALIGN_RIGHT_MID, -35, 0);
    // strcpy(battery_str, "75%"); // TODO 暂未使用，以后可以考虑现实精确电压或精确电量百分比
    strcpy(battery_str, ""); // TODO 暂未使用，以后可以考虑现实精确电压或精确电量百分比
    lv_label_set_text(battery_lb, battery_str);



    // 视频拍摄实时显示区域
    sg_video_preview_img.header.always_zero = 0;
    sg_video_preview_img.header.w = PREVIEW_WIDTH;
    sg_video_preview_img.header.h = PREVIEW_HEIGHT;
    sg_video_preview_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    sg_video_preview_img.data_size = lv_img_buf_get_img_size(PREVIEW_WIDTH, PREVIEW_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    sg_video_preview_img.data = pv_img_buf;
    video_preview_obj = lv_img_create(base_view);
    lv_img_set_src(video_preview_obj, &sg_video_preview_img);
    lv_obj_align(video_preview_obj, LV_ALIGN_TOP_LEFT, 0, TOP_BOT_BOX_HEIGHT);



    // 底边栏
    bottom_box = lv_obj_create(base_view);
    lv_obj_remove_style_all(bottom_box);
    lv_obj_add_style(bottom_box, &style_base, 0);
    lv_obj_set_size(bottom_box, TOP_BOT_BOX_WIDTH, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(bottom_box, LV_ALIGN_TOP_LEFT, 0, TOP_BOT_BOX_HEIGHT + PREVIEW_HEIGHT);

    // 录像规格设置
    video_fmt_obj = lv_obj_create(bottom_box);
    lv_obj_remove_style_all(video_fmt_obj);
    lv_obj_set_size(video_fmt_obj, 100, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(video_fmt_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_event_cb(video_fmt_obj, btn_video_setting_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(video_fmt_obj, LV_OBJ_FLAG_CLICKABLE);
    video_fmt_icon = lv_label_create(video_fmt_obj);
    lv_obj_add_style(video_fmt_icon, &style_text_icon, 0);
    lv_label_set_text(video_fmt_icon, LV_SYMBOL_VIDEO);
    lv_obj_align(video_fmt_icon, LV_ALIGN_LEFT_MID, 5, 0);
    video_fmt_lb = lv_label_create(video_fmt_obj);
    lv_obj_add_style(video_fmt_lb, &style_text_m, 0);
    lv_obj_align(video_fmt_lb, LV_ALIGN_LEFT_MID, 30, 0);
    if (sg_video_size == VIDEO_SIZE_1280_720) {
        lv_label_set_text(video_fmt_lb, "720P");
    }
    else if (sg_video_size == VIDEO_SIZE_1920_1080) {
        lv_label_set_text(video_fmt_lb, "1080P");
    }

    // 文件管理
    storage_obj = lv_obj_create(bottom_box);
    lv_obj_remove_style_all(storage_obj);
    lv_obj_set_size(storage_obj, 75, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(storage_obj, LV_ALIGN_TOP_LEFT, 165, 0);
    lv_obj_add_event_cb(storage_obj, btn_file_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(storage_obj, LV_OBJ_FLAG_CLICKABLE);
    storage_icon = lv_label_create(storage_obj);
    lv_obj_add_style(storage_icon, &style_text_icon, 0);
    lv_label_set_text(storage_icon, LV_SYMBOL_SD_CARD);
    lv_obj_align(storage_icon, LV_ALIGN_LEFT_MID, 0, 0);
    storage_lb = lv_label_create(storage_obj);
    lv_obj_add_style(storage_lb, &style_text_m, 0);
    lv_obj_align(storage_lb, LV_ALIGN_LEFT_MID, 25, 0);
    strcpy(storage_str, "异常");
    lv_label_set_text(storage_lb, storage_str);
}

int32_t cam_gui_init(cam_gui_init_param *param)
{
    memset(&sg_init_param, 0, sizeof(sg_init_param));
    sg_init_param = *param;

    sg_init_param.get_video_format(&sg_video_format, &sg_video_size, &sg_video_quality);

    if (lv_stbtt_init(&my_font_s, 18.0)) {
        return 1;
    }
    lv_style_init(&style_text_s);
    lv_style_set_text_font(&style_text_s, &my_font_s);
    lv_style_set_text_color(&style_text_s, lv_color_hex(0x000000));

    if (lv_stbtt_init(&my_font_m, 22.0)) {
        return 1;
    }
    lv_style_init(&style_text_m);
    lv_style_set_text_font(&style_text_m, &my_font_m);
    lv_style_set_text_color(&style_text_m, lv_color_hex(0x000000));

    if (lv_stbtt_init(&my_font_l, 30.0)) {
        return 1;
    }
    lv_style_init(&style_text_l);
    lv_style_set_text_font(&style_text_l, &my_font_l);
    lv_style_set_text_color(&style_text_l, lv_color_hex(0x000000));

    lv_style_init(&style_text_icon);
    lv_style_set_text_font(&style_text_icon, &lv_font_montserrat_20);


    // 标题栏
    lv_style_init(&style_title);
    lv_style_set_bg_opa(&style_title, LV_OPA_COVER);
    lv_style_set_bg_color(&style_title, lv_color_hex(0xFFFFFF));
    // 标题栏底部的分割线
    lv_style_init(&style_title_line);
    lv_style_set_line_width(&style_title_line, 2);
    lv_style_set_line_color(&style_title_line, lv_color_hex(0xCCCCCC));
    lv_style_set_line_rounded(&style_title_line, true);


    // 基础样式
    lv_style_init(&style_base);
    lv_style_set_bg_opa(&style_base, LV_OPA_COVER);
    lv_style_set_bg_color(&style_base, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&style_base, 0);
    // lv_style_set_border_color(&style_base, lv_color_hex(0xFF0000)); // 调试查看边框用
    // lv_style_set_border_width(&style_base, 1); // 调试查看边框用
    lv_style_set_pad_all(&style_base, 0);
    // lv_style_set_outline_width(&style_base, 0);
    // lv_style_set_line_width(&style_base, 0);
    // lv_style_set_shadow_width(&style_base, 0);
    // lv_style_set_arc_width(&style_base, 0);

    cam_base_gui();
    return 0;
}
