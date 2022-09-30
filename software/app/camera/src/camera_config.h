#ifndef __HI_MPP_CONFIG_H
#define __HI_MPP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "himpp.h"

// #define LCD_SCREEN_WIDTH  (240)
// #define LCD_SCREEN_HEIGHT (240)

#define PREVIEW_WIDTH  (240)
#define PREVIEW_HEIGHT (180)

// 启用的VENC通道总数
#define ENABLE_VENC_CHN_NUM (1)
// 启用的AENC通道总数
#define ENABLE_AENC_CHN_NUM (1)


extern const HI_U32 preview_crop_u32Width;
extern const HI_U32 preview_crop_u32Height;
extern const HI_U32 preview_u32Width;
extern const HI_U32 preview_u32Height;

extern const HI_U32 g_u32SupplementConfig;

extern const HI_U32 g_u32SuperFrameSize;

extern const VI_PIPE    g_ViPipe;
extern const VI_CHN     g_ViChn;
extern const PIC_SIZE_E g_viSize;

extern const VPSS_GRP       g_VpssGrp;
extern const VPSS_CHN       g_VpssChn[];
extern const VENC_CHN       g_VencChn[];
extern const HI_BOOL        g_abChnEnable[];
extern const PIC_SIZE_E     g_vpssSize[];
extern const PIC_SIZE_E     g_vencSize[];
extern const PAYLOAD_TYPE_E g_enPayLoad[];
extern const HI_U32         g_u32Profile[];
extern const HI_U32         g_u32DstFrameRate;

extern const HI_S32              g_s32AiChnCnt;
extern const HI_S32              g_s32AencChnCnt;
extern const AUDIO_DEV           g_AiDev;
extern const HI_BOOL             g_bAiReSample;
extern const AI_CHN              g_AiChn;
extern const AENC_CHN            g_AeChn;
extern const AUDIO_SAMPLE_RATE_E g_AiSamplerate;
extern const AUDIO_BIT_WIDTH_E   g_AiBitwidth;
extern const AUDIO_SOUND_MODE_E  g_AiSoundmode;

extern const HI_S32              g_s32AoChnCnt;
extern const AUDIO_DEV           g_AoDev;
extern const HI_BOOL             g_bAoReSample;
extern const AI_CHN              g_AoChn;
extern const AENC_CHN            g_AdChn;
extern const AUDIO_SAMPLE_RATE_E g_AoSamplerate;
extern const AUDIO_BIT_WIDTH_E   g_AoBitwidth;
extern const AUDIO_SOUND_MODE_E  g_AoSoundmode;

// 调试选项
// TODO

#ifdef __cplusplus
}
#endif

#endif
