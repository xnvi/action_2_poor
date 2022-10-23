#include "lvgl.h"
#include "cam_gui.h"
#include "lv_stb_truetype.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "camera_config.h"

// 存储路径
#define MOUNT_DEV_PATH "/dev/mmcblk0p1"
#define FILE_BASE_PATH "/tmp/sd/video/"

// 缩略图显示区域
#define PREVIEW_WIDTH_SIZE 240
#define PREVIEW_HEIGHT_SIZE 180
// 显示图像时需要手动做裁剪
uint8_t sg_pv_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];
lv_obj_t *video_preview_obj = NULL;
static lv_img_dsc_t video_preview_img;

#define MONITOR_HOR_RES 240 // 模拟器使用的宏，真机记得替换 TODO
#define MONITOR_VER_RES 240 // 模拟器使用的宏，真机记得替换 TODO
#define TOP_BOT_BOX_WIDTH  MONITOR_VER_RES
#define TOP_BOT_BOX_HEIGHT ((MONITOR_VER_RES - PREVIEW_HEIGHT_SIZE) / 2)


#define TIME_FMT_STR "%Y%m%d_%H%M%S"


// 设置时间用
static struct tm tm_info;


#define TITLE_BOX_WIDTH_SIZE 240
#define TITLE_BOX_HEIGHT_SIZE 40


static lv_obj_t *base_view;

// 屏幕顶部和底部的区域
static lv_obj_t *top_box;
static lv_obj_t *bottom_box;

static lv_obj_t *menu_obj;

// 录像图标小红点的尺寸
#define REC_TIME_ICON_SIZE 12

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

int32_t refresh_dev_state = 0;
lv_obj_t *dev_state_text_label = NULL;
char dev_state_text[256] = {0};


// 字体样式，小中大三种规格
static lv_style_t style_text_s;
static lv_font_t my_font_s;
static lv_style_t style_text_m;
static lv_font_t my_font_m;
static lv_style_t style_text_l;
static lv_font_t my_font_l;

// 用于显示图标
static lv_style_t style_text_icon;

// 标题栏样式
static lv_style_t style_title;
// 标题栏底部的分割线
static lv_point_t title_line_points[] = {{0, 0}, {200, 0}};
static lv_style_t style_title_line;

// 基础样式，目前用在主界面顶栏和底栏，和各种白色界面
static lv_style_t style_base;


static int32_t is_recording = 0;


lv_obj_t *make_title_box(lv_obj_t *parent, char *title, int32_t add_back_btn);
lv_obj_t *settings_gui(lv_obj_t *parent);
lv_obj_t *video_fmt_setting_gui(lv_obj_t *parent);
lv_obj_t *file_manager_gui(lv_obj_t *parent);
lv_obj_t *set_time_gui(lv_obj_t *parent);
lv_obj_t *disk_usage_obj(lv_obj_t *parent);
lv_obj_t *dev_state_gui(lv_obj_t *parent);
lv_obj_t *help_gui(lv_obj_t *parent);
lv_obj_t *update_gui(lv_obj_t *parent);
lv_obj_t *about_gui(lv_obj_t *parent);


// TODO
void update_battery(int32_t val)
{
    return;
}

// TODO
void update_rec_anim(int32_t is_running)
{
    return;
}


// -------- event_handler --------

static void btn_rec_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("btn rec Clicked");
    }
}

static void btn_file_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        file_manager_gui(lv_scr_act());
    }
}

static void btn_setting_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        settings_gui(lv_scr_act());
    }
}

static void btn_video_setting_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        video_fmt_setting_gui(lv_scr_act());
    }
}

static void btn_disk_info_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        disk_usage_obj(lv_scr_act());
    }
}

static void btn_set_time_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        set_time_gui(lv_scr_act());
    }
}

static void btn_dev_state_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        dev_state_gui(lv_scr_act());
    }
}

static void btn_help_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        help_gui(lv_scr_act());
    }
}

static void btn_update_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        update_gui(lv_scr_act());
    }
}

static void btn_about_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        about_gui(lv_scr_act());
    }
}

static void btn_event_handler_del_file_yes(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    char *file_name = lv_event_get_user_data(e);


    if(code == LV_EVENT_CLICKED) {
        printf("yes delete file: %s \n", file_name);
        
        // system("rm " file_name);
        
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

static void btn_event_handler_del_file_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

static void del_file_gui(lv_obj_t *parent, char *file_name)
{
    lv_obj_t *del_file_view;
    lv_obj_t *text_label = NULL;
    lv_obj_t *btn;
    char text[128];

    del_file_view = lv_obj_create(parent);
    lv_obj_remove_style_all(del_file_view);
    lv_obj_add_style(del_file_view, &style_base, 0);
    lv_obj_set_size(del_file_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(del_file_view, LV_ALIGN_TOP_LEFT, 0, 0);
    // lv_obj_set_flex_flow(del_file_view, LV_FLEX_FLOW_COLUMN);


    text_label = lv_label_create(del_file_view);
    lv_obj_set_size(text_label, 180, 100);
    lv_obj_set_pos(text_label, 30, 40);
    sprintf(text, "确定删除文件?\n\n%s", "这里我要测试一个超级长的文件名，以免到时候出问题1243487502.ts");
    lv_label_set_text(text_label, text);
    lv_obj_add_style(text_label, &style_text_m, 0);


    // 确定键
    btn = lv_btn_create(del_file_view);
    lv_obj_add_event_cb(btn, btn_event_handler_del_file_yes, LV_EVENT_CLICKED, NULL);
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
    char *file_name = lv_event_get_user_data(e);

    printf("delete file: %s \n", file_name);

    if(code == LV_EVENT_CLICKED) {
        del_file_gui(lv_scr_act(), file_name);
    }
}

static void btn_title_box_close_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * btn = lv_event_get_target(e);
        lv_obj_t * title_box = lv_obj_get_parent(btn);
        lv_obj_t * parent_box = lv_obj_get_parent(title_box);
        // printf("parent1 %p\n", mbox);
        // printf("parent2 %p\n", lv_obj_get_parent(mbox));
        // lv_obj_del(title_box);
        lv_obj_del(parent_box);
        // lv_obj_del(lv_obj_get_parent(mbox));
    }
}

lv_obj_t *make_title_box(lv_obj_t *parent, char *title, int32_t add_back_btn)
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


static char video_encoder_list[] = "H.264\nH.265";
static char video_size_list[] = "720p\n1080p";
static char video_quality_list[] = "低\n中\n高";

static void video_format_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    char *param_type = lv_event_get_user_data(e);

    printf("param: %s \n", param_type);

    if(code == LV_EVENT_VALUE_CHANGED) {
        char buf[32];
        lv_roller_get_selected_str(obj, buf, sizeof(buf));
        // // lv_roller_get_selected(obj);
        // TODO
        LV_LOG_USER("Selected %s\n", buf);
    }
}

#define VFMT_VISIBLE_ROW_COUNT 3
lv_obj_t *video_fmt_setting_gui(lv_obj_t *parent)
{    
    lv_obj_t *vfmt_set_view = NULL;
    lv_obj_t *setting_view = NULL;
    lv_obj_t *tmp_obj = NULL;

    int16_t roller_pos_y = 35;
    int16_t label_pos_y  = 140;


    vfmt_set_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(vfmt_set_view);
    lv_obj_add_style(vfmt_set_view, &style_base, 0);
    lv_obj_set_size(vfmt_set_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(vfmt_set_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(vfmt_set_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(vfmt_set_view, "录像规格", 1);

    setting_view = lv_obj_create(vfmt_set_view);
    lv_obj_remove_style_all(setting_view);
    lv_obj_add_style(setting_view, &style_base, 0);
    lv_obj_set_size(setting_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(setting_view, LV_ALIGN_TOP_LEFT, 0, 0);



    tmp_obj = lv_roller_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 60);
    lv_obj_set_pos(tmp_obj, 15, roller_pos_y);
    lv_obj_set_style_border_width(tmp_obj, 2, 0);
    lv_roller_set_options(tmp_obj, video_encoder_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VFMT_VISIBLE_ROW_COUNT);
    lv_roller_set_selected(tmp_obj, 1, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, "enc");

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
    lv_roller_set_selected(tmp_obj, 1, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, "size");

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
    lv_roller_set_selected(tmp_obj, 1, LV_ANIM_OFF);
    lv_obj_add_event_cb(tmp_obj, video_format_event_handler, LV_EVENT_VALUE_CHANGED, "quality");

    tmp_obj = lv_label_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(tmp_obj, 60, 30);
    lv_obj_set_pos(tmp_obj, 175, label_pos_y);
    lv_label_set_text(tmp_obj, "质量");




    tmp_obj = lv_label_create(setting_view);
    lv_obj_add_style(tmp_obj, &style_text_m, 0);
    lv_obj_align(tmp_obj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_label_set_text(tmp_obj, "此功能正在开发中");

    return vfmt_set_view;
}


void *disk_gui_anim_obj[2];
static void disk_usage_anim_cb(void * obj, int32_t v)
{
    // lv_arc_set_value(obj, v);

    char usage_text[64] = "";
    lv_arc_set_value(disk_gui_anim_obj[0], v);
    sprintf(usage_text, "%d%%", v); // lv_label_set_text 不能处理 %% 转义符
    lv_label_set_text(disk_gui_anim_obj[1], usage_text);
}

lv_obj_t *disk_usage_obj(lv_obj_t *parent)
{
    char usage_text[64] = "";
    lv_obj_t *disk_usage_view = NULL;
    lv_obj_t *disk_usage_box = NULL;
    lv_obj_t *text_label = NULL;
    int32_t parcent = 65;

    // TODO 命令 df -h

    disk_usage_view = lv_obj_create(parent);
    lv_obj_remove_style_all(disk_usage_view);
    lv_obj_add_style(disk_usage_view, &style_base, 0);
    lv_obj_set_size(disk_usage_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(disk_usage_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(disk_usage_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(disk_usage_view, "存储空间", 1);


    // 存储状态绘制区域
    disk_usage_box = lv_obj_create(disk_usage_view);
    lv_obj_set_size(disk_usage_box, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
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
    // lv_obj_set_pos(text_label, 120, 120);
    // sprintf(usage_text, "%d%%", parcent); // lv_label_set_text 不能处理 %% 转义符
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
    lv_anim_set_values(&a, 0, parcent);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);



    // 存储容量
    text_label = lv_label_create(disk_usage_box);
    lv_obj_set_pos(text_label, 30, 140);
    sprintf(usage_text, "已用%s 剩余%s\n共%s", "12.3G", "19.1G", "32.0G");
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

static void set_time_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    char *param_type = lv_event_get_user_data(e);
    int32_t id = 0;

    if(code == LV_EVENT_VALUE_CHANGED) {
        id = lv_roller_get_selected(obj);
        // printf("param: %s, id %d \n", param_type, id);

        if (0 == strcmp(param_type, "year")) {
            tm_info.tm_year = 2022 + id - 1900;
        }
        else if (0 == strcmp(param_type, "month")) {
            tm_info.tm_mon = id;
        }
        else if (0 == strcmp(param_type, "day")) {
            tm_info.tm_mday = id;
        }
        else if (0 == strcmp(param_type, "hour")) {
            tm_info.tm_hour = id;
        }
        else if (0 == strcmp(param_type, "minute")) {
            tm_info.tm_min = id;
        }
    }
}


static void btn_event_handler_set_time_yes(lv_event_t *e)
{
    time_t tv_sec;
    struct timeval tv;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        tv_sec = mktime(&tm_info);
        tv.tv_sec = tv_sec;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

static void btn_event_handler_set_time_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(parent);
    }
}

#define VISIBLE_ROW_COUNT 3
lv_obj_t *set_time_gui(lv_obj_t *parent)
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
    if (time_now < 1640966400) {
        time_now = 1640966400;
    }
    tm_ptr = localtime(&time_now);
    tm_info = *tm_ptr;


    set_time_view = lv_obj_create(parent);
    lv_obj_remove_style_all(set_time_view);
    lv_obj_add_style(set_time_view, &style_base, 0);
    lv_obj_set_size(set_time_view, MONITOR_HOR_RES, MONITOR_VER_RES);
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
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, "year");
    lv_roller_set_options(tmp_obj, year_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = tm_info.tm_year + 1900 - 2022;
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
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, "month");
    lv_roller_set_options(tmp_obj, month_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = tm_info.tm_mon;
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
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, "day");
    lv_roller_set_options(tmp_obj, days_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = tm_info.tm_mday;
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
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, "hour");
    lv_roller_set_options(tmp_obj, hour_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = tm_info.tm_hour;
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
    lv_obj_add_event_cb(tmp_obj, set_time_event_handler, LV_EVENT_VALUE_CHANGED, "minute");
    lv_roller_set_options(tmp_obj, minute_list, LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(tmp_obj, VISIBLE_ROW_COUNT);
    id = tm_info.tm_min;
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

// TODO 新添一个入参 file_info
#define ITEM_BOX_HEIGHT 50
lv_obj_t *add_file_item(lv_obj_t *parent)
{
    lv_obj_t *file_item_box = NULL;
    lv_obj_t *text_label = NULL;
    lv_obj_t *icon = NULL;

    static lv_coord_t col_dsc[] = {40, 160, 40, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] = {30, 20, LV_GRID_TEMPLATE_LAST};

    file_item_box = lv_obj_create(parent);
    lv_obj_set_size(file_item_box, MONITOR_HOR_RES, 60);
    lv_obj_set_style_pad_all(file_item_box, 0, 0);
    lv_obj_set_style_pad_gap(file_item_box, 0, 0);
    lv_obj_set_grid_dsc_array(file_item_box, col_dsc, row_dsc);

    // 文件图标
    icon = lv_label_create(file_item_box);
    lv_obj_add_style(icon, &style_text_icon, 0);
    lv_label_set_text(icon, LV_SYMBOL_FILE);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 0, 1,
                                    LV_GRID_ALIGN_CENTER, 0, 2);

    // 文件名
    text_label = lv_label_create(file_item_box);
    // lv_obj_remove_style_all(text_label);
    lv_obj_set_style_pad_all(text_label, 0, 0);
    lv_obj_set_style_pad_gap(text_label, 0, 0);
    lv_obj_set_style_border_width(text_label, 3, 0);
    lv_obj_set_style_outline_width(text_label, 3, 0);
    lv_obj_set_style_border_color(text_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_outline_color(text_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_size(text_label, 160, 20);
    lv_label_set_text(text_label, "1111-22-33-44-55-66.ts");
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_obj_set_grid_cell(text_label, LV_GRID_ALIGN_CENTER, 1, 1,
                                     LV_GRID_ALIGN_CENTER, 0, 1);


    // 文件大小
    text_label = lv_label_create(file_item_box);
    // lv_obj_remove_style_all(text_label);
    lv_obj_set_style_pad_all(text_label, 0, 0);
    lv_obj_set_style_pad_gap(text_label, 0, 0);
    lv_obj_set_style_border_width(text_label, 3, 0);
    lv_obj_set_style_outline_width(text_label, 3, 0);
    lv_obj_set_style_border_color(text_label, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_outline_color(text_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_size(text_label, 160, 20);
    lv_label_set_text(text_label, "1234 KB");
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_obj_set_grid_cell(text_label, LV_GRID_ALIGN_CENTER, 1, 1,
                                     LV_GRID_ALIGN_CENTER, 1, 1);

    // 删除按钮
    icon = lv_label_create(file_item_box);
    lv_obj_add_style(icon, &style_text_icon, 0);
    lv_label_set_text(icon, LV_SYMBOL_TRASH);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_grid_cell(icon, LV_GRID_ALIGN_CENTER, 2, 1,
                                    LV_GRID_ALIGN_CENTER, 0, 2);
    lv_obj_add_event_cb(icon, btn_del_file_handler, LV_EVENT_CLICKED, "this_is_file_name");
    lv_obj_add_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    return file_item_box;
}

lv_obj_t *file_manager_gui(lv_obj_t *parent)
{
    lv_obj_t *file_manager_view;
    lv_obj_t *file_list_view;

    file_manager_view = lv_obj_create(parent);
    lv_obj_remove_style_all(file_manager_view);
    lv_obj_add_style(file_manager_view, &style_base, 0);
    lv_obj_set_size(file_manager_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(file_manager_view, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_flex_flow(file_manager_view, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title_box = make_title_box(file_manager_view, "录像管理", 1);

    // TODO 绘制文件列表
    // 使用 ls -lh

    file_list_view = lv_obj_create(file_manager_view);
    lv_obj_remove_style_all(file_list_view);
    lv_obj_add_style(file_list_view, &style_base, 0);
    lv_obj_set_size(file_list_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(file_list_view, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_flex_flow(file_list_view, LV_FLEX_FLOW_COLUMN);

    int32_t i;
    for (i = 0; i < 6; i++) {
        add_file_item(file_list_view);
    }

    return file_manager_view;
}


static void btn_event_handler_dev_state_refresh(lv_event_t *e)
{
    char tmp_str[64];
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        memset(dev_state_text, 0, sizeof(dev_state_text));
        sprintf(tmp_str, "CPU负载 %d%%\n", 12);
        strcat(dev_state_text, tmp_str);
        sprintf(tmp_str, "CPU温度 %.2f\n", 56.78);
        strcat(dev_state_text, tmp_str);
        sprintf(tmp_str, "电池电压 %.2f\n", 3.87);
        strcat(dev_state_text, tmp_str);
        if (1) {
            sprintf(tmp_str, "SD卡状态\n");
            strcat(dev_state_text, tmp_str);
            sprintf(tmp_str, "  已使用 %d\n  总容量 %d\n", 12, 34);
            strcat(dev_state_text, tmp_str);
        }
        else {
            sprintf(tmp_str, "SD卡状态：未插卡\n");
            strcat(dev_state_text, tmp_str);
        }

        time_t time_now;
        time(&time_now);
        sprintf(tmp_str, "time %ld\n", time_now);
        strcat(dev_state_text, tmp_str);

        lv_label_set_text(dev_state_text_label, dev_state_text);
        // lv_obj_invalidate(dev_state_text_label);
    }
}

// TODO 随便写的，主要是为了检测温度，没有太大实际用途
lv_obj_t *dev_state_gui(lv_obj_t *parent)
{
    lv_obj_t *dev_state_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *btn;
    lv_obj_t *tmp;

    dev_state_view = lv_obj_create(parent);
    lv_obj_remove_style_all(dev_state_view);
    lv_obj_add_style(dev_state_view, &style_base, 0);
    lv_obj_set_size(dev_state_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(dev_state_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(dev_state_view, "设备状态", 1);

    text_view = lv_obj_create(dev_state_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    dev_state_text_label = lv_label_create(text_view);
    lv_obj_set_size(dev_state_text_label, MONITOR_HOR_RES - 20, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE -20);
    lv_obj_add_style(dev_state_text_label, &style_text_m, 0);
    lv_label_set_text(dev_state_text_label, dev_state_text);
    lv_obj_align(dev_state_text_label, LV_ALIGN_TOP_LEFT, 10, 10);


    btn = lv_btn_create(dev_state_view);
    lv_obj_add_event_cb(btn, btn_event_handler_dev_state_refresh, LV_EVENT_CLICKED, NULL);
    lv_obj_set_size(btn, 60, 30);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 90, 190);

    tmp = lv_label_create(btn);
    lv_obj_add_style(tmp, &style_text_m, 0);
    lv_label_set_text(tmp, "刷新");
    lv_obj_align(tmp, LV_ALIGN_CENTER, 0, -3);
    
    return dev_state_view;
}

lv_obj_t *help_gui(lv_obj_t *parent)
{
    lv_obj_t *help_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;

    help_view = lv_obj_create(parent);
    lv_obj_remove_style_all(help_view);
    lv_obj_add_style(help_view, &style_base, 0);
    lv_obj_set_size(help_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(help_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(help_view, "使用帮助", 1);

    text_view = lv_obj_create(help_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_set_size(text_label, MONITOR_HOR_RES - 20, 500); // TODO 这样处理肯定不是最优方案，先这样吧，以后有时间再优化
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_label_set_text(text_label,
    "长按机身顶部按键开始/停止录像\n"
    "录像过程中所有菜单不可操作\n"
    "设置时间日期是一个使用频率很低，但是开发却较麻烦的功能，月份天数不对的问题使用过程中注意就行，暂不修复\n"
    "TODO\n"
    );

    return help_view;
}

pthread_t pid_update;


void *pth_update_func(void *args)
{
    // while (1) {
    //     /* code */
    //     // system("");
    // }

    // system("tar -xvf /tmp/sd/update_pack.tar TODO");
    sleep(3);


    lv_obj_clean(args);

    lv_obj_t *text_label;
    text_label = lv_label_create(args);
    lv_obj_set_size(text_label, MONITOR_HOR_RES - 20, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE - 20);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);

    int32_t i;
    char tmp_str[64];
    for (i = 5; i >= 0; i--) {
        memset(tmp_str, 0, sizeof(tmp_str));
        sprintf(tmp_str, "\n\n升级完成\n\n%d秒后自动重启", i);
        lv_label_set_text(text_label, tmp_str);
        sleep(1);
    }
    // system("reboot");
    
    return NULL;
}

static void btn_event_handler_update_yes(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *parent = lv_obj_get_parent(btn);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_clean(parent);

        lv_obj_t *text_label;
        text_label = lv_label_create(parent);
        lv_obj_set_size(text_label, MONITOR_HOR_RES - 20, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE - 20);
        lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
        lv_obj_add_style(text_label, &style_text_m, 0);
        lv_label_set_text(text_label, "\n正在升级，请稍等");

        lv_obj_t *tmp;
        tmp = lv_spinner_create(parent, 2500, 50);
        lv_obj_set_size(tmp, 80, 80);
        lv_obj_align(tmp, LV_ALIGN_CENTER, 0, 20);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 256 * 1024);
        pthread_create(&pid_update, &attr, &pth_update_func, parent);
        pthread_attr_destroy(&attr);
    }
}

static void btn_event_handler_update_no(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn);
        lv_obj_del(lv_obj_get_parent(parent));
    }
}

lv_obj_t *update_gui(lv_obj_t *parent)
{
    lv_obj_t *update_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;
    lv_obj_t *btn;

    update_view = lv_obj_create(parent);
    lv_obj_remove_style_all(update_view);
    lv_obj_add_style(update_view, &style_base, 0);
    lv_obj_set_size(update_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(update_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(update_view, "软件升级", 0);

    text_view = lv_obj_create(update_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_set_size(text_label, MONITOR_HOR_RES - 20, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE - 20);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 20, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);


    // if (access("/tmp/sd/update_pack.tar", F_OK) == 0) {
    if (1) {
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

lv_obj_t *about_gui(lv_obj_t *parent)
{
    lv_obj_t *about_view;
    lv_obj_t *title_box;
    lv_obj_t *text_view;
    lv_obj_t *text_label;

    about_view = lv_obj_create(parent);
    lv_obj_remove_style_all(about_view);
    lv_obj_add_style(about_view, &style_base, 0);
    lv_obj_set_size(about_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(about_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(about_view, "关于", 1);

    text_view = lv_obj_create(about_view);
    lv_obj_remove_style_all(text_view);
    lv_obj_add_style(text_view, &style_base, 0);
    lv_obj_set_size(text_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(text_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);

    text_label = lv_label_create(text_view);
    lv_obj_set_size(text_label, MONITOR_HOR_RES - 20, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE - 20);
    lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_add_style(text_label, &style_text_m, 0);
    lv_label_set_text(text_label,
        "软件版本 v0.1\n"
        "项目主页 https://gitee.com/dma/action_2_poor\n"
        "\n\n感谢 嘉立创 对本项目的支持！"
    );

    return about_view;
}

lv_obj_t *settings_gui(lv_obj_t *parent)
{
    lv_obj_t *settings_view;
    lv_obj_t *title_box;
    lv_obj_t *settings_list_view;

    settings_view = lv_obj_create(parent);
    lv_obj_remove_style_all(settings_view);
    lv_obj_add_style(settings_view, &style_base, 0);
    lv_obj_set_size(settings_view, MONITOR_HOR_RES, MONITOR_VER_RES);
    lv_obj_align(settings_view, LV_ALIGN_TOP_LEFT, 0, 0);

    title_box = make_title_box(settings_view, "设置", 1);

    settings_list_view = lv_obj_create(settings_view);
    lv_obj_remove_style_all(settings_list_view);
    lv_obj_add_style(settings_list_view, &style_base, 0);
    lv_obj_set_size(settings_list_view, MONITOR_HOR_RES, MONITOR_VER_RES - TITLE_BOX_HEIGHT_SIZE);
    lv_obj_align(settings_list_view, LV_ALIGN_TOP_LEFT, 0, TITLE_BOX_HEIGHT_SIZE);
    lv_obj_set_flex_flow(settings_list_view, LV_FLEX_FLOW_COLUMN);


    lv_obj_t *setting_item;
    lv_obj_t *setting_label;


    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_set_time_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "时间日期");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_disk_info_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "存储空间");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_dev_state_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "设备状态");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_help_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "使用帮助");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_update_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "软件升级");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    setting_item = lv_obj_create(settings_list_view);
    lv_obj_set_size(setting_item, MONITOR_HOR_RES, 50);
    lv_obj_add_event_cb(setting_item, btn_about_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(setting_item, LV_OBJ_FLAG_CLICKABLE);
    setting_label = lv_label_create(setting_item);
    lv_obj_add_style(setting_label, &style_text_m, 0);
    lv_label_set_text(setting_label, "关于");
    lv_obj_align(setting_label, LV_ALIGN_LEFT_MID, 0, 0);

    return settings_view;
}

void cam_base_gui(void)
{
    lv_obj_t *lv_tmp_obj;

    base_view = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(base_view);
    lv_obj_add_style(base_view, &style_base, 0);
    lv_obj_set_size(base_view, MONITOR_HOR_RES, MONITOR_VER_RES);
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
    lv_obj_add_event_cb(rec_time_obj, btn_rec_event_handler, LV_EVENT_CLICKED, NULL); // TODO 调试期间暂时代替录像物理按键
    lv_obj_add_flag(rec_time_obj, LV_OBJ_FLAG_CLICKABLE); // TODO 调试期间暂时代替录像物理按键
    rec_time_icon = lv_obj_create(rec_time_obj);
    lv_obj_set_size(rec_time_icon, REC_TIME_ICON_SIZE, REC_TIME_ICON_SIZE);
    lv_obj_set_style_radius(rec_time_icon, REC_TIME_ICON_SIZE, 0);
    lv_obj_set_style_bg_opa(rec_time_icon, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(rec_time_icon, 0, 0);
    lv_obj_set_style_bg_color(rec_time_icon, lv_color_hex(0xED1C24), 0);
    lv_obj_align(rec_time_icon, LV_ALIGN_LEFT_MID, 5, 0);
    rec_time_lb = lv_label_create(rec_time_obj);
    lv_obj_add_style(rec_time_lb, &style_text_m, 0);
    lv_obj_align(rec_time_lb, LV_ALIGN_LEFT_MID, REC_TIME_ICON_SIZE + 10, 0);
    strcpy(rec_time_str, "12:34");
    lv_label_set_text(rec_time_lb, rec_time_str);
    lv_obj_invalidate(rec_time_lb);

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
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE_GREY));
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
    strcpy(battery_str, "75%");
    lv_label_set_text(battery_lb, battery_str);
    lv_obj_invalidate(battery_lb);



    // 预览区域，TODO 待完善
    video_preview_img.header.always_zero = 0;
    video_preview_img.header.w = PREVIEW_WIDTH_SIZE;
    video_preview_img.header.h = PREVIEW_HEIGHT_SIZE;
    video_preview_img.header.cf = LV_IMG_CF_TRUE_COLOR;
    video_preview_img.data_size = lv_img_buf_get_img_size(PREVIEW_WIDTH_SIZE, PREVIEW_HEIGHT_SIZE, LV_IMG_CF_TRUE_COLOR);
    video_preview_img.data = sg_pv_img_buf;
    video_preview_obj = lv_img_create(base_view);
    lv_img_set_src(video_preview_obj, &video_preview_img);
    lv_obj_align(video_preview_obj, LV_ALIGN_TOP_LEFT, 0, TOP_BOT_BOX_HEIGHT);



    // 底边栏
    bottom_box = lv_obj_create(base_view);
    lv_obj_remove_style_all(bottom_box);
    lv_obj_add_style(bottom_box, &style_base, 0);
    lv_obj_set_size(bottom_box, TOP_BOT_BOX_WIDTH, TOP_BOT_BOX_HEIGHT);
    lv_obj_align(bottom_box, LV_ALIGN_TOP_LEFT, 0, TOP_BOT_BOX_HEIGHT + PREVIEW_HEIGHT_SIZE);

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
    strcpy(video_fmt_str, "1080P");
    lv_label_set_text(video_fmt_lb, video_fmt_str);
    lv_obj_invalidate(video_fmt_lb);

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
    strcpy(storage_str, "正常");
    lv_label_set_text(storage_lb, storage_str);
    lv_obj_invalidate(storage_lb);
}

// 临时测试用
// void update_pv_img()
// {
//     static int i = 0;
//     ((uint32_t *)preview_frame_buf)[i] = 0xFFFF00FF;
//     if (i < 240 * 180) {
//         i++;
//     }
//     if (lv_obj_is_valid(video_preview_obj)) {
//         lv_obj_invalidate(video_preview_obj);
//     }
// }

void cam_gui_init()
{
    lv_stbtt_init(&my_font_s, 18.0);
	lv_style_init(&style_text_s);
	lv_style_set_text_font(&style_text_s, &my_font_s);
    lv_style_set_text_color(&style_text_s, lv_color_hex(0x000000));

    lv_stbtt_init(&my_font_m, 22.0);
	lv_style_init(&style_text_m);
	lv_style_set_text_font(&style_text_m, &my_font_m);
    lv_style_set_text_color(&style_text_m, lv_color_hex(0x000000));

    lv_stbtt_init(&my_font_l, 30.0);
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
    lv_style_set_line_color(&style_title_line, lv_color_hex(0xA0A0A0));
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
}