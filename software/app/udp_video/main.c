#include <signal.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "socket_udp.h"
#include "himm.h"
#include "himpp.h"
#include "camera_config.h"
#include "media_buf.h"
#include "audio_aac_adp.h"

// 目前只能测一个，不能同时启用
#define DEBUG_AUDIO 1
#define DEBUG_VIDEO 0


extern void OV5640_init(VI_PIPE ViPipe);


// UDP 相关配置
#define DEF_IP_ADDR "192.168.1.106"
int32_t cfg_port = 34543;
char *cfg_ip_addr = DEF_IP_ADDR;

uintptr_t g_udp_fd;



#define MAX_VIDEO_FRAME_SIZE (1024 * 512)

typedef struct _vframe_data_s {
    uint32_t is_i_frame;
    uint32_t frame_size;
    uint32_t frame_id; // 帧序号
    uint64_t pts_ms;
    uint8_t frame_data[1];
} vframe_data_s;

mbuf_handle *g_vframe_buf;



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
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt = 3;
	s32PoolCnt += 1;


	// VPSS 输入
	s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[0], &picSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_422, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 2;
	s32PoolCnt += 1;


    // VPSS输出
	s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[3], &picSize);
    if (HI_SUCCESS != s32Ret)
    {
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
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }
    u64BlkSize = COMMON_GetPicBufferSize(picSize.u32Width, picSize.u32Height, PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE,DEFAULT_ALIGN);
    stVbConf.astCommPool[s32PoolCnt].u64BlkSize   = u64BlkSize;
    stVbConf.astCommPool[s32PoolCnt].u32BlkCnt    = 5;
	s32PoolCnt += 1;


    stVbConf.u32MaxPoolCnt = s32PoolCnt;

    if (0 == g_u32SupplementConfig)
    {
        s32Ret = HIMPP_SYS_Init(&stVbConf);
    }
    else
    {
        s32Ret = HIMPP_SYS_InitWithVbSupplement(&stVbConf, g_u32SupplementConfig);
    }

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}


// 用于从编码器获取数据
HI_S32 get_hisi_video_frame(VENC_CHN VencChn, VENC_STREAM_S* pstStream)
{
    int32_t frame_size = 0;
    int32_t offset = 0;
    int32_t i = 0;

    for (i = 0; i < pstStream->u32PackCount; i++)
    {
        int pack_size = pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset;
        frame_size += pack_size;
    }

    if (frame_size > MAX_VIDEO_FRAME_SIZE) {
        printf("frame size is %d, larger than buffer size %d\n", frame_size, MAX_VIDEO_FRAME_SIZE);
        return HI_SUCCESS;
    }

    if (mbuf_is_full(g_vframe_buf)) {
        mbuf_free(g_vframe_buf);
    }

    vframe_data_s *vdata;
    vdata = (vframe_data_s *)mbuf_write_no_cp(g_vframe_buf, frame_size + offsetof(struct _vframe_data_s, frame_data));
    if (vdata) {
        vdata->is_i_frame = 0;
        vdata->frame_size = frame_size;
        vdata->frame_id = pstStream->u32Seq;
        vdata->pts_ms = pstStream->pstPack->u64PTS / 1000;
        for (i = 0; i < pstStream->u32PackCount; i++)
        {
            // if (g_enPayLoad[i] == PT_H264) {
                if (pstStream->pstPack[i].DataType.enH264EType >= H264E_NALU_IDRSLICE) {
                    vdata->is_i_frame = 1;
                }
            // }
            // else if (g_enPayLoad[i] == PT_H265) {
            //     if (pstStream->pstPack[i].DataType.enH265EType >= H265E_NALU_IDRSLICE) {
            //         vdata->is_i_frame = 1;
            //     }
            // }

            int pack_size = pstStream->pstPack[i].u32Len - pstStream->pstPack[i].u32Offset;
            uint8_t *addr = pstStream->pstPack[i].pu8Addr + pstStream->pstPack[i].u32Offset;
#if DBG_DUMP_HISI_RAW_VIDEO_DATA
        if (VencChn == DUMP_VENC_CHN) {
            fwrite(addr, 1, pack_size, fpw);
        }
#endif
            memcpy(&vdata->frame_data[offset], addr, pack_size);
            offset += pack_size;
        }
        mbuf_write_end_no_cp(g_vframe_buf);
    }

    return HI_SUCCESS;
}


int32_t ai_init()
{
    HI_S32              i, j, s32Ret;
    // PAYLOAD_TYPE_E      enPayloadType = PT_LPCM;
    PAYLOAD_TYPE_E      enPayloadType = PT_AAC;
    AUDIO_SAMPLE_RATE_E AiReSmp       = AUDIO_SAMPLE_RATE_BUTT;
	AI_CHN      AiChn;
    AENC_CHN    AeChn;

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

    HI_MPI_AENC_AacInit();

    s32Ret = HIMPP_AUDIO_StartAi(g_AiDev, g_s32AiChnCnt, &stAiAttr, AiReSmp, g_bAiReSample, NULL, 0, -1);
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

int32_t ai_exit()
{
    HI_S32   i, s32Ret;
    AI_CHN   AiChn;
    AENC_CHN AeChn;

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


int main()
{
    int i;

    process_exit = 0;
    signal(SIGTERM, main_exit);
    signal(SIGINT, main_exit);

    g_udp_fd = socket_udp_open(cfg_ip_addr, (uint16_t)cfg_port);
    if (g_udp_fd <= 0) {
        printf("create udp connect(%s:%d) error\n", cfg_ip_addr, cfg_port);
        return 0;
    }

#if DEBUG_VIDEO
	g_vframe_buf = mbuf_init_dyn(MAX_VIDEO_FRAME_SIZE + offsetof(struct _vframe_data_s, frame_data), 3);
    if (g_vframe_buf == NULL) {
        printf("create video buffer error\n");
        return 1;
    }
#endif

	memopen();
	set_hi_reg();
	usleep(100 * 1000);

    // 初始化媒体系统、内存池
    HIMPP_MPP_SYS_Init();

#if DEBUG_AUDIO
    if (ai_init()) {
        printf("start audio error\n");
        goto end;
    }

    HI_S32 s32Ret;
    HI_S32 AencFd;
    AUDIO_STREAM_S stStream;
    fd_set read_fds;
    struct timeval TimeoutVal;

    FD_ZERO(&read_fds);
    AencFd = HI_MPI_AENC_GetFd(g_AeChn);
    FD_SET(AencFd, &read_fds);
#endif

#if DEBUG_VIDEO
    // 初始化 VI,VPSS,VENC
    HIMPP_VENC_H265_H264();

	// hisi_media_init 内会改变一些寄存器，OV5640初始化只能放它后面
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
        goto end;
    }
#endif

	while (!process_exit) {
#if DEBUG_AUDIO
        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(AencFd, &read_fds);

        s32Ret = select(AencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) {
            break;
        }
        else if (0 == s32Ret) {
            printf("get aenc stream select time out\n");
            continue;
        }

        if (FD_ISSET(AencFd, &read_fds))
        {
            s32Ret = HI_MPI_AENC_GetStream(g_AeChn, &stStream, HI_FALSE);
            if (HI_SUCCESS != s32Ret ) {
                printf("HI_MPI_AENC_GetStream(%d), failed with %#x!\n", g_AeChn, s32Ret);
                break;
            }

            socket_udp_write(g_udp_fd, stStream.pStream, stStream.u32Len);

            s32Ret = HI_MPI_AENC_ReleaseStream(g_AeChn, &stStream);
            if (HI_SUCCESS != s32Ret ) {
                printf("HI_MPI_AENC_ReleaseStream(%d), failed with %#x!\n", g_AeChn, s32Ret);
                break;
            }
        }
#endif

#if DEBUG_VIDEO
		// 发送h264裸数据
		uint32_t vdata_len = 0;
		vframe_data_s *vdata = (vframe_data_s *)mbuf_read_no_cp(g_vframe_buf, &vdata_len);
        if (vdata) {
			socket_udp_write(g_udp_fd, vdata->frame_data, vdata->frame_size);
			mbuf_read_end_no_cp(g_vframe_buf);
        }
#endif
        usleep(10 * 1000);
    }

end:

#if DEBUG_AUDIO
    ai_exit();
#endif

#if DEBUG_VIDEO
    // 停止读取视频数据
    HIMPP_VENC_StopGetStream();

    // 关闭 VI,VPSS,VENC
    HIMPP_VENC_H265_H264_stop();
#endif

    // 关闭海思媒体系统
    HIMPP_SYS_Exit();

#if DEBUG_VIDEO
    if (g_vframe_buf) {
        if(mbuf_destroy_dyn(g_vframe_buf)) {
            printf("destroy video buffer error\n");
        }
    }
#endif

    socket_udp_close(g_udp_fd);
	memclose();

    return 0;
}
