#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/select.h>

#include "media.h"
#include "color_convert.h"
#include "audio_aac_adp.h"
#include "camera_base_config.h"
#include "camera_himpp_config.h"
#include "himm.h"
#include "ts_mux.h"
#include "himpp.h"

// 海思头文件
#include "hi_ive.h"
#include "mpi_ive.h"

typedef struct {
    pthread_t thread;
    char running;
    char exit;
} pthread_manger_t;

static pthread_manger_t sg_pth_get_audio = {0};

typedef struct {
    VPSS_CHN VpssChn;
    VIDEO_FRAME_INFO_S pstVideoFrame;
    IVE_HANDLE ive_handle;
    IVE_SRC_IMAGE_S src_img;
    IVE_DST_IMAGE_S dst_img;
    IVE_CSC_CTRL_S csc_ctrl;
    uint8_t *yuv_buf;
    uint8_t *rgb888_buf;
} perview_img_ctx;

typedef struct {
    pthread_mutex_t lock;
    hisi_sdk_state_e hisi_state;
    rec_state_e rec_state;
    uint64_t rec_start_ms;

    uint8_t *vbuf; // 用来将编码器输出的数据拼成一个完整视频帧
    void *ts_handle;
    char dir_path[64];
    char filename[64];
    perview_img_ctx pv_ctx;
} media_context_t;
static media_context_t sg_media_ctx = {0};

extern void OV5640_init(VI_PIPE ViPipe);


int32_t rec_set_filename(char *filename)
{
    if (strlen(filename) > sizeof(sg_media_ctx.filename) - 1) {
        return 1;
    }
    pthread_mutex_lock(&sg_media_ctx.lock);
    memset(sg_media_ctx.filename, 0, sizeof(sg_media_ctx.filename));
    strcpy(sg_media_ctx.filename, filename);
    pthread_mutex_unlock(&sg_media_ctx.lock);
    return 0;
}

void rec_set_state(rec_state_e state)
{
    char full_name[128];
    struct timeval time_base;
    uint64_t time_ms;
    pthread_mutex_lock(&sg_media_ctx.lock);
    sg_media_ctx.rec_state = state;
    pthread_mutex_unlock(&sg_media_ctx.lock);

    if (sg_media_ctx.rec_state == REC_STATE_DO_START) {
        memset(full_name, 0, sizeof(full_name));
        strcat(full_name, sg_media_ctx.dir_path);
        strcat(full_name, sg_media_ctx.filename);
        strcat(full_name, ".ts");
        // 读取配置，设置编码格式，应该读取实际编码器的设置，暂且这样写吧
        if (g_enPayLoad[0] == PT_H264) {
            ts_open_file(sg_media_ctx.ts_handle, '4', full_name);
        }
        else if (g_enPayLoad[0] == PT_H265) {
            ts_open_file(sg_media_ctx.ts_handle, '5', full_name);
        }

        gettimeofday(&time_base, NULL);
        time_ms = time_base.tv_sec * 1000 + time_base.tv_usec / 1000;

        pthread_mutex_lock(&sg_media_ctx.lock);
        sg_media_ctx.rec_start_ms = time_ms;
        sg_media_ctx.rec_state = REC_STATE_START_OK;
        pthread_mutex_unlock(&sg_media_ctx.lock);
    }

    if (sg_media_ctx.rec_state == REC_STATE_DO_STOP) {
        ts_close_file(sg_media_ctx.ts_handle);

        pthread_mutex_lock(&sg_media_ctx.lock);
        sg_media_ctx.rec_state = REC_STATE_STOP_OK;
        pthread_mutex_unlock(&sg_media_ctx.lock);
    }
}

rec_state_e rec_get_state(void)
{
    return sg_media_ctx.rec_state;
}

uint64_t rec_get_start_time_ms(void)
{
    return sg_media_ctx.rec_start_ms;
}

// 用于从编码器获取数据
HI_S32 get_hisi_video_frame(VENC_CHN VencChn, VENC_STREAM_S* pstStream)
{
    int32_t is_i_frame = 0;
    int32_t frame_size = 0;
    int32_t offset = 0;
    int32_t i = 0;
    int32_t ret = 0;

    if (VencChn != 0) {
        // 目前只使用了1路编码器
        return HI_SUCCESS;
    }

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        int pack_size = pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset;
        frame_size += pack_size;
    }

    if (frame_size > g_u32SuperFrameSize) {
        printf("frame size is %d, larger than buffer size %d\n", frame_size, g_u32SuperFrameSize);
        return HI_FAILURE;
    }

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        if (g_enPayLoad[i] == PT_H264) {
            if (pstStream->pstPack[i].DataType.enH264EType >= H264E_NALU_IDRSLICE) {
                is_i_frame = 1;
            }
        }
        else if (g_enPayLoad[i] == PT_H265) {
            if (pstStream->pstPack[i].DataType.enH265EType >= H265E_NALU_IDRSLICE) {
                is_i_frame = 1;
            }
        }

        int pack_size = pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset;
        uint8_t *addr = pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset;

        memcpy(&sg_media_ctx.vbuf[offset], addr, pack_size);
        offset += pack_size;
    }

    ret = ts_write_file(sg_media_ctx.ts_handle, sg_media_ctx.vbuf, frame_size, 
                        pstStream->pstPack[0].u64PTS / 1000, 'v', is_i_frame);
    if (ret) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int32_t get_preview_img(uint8_t *buf)
{
    HI_S32 s32Ret;
    YCbCrType yuv_type = YCBCR_709;
    HI_U8* yuv_addr[3];
    perview_img_ctx *pv_ctx = &sg_media_ctx.pv_ctx;

    pthread_mutex_lock(&sg_media_ctx.lock);
    if (sg_media_ctx.hisi_state != HISI_SDK_INIT_OK) {
        pthread_mutex_unlock(&sg_media_ctx.lock);
        return 1;
    }
    // 通道号和 camera_config 的配置保持一致
    s32Ret = HI_MPI_VPSS_GetChnFrame(g_VpssGrp, pv_ctx->VpssChn,
                                     &pv_ctx->pstVideoFrame, 0);
    if(s32Ret != HI_SUCCESS) {
        pthread_mutex_unlock(&sg_media_ctx.lock);
        return 1;
    }

    if (pv_ctx->pstVideoFrame.stVFrame.enColorGamut == COLOR_GAMUT_BT601) {
        yuv_type = YCBCR_601;
    }
    else if (pv_ctx->pstVideoFrame.stVFrame.enColorGamut == COLOR_GAMUT_BT709) {
        yuv_type = YCBCR_709;
    }
    else {
        printf("ColorGamut[%d] not support\n", pv_ctx->pstVideoFrame.stVFrame.enColorGamut);
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(g_VpssGrp, pv_ctx->VpssChn,
                                             &pv_ctx->pstVideoFrame);
        if(s32Ret != HI_SUCCESS) {
            printf("HI_MPI_VPSS_ReleaseChnFrame fail with %#x\n", s32Ret);
        }
        pthread_mutex_unlock(&sg_media_ctx.lock);
        return 1;
    }

    if (pv_ctx->pstVideoFrame.stVFrame.enPixelFormat != PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        printf("PixelFormat[%d] not support\n", pv_ctx->pstVideoFrame.stVFrame.enPixelFormat);
        s32Ret = HI_MPI_VPSS_ReleaseChnFrame(g_VpssGrp, pv_ctx->VpssChn,
                                             &pv_ctx->pstVideoFrame);
        if(s32Ret != HI_SUCCESS) {
            printf("HI_MPI_VPSS_ReleaseChnFrame fail with %#x\n", s32Ret);
        }
        pthread_mutex_unlock(&sg_media_ctx.lock);
        return 1;
    }

    // HI_MPI_SYS_Mmap、memcpy、HI_MPI_SYS_Munmap这里注意，因为我知道目前的视频宽度是240，y通道的stride也正好是240，
    // 所以这里的大小都直接使用 PREVIEW_WIDTH 和 PREVIEW_HEIGHT 进行计算，如果宽度和stride不相等，
    // 则必须用 pstVideoFrame.stVFrame.u32Stride[] * PREVIEW_HEIGHT 进行计算
    yuv_addr[0] = (HI_U8*) HI_MPI_SYS_Mmap(pv_ctx->pstVideoFrame.stVFrame.u64PhyAddr[0], PREVIEW_WIDTH * PREVIEW_HEIGHT);
    yuv_addr[1] = (HI_U8*) HI_MPI_SYS_Mmap(pv_ctx->pstVideoFrame.stVFrame.u64PhyAddr[1], PREVIEW_WIDTH * PREVIEW_HEIGHT / 2);

    // 其实完全可以直接将yuv_addr[0]和yuv_addr[1]送入 nv21_to_rgb888_soft 函数进行色彩转换
    // nv21_to_rgb888_soft 纯跑分性能测试 240*180 转换一次耗时不足2ms
    // 但是实际测试这样做效率非常低，用户态cpu开销20%，内核态cpu开销8%，非常离谱
    // 推测应该是大量CPU被用于在cache、linux内存空间、海思媒体内存之间进行数据交换
    // 这里做一次内存拷贝后性能基本接近跑分测试的性能
    memcpy(&pv_ctx->yuv_buf[0], yuv_addr[0], PREVIEW_WIDTH * PREVIEW_HEIGHT);
    memcpy(&pv_ctx->yuv_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT], yuv_addr[1], PREVIEW_WIDTH * PREVIEW_HEIGHT / 2);

    HI_MPI_SYS_Munmap(yuv_addr[0], PREVIEW_WIDTH * PREVIEW_HEIGHT);
    HI_MPI_SYS_Munmap(yuv_addr[1], PREVIEW_WIDTH * PREVIEW_HEIGHT / 2);


    s32Ret = HI_MPI_VPSS_ReleaseChnFrame(g_VpssGrp, pv_ctx->VpssChn, &pv_ctx->pstVideoFrame);
    pthread_mutex_unlock(&sg_media_ctx.lock);
    if(s32Ret != HI_SUCCESS) {
        printf("HI_MPI_IVE_CSC fail with %#x\n", s32Ret);
        return 1;
    }

    nv21_to_rgb888_soft(pv_ctx->pstVideoFrame.stVFrame.u32Width, pv_ctx->pstVideoFrame.stVFrame.u32Height,
                        &pv_ctx->yuv_buf[0], &pv_ctx->yuv_buf[PREVIEW_WIDTH * PREVIEW_HEIGHT],
                        pv_ctx->pstVideoFrame.stVFrame.u32Stride[0], pv_ctx->pstVideoFrame.stVFrame.u32Stride[1],
                        pv_ctx->rgb888_buf, PREVIEW_WIDTH * 3,
                        yuv_type);

    rgb888_to_rgb565_swap_neon(pv_ctx->rgb888_buf, buf, preview_u32Width, preview_u32Height);
    return 0;
}

static void *pth_get_audio()
{
    HI_S32 s32Ret;
    HI_S32 AencFd;
    AUDIO_STREAM_S stStream;
    fd_set read_fds;
    struct timeval TimeoutVal;

    prctl(PR_SET_NAME, "get_audio_data");

    FD_ZERO(&read_fds);
    AencFd = HI_MPI_AENC_GetFd(g_AeChn);
    FD_SET(AencFd, &read_fds);

    sg_pth_get_audio.running = 1;
    sg_pth_get_audio.exit = 0;

    while (!sg_pth_get_audio.exit) {
        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(AencFd, &read_fds);

        s32Ret = select(AencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            break;
        }
        else if (0 == s32Ret)
        {
            printf("get aenc stream select time out\n");
            // break;
            continue;
        }

        if (FD_ISSET(AencFd, &read_fds))
        {
            s32Ret = HI_MPI_AENC_GetStream(g_AeChn, &stStream, HI_FALSE);
            if (HI_SUCCESS != s32Ret )
            {
                printf("HI_MPI_AENC_GetStream(%d), failed with %#x!\n", g_AeChn, s32Ret);
                break;
            }

            s32Ret = ts_write_file(sg_media_ctx.ts_handle, stStream.pStream, stStream.u32Len, 
                                   stStream.u64TimeStamp / 1000, 'a', 0);
            if (s32Ret) {
                // printf("write audio error\n");
            }

            s32Ret = HI_MPI_AENC_ReleaseStream(g_AeChn, &stStream);
            if (HI_SUCCESS != s32Ret )
            {
                printf("HI_MPI_AENC_ReleaseStream(%d), failed with %#x!\n", g_AeChn, s32Ret);
                break;
            }
        }

        usleep(10 * 1000);
    }

    sg_pth_get_audio.running = 0;

    return NULL;
}

int32_t ao_send_frame(uint8_t *data, int32_t len, int32_t block)
{
    HI_S32 s32Ret;
    AUDIO_STREAM_S ao_stream = {0};
    ao_stream.pStream = data;
    ao_stream.u32Len = len;
    
    if (g_AoSoundmode == AUDIO_SOUND_MODE_MONO) {
        s32Ret = HI_MPI_ADEC_SendStream(g_AdChn, &ao_stream, block);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ADEC_SendStream(%d), failed with %#x!\n", g_AdChn, s32Ret);
            return 1;
        }
    }

    // 双声道音频丢弃一个声道，目前改成双声道有点麻烦，以后如果有需求再完善
    if (g_AoSoundmode == AUDIO_SOUND_MODE_STEREO) {
        if (len % 2 != 0) {
            printf("stereo audio len must be even number\n");
            return 1;
        }
        if (len > 4096) {
            printf("audio frame len(%d) larger than 4096\n", len);
            return 1;
        }

        int16_t abuf[2048];
        for (int32_t i = 0; i < len / 2; i++)
        {
            abuf[i] = ((int16_t *)data)[i * 2];
        }

        ao_stream.u32Len = len / 2;
        ao_stream.pStream = (uint8_t *)abuf;

        s32Ret = HI_MPI_ADEC_SendStream(g_AdChn, &ao_stream, block);
        if (HI_SUCCESS != s32Ret)
        {
            printf("HI_MPI_ADEC_SendStream(%d), failed with %#x!\n", g_AdChn, s32Ret);
            return 1;
        }
    }

    return 0;
}

static int32_t _ai_init()
{
    HI_S32              i, j, s32Ret;
    // PAYLOAD_TYPE_E      enPayloadType = PT_LPCM;
    PAYLOAD_TYPE_E      enPayloadType = PT_AAC;
    AUDIO_SAMPLE_RATE_E AiReSmp       = AUDIO_SAMPLE_RATE_BUTT;
    AI_CHN      AiChn;
    AENC_CHN    AeChn;
    pthread_attr_t attr;

    AIO_ATTR_S stAiAttr;
    stAiAttr.enSamplerate   = g_AiSamplerate;
    stAiAttr.enBitwidth     = g_AiBitwidth;
    stAiAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAiAttr.enSoundmode    = g_AiSoundmode;
    stAiAttr.u32EXFlag      = 0;
    stAiAttr.u32FrmNum      = 30;
    stAiAttr.u32PtNumPerFrm = 1024;
    stAiAttr.u32ChnCnt      = g_s32AiChnCnt;
    stAiAttr.u32ClkSel      = 0;
    stAiAttr.enI2sType      = AIO_I2STYPE_INNERCODEC;

    AI_RECORDVQE_CONFIG_S stAiVqeAttr;
    memset(&stAiVqeAttr, 0, sizeof(AI_RECORDVQE_CONFIG_S));
    stAiVqeAttr.u32OpenMask = AI_RECORDVQE_MASK_RNR;
    stAiVqeAttr.s32WorkSampleRate = g_AiSamplerate;
    stAiVqeAttr.s32FrameSample = stAiAttr.u32PtNumPerFrm;
    stAiVqeAttr.enWorkstate = VQE_WORKSTATE_COMMON;
    stAiVqeAttr.s32InChNum = 1;
    stAiVqeAttr.s32OutChNum = 1;
    stAiVqeAttr.enRecordType = VQE_RECORD_NORMAL;
    stAiVqeAttr.stHpfCfg.bUsrMode = HI_FALSE;
    stAiVqeAttr.stRnrCfg.bUsrMode = HI_FALSE;
    stAiVqeAttr.stHdrCfg.bUsrMode = HI_FALSE;
    stAiVqeAttr.stDrcCfg.bUsrMode = HI_FALSE;
    // stAiVqeAttr.stEqCfg.s8GaindB = {0};
    stAiVqeAttr.stAgcCfg.bUsrMode = HI_FALSE;

    HI_MPI_AENC_AacInit();

    s32Ret = HIMPP_AUDIO_StartAi(g_AiDev, g_s32AiChnCnt, &stAiAttr, AiReSmp, g_bAiReSample, &stAiVqeAttr, 1, -1);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StartAi failed with %#x!\n", s32Ret);
        goto AIAENC_ERR4;
    }

    s32Ret = HIMPP_AUDIO_CfgAcodec(&stAiAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_CfgAcodec failed with %#x!\n", s32Ret);
        goto AIAENC_ERR3;
    }

    s32Ret = HIMPP_AUDIO_StartAenc(g_s32AencChnCnt, &stAiAttr, enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StartAenc failed with %#x!\n", s32Ret);
        goto AIAENC_ERR2;
    }

    for (i = 0; i < g_s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        s32Ret = HIMPP_AUDIO_AencBindAi(g_AiDev, AiChn, AeChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("AiDev[%d] AiChn[%d] bind to AeChn[%d] failed with %#x!\n",
                    g_AiDev, AiChn, AeChn, s32Ret);
            for (j=0; j<i; j++)
            {
                HIMPP_AUDIO_AencUnbindAi(g_AiDev, j, j);
            }
            goto AIAENC_ERR1;
        }
        printf("Ai(%d,%d) bind to AencChn:%d ok!\n", g_AiDev, AiChn, AeChn);
    }

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 128 * 1024);
    pthread_create(&sg_pth_get_audio.thread, &attr, &pth_get_audio, NULL);
    pthread_attr_destroy(&attr);

    return 0;

AIAENC_ERR1:
    for (i = 0; i < g_s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        s32Ret = HIMPP_AUDIO_AencUnbindAi(g_AiDev, AiChn, AeChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HIMPP_AUDIO_AencUnbindAi failed with %#x!\n", s32Ret);
        }
    }

AIAENC_ERR2:
    s32Ret = HIMPP_AUDIO_StopAenc(g_s32AencChnCnt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAenc failed with %#x!\n", s32Ret);
    }

AIAENC_ERR3:
    s32Ret = HIMPP_AUDIO_StopAi(g_AiDev, g_s32AiChnCnt, g_bAiReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAi failed with %#x!\n", s32Ret);
    }

AIAENC_ERR4:
    HI_MPI_AENC_AacDeInit();
    return s32Ret;
}

static int32_t _ai_exit()
{
    HI_S32   i, s32Ret;
    AI_CHN   AiChn;
    AENC_CHN AeChn;

    sg_pth_get_audio.exit = 1;
    while(sg_pth_get_audio.running) {usleep(10 * 1000);}

    for (i = 0; i < g_s32AencChnCnt; i++)
    {
        AeChn = i;
        AiChn = i;

        s32Ret = HIMPP_AUDIO_AencUnbindAi(g_AiDev, AiChn, AeChn);
        if (s32Ret != HI_SUCCESS)
        {
            printf("HIMPP_AUDIO_AencUnbindAi failed with %#x!\n", s32Ret);
        }
    }

    s32Ret = HIMPP_AUDIO_StopAenc(g_s32AencChnCnt);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAenc failed with %#x!\n", s32Ret);
    }

    s32Ret = HIMPP_AUDIO_StopAi(g_AiDev, g_s32AiChnCnt, g_bAiReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAi failed with %#x!\n", s32Ret);
    }

    HI_MPI_AENC_AacDeInit();

    return 0;
}

static int32_t _ao_init()
{
    HI_S32              s32Ret;
    PAYLOAD_TYPE_E      enPayloadType = PT_LPCM;
    AUDIO_SAMPLE_RATE_E AoReSmp       = AUDIO_SAMPLE_RATE_BUTT;
    AUDIO_DEV AoDev = 0;
    HI_S32 s32Volume = 0;

    AIO_ATTR_S stAoAttr;
    stAoAttr.enSamplerate   = g_AoSamplerate;
    stAoAttr.enBitwidth     = g_AoBitwidth;
    stAoAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAoAttr.enSoundmode    = g_AoSoundmode;
    stAoAttr.u32EXFlag      = 0;
    stAoAttr.u32FrmNum      = 30;
    stAoAttr.u32PtNumPerFrm = 1024;
    stAoAttr.u32ChnCnt      = g_s32AoChnCnt;
    stAoAttr.u32ClkSel      = 0;
    stAoAttr.enI2sType      = AIO_I2STYPE_INNERCODEC;

    s32Ret = HIMPP_AUDIO_StartAdec(g_AdChn, enPayloadType);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StartAdec failed with %#x!\n", s32Ret);
        goto ADECAO_ERR4;
    }

    s32Ret = HIMPP_AUDIO_StartAo(g_AoDev, g_s32AoChnCnt, &stAoAttr, AoReSmp, g_bAoReSample);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StartAo failed with %#x!\n", s32Ret);
        goto ADECAO_ERR3;
    }

    s32Ret = HIMPP_AUDIO_CfgAcodec(&stAoAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_CfgAcodec failed with %#x!\n", s32Ret);
        goto ADECAO_ERR2;
    }

    s32Ret = HIMPP_AUDIO_AoBindAdec(g_AoDev, g_AoChn, g_AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_AoBindAdec failed with %#x!\n", s32Ret);
        goto ADECAO_ERR1;
    }

    // 音量按实际设置，单位db，取值范围[-121,6]，默认0就可以
    s32Ret = HI_MPI_AO_SetVolume(AoDev, s32Volume);
    if (HI_SUCCESS != s32Ret)
    {
        printf("HI_MPI_AO_SetVolume(%d), failed with %#x!\n", AoDev, s32Ret);
    }

    return 0;

ADECAO_ERR1:
    s32Ret = HIMPP_AUDIO_AoUnbindAdec(g_AoDev, g_AoChn, g_AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_AoUnbindAdec failed with %#x!\n", s32Ret);
    }

ADECAO_ERR2:
    s32Ret |= HIMPP_AUDIO_StopAo(g_AoDev, g_s32AoChnCnt, g_bAoReSample);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAo failed with %#x!\n", s32Ret);
    }

ADECAO_ERR3:
    s32Ret |= HIMPP_AUDIO_StopAdec(g_AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAdec failed with %#x!\n", s32Ret);
    }

ADECAO_ERR4:
    return s32Ret;
}

static int32_t _ao_exit()
{
    HI_S32 s32Ret;

    s32Ret = HIMPP_AUDIO_AoUnbindAdec(g_AoDev, g_AoChn, g_AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_AoUnbindAdec failed with %#x!\n", s32Ret);
    }

    s32Ret |= HIMPP_AUDIO_StopAo(g_AoDev, g_s32AoChnCnt, g_bAoReSample);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAo failed with %#x!\n", s32Ret);
    }

    s32Ret |= HIMPP_AUDIO_StopAdec(g_AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        printf("HIMPP_AUDIO_StopAdec failed with %#x!\n", s32Ret);
    }

    return 0;
}

HI_S32 HIMPP_MPP_SYS_Init()
{
    HI_S32 s32Ret;
    HI_S32 s32PoolCnt;
    HI_U64 u64BlkSize;
    VB_CONFIG_S stVbConf;
    SIZE_S picSize;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
    s32PoolCnt = 0;

#if 0
    // VI 输入
    s32Ret = HIMPP_SYS_GetPicSize(g_viSize, &picSize);
    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt = 3;
    s32PoolCnt += 1;


    // VPSS 输入
    s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[0], &picSize);
    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 2;
    s32PoolCnt += 1;


    // VPSS输出
    s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[3], &picSize);
    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 2;
    s32PoolCnt += 1;
#endif

    // VI VPSS 共用缓存
    s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[0], &picSize);
    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 5;
    s32PoolCnt += 1;

    // 做缩略图显存
    u64BlkSize = COMMON_GetPicBufferSize(preview_u32Width, preview_u32Height, PIXEL_FORMAT_RGB_888, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 2;
    s32PoolCnt += 1;


    stVbConf.u32MaxPoolCnt = s32PoolCnt;

    if (0 == g_u32SupplementConfig) {
        s32Ret = HIMPP_SYS_Init(&stVbConf);
    }
    else {
        s32Ret = HIMPP_SYS_InitWithVbSupplement(&stVbConf, g_u32SupplementConfig);
    }

    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

// 这个函数所有初始化失败的地方都没有做处理
int32_t hisi_media_init(char *path)
{    
    int32_t i;

    pthread_mutex_lock(&sg_media_ctx.lock);
    sg_media_ctx.hisi_state = HISI_SDK_DO_INIT;
    pthread_mutex_unlock(&sg_media_ctx.lock);

    // 初始化媒体系统、内存池
    HIMPP_MPP_SYS_Init();
    // 初始化 VI,VPSS,VENC
    HIMPP_VENC_H265_H264();
    // HIMPP_VENC_H265_H264 内会改变一些寄存器，OV5640初始化只能放它后面
    // VICAP 寄存器，自己摸索出来的配置，手册中说明3516EV200不支持MIPI输入YUV，瞎调给调通了
    himm(0x11001010, 0xFF000000);
    himm(0x11001014, 0xFF000000);
    himm(0x11001040, 0x80000004);

    OV5640_init(0);
    HI_S32 s32ChnNum = 0;
    VENC_CHN VencChn[ENABLE_VENC_CHN_NUM] = {0};
    for (i = 0; i < ENABLE_VENC_CHN_NUM; i++) {
        if (g_enPayLoad[i] == PT_H264 || g_enPayLoad[i] == PT_H265) {
            VencChn[i] = g_VencChn[i];
            s32ChnNum += 1;
        }
    }

    if (HI_SUCCESS != HIMPP_VENC_StartGetStream(VencChn, s32ChnNum)) {
        HIMPP_PRT("Start Venc failed!\n");
        return 1;
    }

    sg_media_ctx.pv_ctx.VpssChn = -1;
    for (i = 0; i < VPSS_MAX_CHN_NUM; i++) {
        if (g_vpssSize[i] == PIC_PREVIEW) {
            sg_media_ctx.pv_ctx.VpssChn = i;
        }
    }

    // 开启音频
    _ai_init();
    _ao_init();

    pthread_mutex_lock(&sg_media_ctx.lock);
    sg_media_ctx.hisi_state = HISI_SDK_INIT_OK;
    pthread_mutex_unlock(&sg_media_ctx.lock);

    return 0;
}

int32_t hisi_media_exit()
{
    // HI_S32 s32Ret;

    pthread_mutex_lock(&sg_media_ctx.lock);
    sg_media_ctx.hisi_state = HISI_SDK_DO_EXIT;
    pthread_mutex_unlock(&sg_media_ctx.lock);

    // 关闭音频
    _ao_exit();
    _ai_exit();

    // 停止取流
    HIMPP_VENC_StopGetStream();

    // 关闭 VI,VPSS,VENC
    HIMPP_VENC_H265_H264_stop();

    // 关闭海思媒体系统
    HIMPP_SYS_Exit();

    pthread_mutex_lock(&sg_media_ctx.lock);
    sg_media_ctx.hisi_state = HISI_SDK_EXIT_OK;
    pthread_mutex_unlock(&sg_media_ctx.lock);

    return 0;
}


int32_t cam_media_init(char *path)
{
    memset(&sg_media_ctx, 0, sizeof(media_context_t));

    pthread_mutex_init(&sg_media_ctx.lock, NULL);

    if (strlen(path) > sizeof(sg_media_ctx.dir_path) - 1) {
        return 1;
    }
    strcpy(sg_media_ctx.dir_path, path);

    sg_media_ctx.vbuf = (uint8_t *)malloc(g_u32SuperFrameSize);
    if (sg_media_ctx.vbuf == NULL) {
        return 1;
    }
    sg_media_ctx.pv_ctx.yuv_buf = (uint8_t *)malloc(PREVIEW_WIDTH * PREVIEW_HEIGHT * 3 / 2);
    if (sg_media_ctx.pv_ctx.yuv_buf == NULL) {
        return 1;
    }
    sg_media_ctx.pv_ctx.rgb888_buf = (uint8_t *)malloc(PREVIEW_WIDTH * PREVIEW_HEIGHT * 3);
    if (sg_media_ctx.pv_ctx.rgb888_buf == NULL) {
        return 1;
    }
    ts_err_code ret = ts_init(&sg_media_ctx.ts_handle);
    if (ret != TS_OK) {
        return 1;
    }

    return 0;
}

int32_t cam_media_exit()
{
    if (sg_media_ctx.vbuf) {
        free(sg_media_ctx.vbuf);
    }
    if (sg_media_ctx.pv_ctx.yuv_buf) {
        free(sg_media_ctx.pv_ctx.yuv_buf);
    }
    if (sg_media_ctx.pv_ctx.rgb888_buf) {
        free(sg_media_ctx.pv_ctx.rgb888_buf);
    }
    if (sg_media_ctx.ts_handle) {
        ts_exit(&sg_media_ctx.ts_handle);
    }
    pthread_mutex_destroy(&sg_media_ctx.lock);

    memset(&sg_media_ctx, 0, sizeof(media_context_t));
    return 0;
}
