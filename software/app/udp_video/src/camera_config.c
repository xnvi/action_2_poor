#include "camera_config.h"

// 海思多媒体系统的相关配置
// 如果修改了分辨率，还需要修改 MPP 内存池相关配置


// 是否启用 VB 内存的附加信息
const HI_U32 g_u32SupplementConfig = HI_FALSE;

// VI相关，此芯片只支持单摄，连接在VI管道0、通道0上
const VI_PIPE g_ViPipe = 0;
const VI_CHN g_ViChn = 0;

// OV5640 的输入图像尺寸，如有变动需配合OV5640驱动一起修改
const PIC_SIZE_E g_viSize = PIC_1080P;

// VPSS组序号，目前只使用 Group 0
const VPSS_GRP g_VpssGrp = 0;

// VPSS 通道和 VENC 通道的绑定关系，即VpssChn[i]绑定到VencChn[i]，以下面为例就是VpssChn4绑定VencChn0
const VPSS_CHN g_VpssChn[ENABLE_VENC_CHN_NUM] = {3};
const VENC_CHN g_VencChn[ENABLE_VENC_CHN_NUM] = {0};

// 要使用的vpss通道，3516EV200是3物理通道+4扩展通道，0到2是物理通道，3到6是扩展通道
const HI_BOOL g_abChnEnable[VPSS_MAX_CHN_NUM] = {HI_TRUE, HI_FALSE, HI_FALSE,
                                         HI_TRUE, HI_TRUE, HI_FALSE, HI_FALSE};

// 各个VPSS通道的输出图像尺寸
const PIC_SIZE_E g_vpssSize[VPSS_MAX_CHN_NUM] = {PIC_1080P, PIC_BUTT, PIC_BUTT, 
                                         PIC_1080P, PIC_PREVIEW, PIC_BUTT, PIC_BUTT};

// 各个VENC通道的输入图像尺寸，必须和上面的VpssChn、VencChn绑定关系一致
const PIC_SIZE_E g_vencSize[ENABLE_VENC_CHN_NUM] = {PIC_1080P};

// 各个VENC通道的编码类型
// PAYLOAD_TYPE_E g_enPayLoad[ENABLE_VENC_CHN_NUM] = {PT_H265};
const PAYLOAD_TYPE_E g_enPayLoad[ENABLE_VENC_CHN_NUM] = {PT_H264};

// 各个VENC通道的编码等级，一般而言对于H264、H265、JPEG都配置为0就行，如有其它需求参考文档进行配置
const HI_U32 g_u32Profile[ENABLE_VENC_CHN_NUM] = {0};


// 我在VPSS做了帧率控制，这个就是VPSS限制后的帧率，也是下级模块的输入帧率
const HI_U32 g_u32DstFrameRate = 30;


// PIC_PREVIEW 视频预览区尺寸，VPSS 缩放并裁剪到这个大小
const HI_U32 preview_u32Width  = 320;
const HI_U32 preview_u32Height = 180;


// 音频输入配置
const HI_S32              g_s32AiChnCnt   = 1;
const HI_S32              g_s32AencChnCnt = 1; // AENC 通道数 = AiChnCnt >> (声道数 - 1)
const AUDIO_DEV           g_AiDev         = HIMPP_AUDIO_INNER_AI_DEV;
const HI_BOOL             g_bAiReSample   = HI_FALSE;
const AI_CHN              g_AiChn         = 0;
const AENC_CHN            g_AeChn         = 0;
const AUDIO_SAMPLE_RATE_E g_AiSamplerate  = AUDIO_SAMPLE_RATE_48000;
const AUDIO_BIT_WIDTH_E   g_AiBitwidth    = AUDIO_BIT_WIDTH_16; // PCM 默认就用16BIT，不要改
const AUDIO_SOUND_MODE_E  g_AiSoundmode   = AUDIO_SOUND_MODE_MONO;

// 音频输出配置
const HI_S32              g_s32AoChnCnt  = 1; // 单声道为1，双声道为2
const AUDIO_DEV           g_AoDev        = HIMPP_AUDIO_INNER_AO_DEV;
const HI_BOOL             g_bAoReSample  = HI_FALSE;
const AI_CHN              g_AoChn        = 0;
const AENC_CHN            g_AdChn        = 0;
const AUDIO_SAMPLE_RATE_E g_AoSamplerate = AUDIO_SAMPLE_RATE_48000;
const AUDIO_BIT_WIDTH_E   g_AoBitwidth   = AUDIO_BIT_WIDTH_16; // PCM 默认就用16BIT，不要改
const AUDIO_SOUND_MODE_E  g_AoSoundmode  = AUDIO_SOUND_MODE_MONO;


// 调试选项
// TODO
