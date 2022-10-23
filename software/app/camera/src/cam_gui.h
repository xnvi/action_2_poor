#ifndef _CAM_GUI_H
#define _CAM_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "camera_config.h"

uint8_t sg_pv_img_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT * LV_COLOR_SIZE / 8];
lv_obj_t *video_preview_obj;

void cam_gui_init();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
