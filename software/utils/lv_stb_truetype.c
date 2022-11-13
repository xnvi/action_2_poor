/*
适配 stb_truetype 作为 lvgl 的字体引擎
目前中英文采用相互独立的字体，支持多种字号

我前期开发用的 LVGL v8.3，这个版本官方还没适配 stb_truetype
但目前 LVGL 的 master 分支上官方已经适配好了，叫做 Tiny TTF font engine
我在本项目中就用自己适配的字体引擎，以后除非UI重构我再使用 Tiny TTF font engine
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvgl.h"
#include "lv_stb_truetype.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define EN_FONT_PATH "./font/Inconsolata-Bold.ttf"
#define ZH_FONT_PATH "./font/HarmonyOS_Sans_SC_Regular_L1.ttf"
// 中英文字体文件的静态内存，根据对应字体文件的实际大小设置
#define EN_FONT_SIZE (96 * 1024)
#define ZH_FONT_SIZE (928* 1024)

#define MAX_FONT_SIZE 64

static uint8_t en_font_data[EN_FONT_SIZE] = {0};
static uint8_t zh_font_data[ZH_FONT_SIZE] = {0};

static stbtt_fontinfo en_font;
static stbtt_fontinfo zh_font;
static uint8_t font_bitmap[MAX_FONT_SIZE * MAX_FONT_SIZE] = {0};

typedef struct {
    float font_height; // 字体高度，即字体大小
    float font_scale; // 缩放系数，stb_truetype使用
    int32_t font_size_fix; // 为了优化显示效果摸索出来的一个值，不一定靠谱，看情况改吧
} font_info;

static int32_t _dbg_print_one_char(uint8_t *font, int32_t w, int32_t h)
{
    uint8_t gray[8] = {" .:+xem#"}; // 模拟灰度显示的效果
    int32_t i, j;
    int32_t g;

	putchar('\n');
	for(i = 0; i < h; i++)
	{
		for(j = 0; j < w; j++)
		{
			g = font[i * w + j];
			g = g >> 5;
			putchar(gray[g]);
		}
		putchar('\n');
	}
	putchar('\n');

    return 0;
}

static bool get_glyph_dsc_stbtt(const struct _lv_font_t *font, lv_font_glyph_dsc_t *dsc_out, uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    // 想要更完美的呈现字体效果可能需要用到 unicode_letter_next 来计算下一个字符的间距
    int32_t x0, y0, x1, y1;
    font_info *ft_info = (font_info *)font->user_data;

    // 除tab以外的控制字符按空字符处理
    if (unicode_letter < ' ' && unicode_letter != '\t') {
        dsc_out->adv_w = 0;
        dsc_out->box_w = 0;
		dsc_out->box_h = 0;
		dsc_out->ofs_x = 0;
		dsc_out->ofs_y = 0;
		dsc_out->bpp = 8;
		return 1;
    }

    // 空格和tab都按空格处理
	if (unicode_letter == ' ' || unicode_letter == '\t') {
		dsc_out->adv_w = (ft_info->font_height + ft_info->font_size_fix) / 2;
        dsc_out->box_w = 0;
		dsc_out->box_h = 0;
		dsc_out->ofs_x = 0;
		dsc_out->ofs_y = 0;
		dsc_out->bpp = 8;
		return 1;
	}

	if (unicode_letter < 128) {
    	stbtt_GetCodepointBitmapBox(&en_font, unicode_letter, ft_info->font_scale, ft_info->font_scale, &x0, &y0, &x1, &y1);
		dsc_out->adv_w = (ft_info->font_height + ft_info->font_size_fix) / 2; // 如果字体本身就有边距可以当字符宽度处理
		dsc_out->box_w = (ft_info->font_height + 1) / 2;
	}
	else {
    	stbtt_GetCodepointBitmapBox(&zh_font, unicode_letter, ft_info->font_scale, ft_info->font_scale, &x0, &y0, &x1, &y1);
		dsc_out->adv_w = ft_info->font_height + ft_info->font_size_fix; // 如果字体本身就有边距可以当字符宽度处理
		dsc_out->box_w = ft_info->font_height;
	}

	dsc_out->box_h = y1 - y0;

	// 反复测试下来发现，ofs_x使用x0最好好，ofs_y为-y1，英文的话，y可以适当再加高一点点
	dsc_out->ofs_x = x0;
	dsc_out->ofs_y = -y1;
	dsc_out->bpp = 8;

	// printf("0x%x -> %d %d %d %d\n", unicode_letter, x0, x1, y0, y1);

    return 1; // 0失败，1成功
}

static const uint8_t *get_glyph_bitmap_stbtt(const struct _lv_font_t *font, uint32_t unicode_letter)
{
    int32_t x0, y0, x1, y1;
    font_info *ft_info = (font_info *)font->user_data;

	memset(font_bitmap, 0, sizeof(font_bitmap));

    // 除tab以外的控制字符按空字符处理
    if (unicode_letter < ' ' && unicode_letter != '\t') {
		return font_bitmap;
	}

    // 空格和tab都按空格处理
	if (unicode_letter == ' ' || unicode_letter == '\t') {
		return font_bitmap;
	}

	if (unicode_letter < 128) {
    	stbtt_GetCodepointBitmapBox(&en_font, unicode_letter, ft_info->font_scale, ft_info->font_scale, &x0, &y0, &x1, &y1);
		stbtt_MakeCodepointBitmap(&en_font, font_bitmap, x1 - x0, y1 - y0, (ft_info->font_height + 1) / 2, ft_info->font_scale, ft_info->font_scale, unicode_letter);
		// _dbg_print_one_char(font_bitmap, (font_height + 1) / 2, y1-y0);
	}
	else {
    	stbtt_GetCodepointBitmapBox(&zh_font, unicode_letter, ft_info->font_scale, ft_info->font_scale, &x0, &y0, &x1, &y1);
		stbtt_MakeCodepointBitmap(&zh_font, font_bitmap, x1 - x0, y1 - y0, ft_info->font_height, ft_info->font_scale, ft_info->font_scale, unicode_letter);
		// _dbg_print_one_char(font_bitmap, font_height, y1-y0);
	}

	return font_bitmap;
}

int32_t lv_stbtt_init(lv_font_t *lv_font, float size)
{
    int32_t ret = 0;
    int32_t x0, y0, x1, y1;
    FILE *fpr = NULL;
    font_info *ft_info;

    if (size > MAX_FONT_SIZE) {
        printf("font size(%f) larger than %d\n", size, MAX_FONT_SIZE);
        return 1;
    }

    memset(&en_font, 0, sizeof(stbtt_fontinfo));
    memset(&zh_font, 0, sizeof(stbtt_fontinfo));
    memset(en_font_data, 0, sizeof(en_font_data));
    memset(zh_font_data, 0, sizeof(zh_font_data));

    fpr = fopen(EN_FONT_PATH, "rb");
    if (fpr == NULL) {
        printf("can not open font file(%s)\n", EN_FONT_PATH);
        return 1;
    }
    fread(en_font_data, 1, sizeof(en_font_data), fpr);
    fclose(fpr);
    fpr = NULL;

    fpr = fopen(ZH_FONT_PATH, "rb");
    if (fpr == NULL) {
        printf("can not open font file(%s)\n", ZH_FONT_PATH);
        return 1;
    }
    fread(zh_font_data, 1, sizeof(zh_font_data), fpr);
    fclose(fpr);
    fpr = NULL;

    ret = stbtt_InitFont(&en_font, en_font_data, 0);
    if (ret == 0) {
        printf("init en font err(%d)\n", ret);
        return 1;
    }

    ret = stbtt_InitFont(&zh_font, zh_font_data, 0);
    if (ret == 0) {
        printf("init zh font err(%d)\n", ret);
        return 1;
    }

    ft_info = (font_info *)malloc(sizeof(font_info));
    if (ft_info == NULL) {
        printf("font info malloc err\n");
        return 1;
    }

    ft_info->font_height = size;
    ft_info->font_size_fix = -(ft_info->font_height / 5 - 1); // 为了优化显示效果摸索出来的一个值，不一定靠谱，看情况改吧
    ft_info->font_scale = stbtt_ScaleForPixelHeight(&zh_font, size);

    stbtt_GetFontVMetrics(&zh_font, &y0, &y1, &x0);
    lv_font->line_height = ft_info->font_scale * (y0 - y1 + x0) + ft_info->font_size_fix;
    stbtt_GetFontBoundingBox(&zh_font, &x0, &y0, &x1, &y1);
    lv_font->base_line = ft_info->font_scale * (-y0);

    lv_font->get_glyph_dsc = get_glyph_dsc_stbtt;
    lv_font->get_glyph_bitmap = get_glyph_bitmap_stbtt;
    lv_font->subpx = LV_FONT_SUBPX_NONE;
    // lv_font->underline_position = (int)size;
    lv_font->underline_position = ft_info->font_scale * y0;
    // lv_font->underline_thickness = (int)size / 12 + 1; // 根据字体大小适当调整
    lv_font->underline_thickness = 1; // 固定值
    lv_font->dsc = NULL;
    lv_font->user_data = ft_info;

    return 0;
}

void lv_stbtt_exit(lv_font_t *lv_font)
{
    free(lv_font->user_data);
    memset(lv_font, 0, sizeof(lv_font_t));
}
