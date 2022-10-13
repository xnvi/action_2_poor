#ifndef __LV_STB_TRUETYPE_H
#define __LV_STB_TRUETYPE_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include <stdint.h>
#include "lvgl.h"

int lv_stbtt_init(lv_font_t *lv_font, float size);
void lv_stbtt_exit(lv_font_t *lv_font);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __LV_STB_TRUETYPE_H