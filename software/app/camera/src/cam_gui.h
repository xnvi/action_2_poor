#ifndef _CAM_GUI_H
#define _CAM_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "camera_base_config.h"

uint8_t pv_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];
lv_obj_t *video_preview_obj;

typedef enum {
    VIDEO_FORMAT_H264,
    VIDEO_FORMAT_H265,
} video_format_e;

typedef enum {
    VIDEO_SIZE_1280_720,
    VIDEO_SIZE_1920_1080,
} video_size_e;

typedef enum {
    VIDEO_QUALITY_LOW,
    VIDEO_QUALITY_MID,
    VIDEO_QUALITY_HIGH,
} video_quality_e;

typedef float (*func_get_cpu_temp)(void);
typedef float (*func_get_battery_voltage)(void);
typedef int (*func_get_video_format)(video_format_e *format, video_size_e *size, video_quality_e *quality);
typedef int (*func_set_video_format)(video_format_e format, video_size_e size, video_quality_e quality);

typedef struct {
    func_get_cpu_temp get_cpu_temp;
    func_get_battery_voltage get_battery_voltage;
    func_get_video_format get_video_format;
    func_set_video_format set_video_format;
} cam_gui_init_param;

typedef enum {
    SD_CARD_NOT_DETECT,
    SD_CARD_UNKNOWN_FS,
    SD_CARD_OK,
} sd_card_state_e;

int32_t cam_gui_init(cam_gui_init_param *param);

void other_gui_job();

void set_sd_card_icon(sd_card_state_e state);
void refresh_rec_time(uint64_t time_ms);
void set_rec_icon_start();
void set_rec_icon_stop();
void enable_gui_btn();
void disable_gui_btn();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
