/******************************************************************************

  Copyright (C), 2017, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sample_venc.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2017
  Description   :
******************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "himpp.h"
#include "camera_config.h"

#define BIG_STREAM_SIZE     PIC_2688x1944
#define SMALL_STREAM_SIZE   PIC_VGA


#define VB_MAX_NUM            10
#define ONLINE_LIMIT_WIDTH    2304

#define WRAP_BUF_LINE_EXT     416


typedef struct hiHIMPP_VPSS_ATTR_S
{
    SIZE_S            stMaxSize;
    DYNAMIC_RANGE_E   enDynamicRange;
    PIXEL_FORMAT_E    enPixelFormat;
    COMPRESS_MODE_E   enCompressMode[VPSS_MAX_CHN_NUM];
    SIZE_S            stOutPutSize[VPSS_MAX_CHN_NUM];
    FRAME_RATE_CTRL_S stFrameRate[VPSS_MAX_CHN_NUM];
    HI_BOOL           bMirror[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL           bFlip[VPSS_MAX_PHY_CHN_NUM];
    HI_BOOL           bChnEnable[VPSS_MAX_CHN_NUM];

    HIMPP_SNS_TYPE_E  enSnsType;
	PIC_SIZE_E        enSnsSize;
    HI_U32            BigStreamId;
    HI_U32            SmallStreamId;
    VI_VPSS_MODE_E    ViVpssMode; // my edit 目前改为手动设置，这个参数基本用不到
    HI_BOOL           bWrapEn;
    HI_U32            WrapBufLine;
} HIMPP_VPSS_CHN_ATTR_S;

typedef struct hiHIMPP_VB_ATTR_S
{
    HI_U32            validNum;
    HI_U64            blkSize[VB_MAX_NUM];
    HI_U32            blkCnt[VB_MAX_NUM];
    HI_U32            supplementConfig;
} HIMPP_VB_ATTR_S;


/******************************************************************************
* function : show usage
******************************************************************************/
void HIMPP_VENC_Usage(char* sPrgNm)
{
    printf("Usage : %s [index] \n", sPrgNm);
    printf("index:\n");
    printf("\t  0) H265(Large Stream)+H264(Small Stream)+JPEG lowdelay encode with RingBuf.\n");
    printf("\t  1) H.265e + H264e.\n");
    printf("\t  2) Qpmap:H.265e + H264e.\n");
    printf("\t  3) IntraRefresh:H.265e + H264e.\n");
    printf("\t  4) RoiBgFrameRate:H.265e + H.264e.\n");
    printf("\t  5) Mjpeg +Jpeg snap.\n");

    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void HIMPP_VENC_HandleSig(HI_S32 signo)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if (SIGINT == signo || SIGTERM == signo)
    {
        HIMPP_VENC_StopSendQpmapFrame();
        HIMPP_VENC_StopGetStream();
        HIMPP_All_ISP_Stop();
        HIMPP_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : to process abnormal case - the case of stream venc
******************************************************************************/
void HIMPP_VENC_StreamHandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {
        HIMPP_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

VENC_GOP_MODE_E HIMPP_VENC_GetGopMode(void)
{
    char c;
    VENC_GOP_MODE_E enGopMode = 0;

Begin_Get:

    printf("please input choose gop mode!\n");
    printf("\t 0) NORMALP.\n");
    printf("\t 1) DUALP.\n");
    printf("\t 2) SMARTP.\n");

    while((c = getchar()) != '\n' && c != EOF)
    switch(c)
    {
        case '0':
            enGopMode = VENC_GOPMODE_NORMALP;
            break;
        case '1':
            enGopMode = VENC_GOPMODE_DUALP;
            break;
        case '2':
            enGopMode = VENC_GOPMODE_SMARTP;
            break;
        default:
            HIMPP_PRT("input rcmode: %c, is invaild!\n",c);
            goto Begin_Get;
    }

    return enGopMode;
}

HIMPP_RC_E HIMPP_VENC_GetRcMode(void)
{
    char c;
    HIMPP_RC_E  enRcMode = 0;

Begin_Get:

    printf("please input choose rc mode!\n");
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
    printf("\t a) avbr.\n");
    printf("\t x) cvbr.\n");
    printf("\t q) qvbr.\n");
    printf("\t f) fixQp\n");

    while((c = getchar()) != '\n' && c != EOF)
    switch(c)
    {
        case 'c':
            enRcMode = HIMPP_RC_CBR;
            break;
        case 'v':
            enRcMode = HIMPP_RC_VBR;
            break;
        case 'a':
            enRcMode = HIMPP_RC_AVBR;
            break;
        case 'f':
            enRcMode = HIMPP_RC_FIXQP;
            break;
        case 'x':
            enRcMode = HIMPP_RC_CVBR;
            break;
        case 'q':
            enRcMode = HIMPP_RC_QVBR;
            break;
        default:
            HIMPP_PRT("input rcmode: %c, is invaild!\n",c);
            goto Begin_Get;
    }
    return enRcMode;
}

VENC_INTRA_REFRESH_MODE_E HIMPP_VENC_GetIntraRefreshMode(void)
{
    char c;
    VENC_INTRA_REFRESH_MODE_E   enIntraRefreshMode = INTRA_REFRESH_ROW;

Begin_Get:

    printf("please input choose IntraRefresh mode!\n");
    printf("\t r) ROW.\n");
    printf("\t c) COLUMN.\n");

    while((c = getchar()) != '\n' && c != EOF)
    switch(c)
    {
        case 'r':
            enIntraRefreshMode = INTRA_REFRESH_ROW;
            break;
        case 'c':
            enIntraRefreshMode = INTRA_REFRESH_COLUMN;
            break;

        default:
            HIMPP_PRT("input IntraRefresh Mode: %c, is invaild!\n",c);
            goto Begin_Get;
    }
    return enIntraRefreshMode;
}


static HI_U32 GetFrameRateFromSensorType(HIMPP_SNS_TYPE_E enSnsType)
{
    HI_U32 FrameRate;

    HIMPP_VI_GetFrameRateBySensor(enSnsType, &FrameRate);

    return FrameRate;
}

static HI_U32 GetFullLinesStdFromSensorType(HIMPP_SNS_TYPE_E enSnsType)
{
    HI_U32 FullLinesStd = 0;

    switch (enSnsType)
    {
		// my edit
		case OV5640_YUV_MIPI_2M_30FPS_8BIT:
            FullLinesStd = 1125;
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
            FullLinesStd = 1125;
            break;
        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SMART_SC2235_DC_2M_30FPS_10BIT:
        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
            FullLinesStd = 1125;
            break;
        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            FullLinesStd = 1875;
            break;
        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            FullLinesStd = 1375;
            break;
        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            FullLinesStd = 1600;
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            FullLinesStd = 1108;
            break;
        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            FullLinesStd = 1350;                     /* 1350 sensor SC3235 full lines */
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
            HIMPP_PRT("Error: sensor type %d resolution out of limits, not support ring!\n",enSnsType);
            break;
        default:
            HIMPP_PRT("Error: Not support this sensor now! ==> %d\n",enSnsType);
            break;
    }

    return FullLinesStd;
}

static HI_VOID AdjustWrapBufLineBySnsType(HIMPP_SNS_TYPE_E enSnsType, HI_U32 *pWrapBufLine)
{
    /*some sensor as follow need to expand the wrapBufLine*/
    if ((enSnsType == SMART_SC4236_MIPI_3M_30FPS_10BIT) ||
        (enSnsType == SMART_SC4236_MIPI_3M_20FPS_10BIT) ||
        (enSnsType == SMART_SC2235_DC_2M_30FPS_10BIT))
    {
        *pWrapBufLine += WRAP_BUF_LINE_EXT;
    }

    return;
}

static HI_VOID GetSensorResolution(HIMPP_SNS_TYPE_E enSnsType, SIZE_S *pSnsSize)
{
    HI_S32          ret;
    SIZE_S          SnsSize;
    PIC_SIZE_E      enSnsSize;

    ret = HIMPP_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != ret)
    {
        HIMPP_PRT("HIMPP_VI_GetSizeBySensor failed!\n");
        return;
    }
    ret = HIMPP_SYS_GetPicSize(enSnsSize, &SnsSize);
    if (HI_SUCCESS != ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return;
    }

    *pSnsSize = SnsSize;

    return;
}

static HI_VOID GetVpssWrapBufLine(HIMPP_VPSS_CHN_ATTR_S *pParam)
{
    HI_U32 vpssWrapBufLine = 0;

    VPSS_VENC_WRAP_PARAM_S wrapParam;

    memset(&wrapParam, 0, sizeof(VPSS_VENC_WRAP_PARAM_S));
    wrapParam.bAllOnline      = (pParam->ViVpssMode == VI_ONLINE_VPSS_ONLINE) ? 1 : 0;
    wrapParam.u32FrameRate    = GetFrameRateFromSensorType(pParam->enSnsType);
    wrapParam.u32FullLinesStd = GetFullLinesStdFromSensorType(pParam->enSnsType);
    wrapParam.stLargeStreamSize.u32Width = pParam->stOutPutSize[pParam->BigStreamId].u32Width;
    wrapParam.stLargeStreamSize.u32Height= pParam->stOutPutSize[pParam->BigStreamId].u32Height;
    wrapParam.stSmallStreamSize.u32Width = pParam->stOutPutSize[pParam->SmallStreamId].u32Width;
    wrapParam.stSmallStreamSize.u32Height= pParam->stOutPutSize[pParam->SmallStreamId].u32Height;

    if (HI_MPI_SYS_GetVPSSVENCWrapBufferLine(&wrapParam, &vpssWrapBufLine) != HI_SUCCESS)
    {
        HIMPP_PRT("Error:Current BigStream(%dx%d@%d fps) and SmallStream(%dx%d@%d fps) not support Ring!== return 0x%x(0x%x)\n",
            wrapParam.stLargeStreamSize.u32Width,wrapParam.stLargeStreamSize.u32Height,wrapParam.u32FrameRate,
            wrapParam.stSmallStreamSize.u32Width,wrapParam.stSmallStreamSize.u32Height,wrapParam.u32FrameRate,
            HI_MPI_SYS_GetVPSSVENCWrapBufferLine(&wrapParam, &vpssWrapBufLine), HI_ERR_SYS_NOT_SUPPORT);
        vpssWrapBufLine = 0;
    }
    else
    {
        AdjustWrapBufLineBySnsType(pParam->enSnsType, &vpssWrapBufLine);
    }

    pParam->WrapBufLine = vpssWrapBufLine;

    return;
}

static VI_VPSS_MODE_E GetViVpssModeFromResolution(HIMPP_SNS_TYPE_E SnsType)
{
    SIZE_S SnsSize = {0};
    VI_VPSS_MODE_E ViVpssMode;

    GetSensorResolution(SnsType, &SnsSize);

    if (SnsSize.u32Width > ONLINE_LIMIT_WIDTH)
    {
        ViVpssMode = VI_OFFLINE_VPSS_ONLINE;
    }
    else
    {
        ViVpssMode = VI_ONLINE_VPSS_ONLINE;
    }

    return ViVpssMode;
}

static HI_VOID HIMPP_VENC_GetDefaultVpssAttr(HIMPP_SNS_TYPE_E enSnsType, HI_BOOL *pChanEnable, SIZE_S stEncSize[], HIMPP_VPSS_CHN_ATTR_S *pVpssAttr)
{
    HI_S32 i;

    memset(pVpssAttr, 0, sizeof(HIMPP_VPSS_CHN_ATTR_S));

    pVpssAttr->enDynamicRange = DYNAMIC_RANGE_SDR8;
    pVpssAttr->enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    pVpssAttr->bWrapEn        = 0;
    pVpssAttr->enSnsType      = enSnsType;
    pVpssAttr->ViVpssMode     = GetViVpssModeFromResolution(enSnsType);

    for (i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++)
    {
        if (HI_TRUE == pChanEnable[i])
        {
            pVpssAttr->enCompressMode[i]          = (i == 0)? COMPRESS_MODE_SEG : COMPRESS_MODE_NONE;
            pVpssAttr->stOutPutSize[i].u32Width   = stEncSize[i].u32Width;
            pVpssAttr->stOutPutSize[i].u32Height  = stEncSize[i].u32Height;
            pVpssAttr->stFrameRate[i].s32SrcFrameRate  = -1;
            pVpssAttr->stFrameRate[i].s32DstFrameRate  = -1;
            pVpssAttr->bMirror[i]                      = HI_FALSE;
            pVpssAttr->bFlip[i]                        = HI_FALSE;

            pVpssAttr->bChnEnable[i]                   = HI_TRUE;
        }
    }

    return;
}

HI_S32 HIMPP_VENC_SYS_Init(HIMPP_VB_ATTR_S *pCommVbAttr)
{
    HI_S32 i;
    HI_S32 s32Ret;
    VB_CONFIG_S stVbConf;

    if (pCommVbAttr->validNum > VB_MAX_COMM_POOLS)
    {
        HIMPP_PRT("HIMPP_VENC_SYS_Init validNum(%d) too large than VB_MAX_COMM_POOLS(%d)!\n", pCommVbAttr->validNum, VB_MAX_COMM_POOLS);
        return HI_FAILURE;
    }

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

    for (i = 0; i < pCommVbAttr->validNum; i++)
    {
        stVbConf.astCommPool[i].u64BlkSize   = pCommVbAttr->blkSize[i];
        stVbConf.astCommPool[i].u32BlkCnt    = pCommVbAttr->blkCnt[i];
        //printf("%s,%d,stVbConf.astCommPool[%d].u64BlkSize = %lld, blkSize = %d\n",__func__,__LINE__,i,stVbConf.astCommPool[i].u64BlkSize,stVbConf.astCommPool[i].u32BlkCnt);
    }

    stVbConf.u32MaxPoolCnt = pCommVbAttr->validNum;

    if(pCommVbAttr->supplementConfig == 0)
    {
        s32Ret = HIMPP_SYS_Init(&stVbConf);
    }
    else
    {
        s32Ret = HIMPP_SYS_InitWithVbSupplement(&stVbConf,pCommVbAttr->supplementConfig);
    }

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_VOID HIMPP_VENC_SetDCFInfo(VI_PIPE ViPipe)
{
    ISP_DCF_INFO_S stIspDCF;

    HI_MPI_ISP_GetDCFInfo(ViPipe, &stIspDCF);

    //description: Thumbnail test
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8ImageDescription,"Thumbnail test",DCF_DRSCRIPTION_LENGTH);
    //manufacturer: Hisilicon
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Make,"Hisilicon",DCF_DRSCRIPTION_LENGTH);
    //model number: Hisilicon IP Camera
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Model,"Hisilicon IP Camera",DCF_DRSCRIPTION_LENGTH);
    //firmware version: v.1.1.0
    strncpy((char *)stIspDCF.stIspDCFConstInfo.au8Software,"v.1.1.0",DCF_DRSCRIPTION_LENGTH);


    stIspDCF.stIspDCFConstInfo.u32FocalLength             = 0x00640001;
    stIspDCF.stIspDCFConstInfo.u8Contrast                 = 5;
    stIspDCF.stIspDCFConstInfo.u8CustomRendered           = 0;
    stIspDCF.stIspDCFConstInfo.u8FocalLengthIn35mmFilm    = 1;
    stIspDCF.stIspDCFConstInfo.u8GainControl              = 1;
    stIspDCF.stIspDCFConstInfo.u8LightSource              = 1;
    stIspDCF.stIspDCFConstInfo.u8MeteringMode             = 1;
    stIspDCF.stIspDCFConstInfo.u8Saturation               = 1;
    stIspDCF.stIspDCFConstInfo.u8SceneCaptureType         = 1;
    stIspDCF.stIspDCFConstInfo.u8SceneType                = 0;
    stIspDCF.stIspDCFConstInfo.u8Sharpness                = 5;
    stIspDCF.stIspDCFUpdateInfo.u32ISOSpeedRatings         = 500;
    stIspDCF.stIspDCFUpdateInfo.u32ExposureBiasValue       = 5;
    stIspDCF.stIspDCFUpdateInfo.u32ExposureTime            = 0x00010004;
    stIspDCF.stIspDCFUpdateInfo.u32FNumber                 = 0x0001000f;
    stIspDCF.stIspDCFUpdateInfo.u8WhiteBalance             = 1;
    stIspDCF.stIspDCFUpdateInfo.u8ExposureMode             = 0;
    stIspDCF.stIspDCFUpdateInfo.u8ExposureProgram          = 1;
    stIspDCF.stIspDCFUpdateInfo.u32MaxApertureValue        = 0x00010001;

    HI_MPI_ISP_SetDCFInfo(ViPipe, &stIspDCF);

    return;
}

HI_S32 HIMPP_VENC_VI_Init( HIMPP_VI_CONFIG_S *pstViConfig, VI_VPSS_MODE_E ViVpssMode)
{
    HI_S32              s32Ret;
    HIMPP_SNS_TYPE_E   enSnsType;
    ISP_CTRL_PARAM_S    stIspCtrlParam;
    HI_U32              u32FrameRate;


    enSnsType = pstViConfig->astViInfo[0].stSnsInfo.enSnsType;

    pstViConfig->as32WorkingViId[0]                           = 0;

    pstViConfig->astViInfo[0].stSnsInfo.MipiDev            = HIMPP_VI_GetComboDevBySensor(pstViConfig->astViInfo[0].stSnsInfo.enSnsType, 0);
    pstViConfig->astViInfo[0].stSnsInfo.s32BusId           = 0;
    pstViConfig->astViInfo[0].stDevInfo.enWDRMode          = WDR_MODE_NONE;
    pstViConfig->astViInfo[0].stPipeInfo.enMastPipeMode    = ViVpssMode;

    //pstViConfig->astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    pstViConfig->astViInfo[0].stPipeInfo.aPipe[1]          = -1;

    //pstViConfig->astViInfo[0].stChnInfo.ViChn              = ViChn;
    //pstViConfig->astViInfo[0].stChnInfo.enPixFormat        = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    //pstViConfig->astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    pstViConfig->astViInfo[0].stChnInfo.enVideoFormat      = VIDEO_FORMAT_LINEAR;
    pstViConfig->astViInfo[0].stChnInfo.enCompressMode     = COMPRESS_MODE_NONE;
    s32Ret = HIMPP_VI_SetParam(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_SetParam failed with %d!\n", s32Ret);
        return s32Ret;
    }

    HIMPP_VI_GetFrameRateBySensor(enSnsType, &u32FrameRate);

    s32Ret = HI_MPI_ISP_GetCtrlParam(pstViConfig->astViInfo[0].stPipeInfo.aPipe[0], &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HI_MPI_ISP_GetCtrlParam failed with %d!\n", s32Ret);
        return s32Ret;
    }
    stIspCtrlParam.u32StatIntvl  = u32FrameRate/30;
    if (stIspCtrlParam.u32StatIntvl == 0)
    {
        stIspCtrlParam.u32StatIntvl = 1;
    }

    s32Ret = HI_MPI_ISP_SetCtrlParam(pstViConfig->astViInfo[0].stPipeInfo.aPipe[0], &stIspCtrlParam);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HI_MPI_ISP_SetCtrlParam failed with %d!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HIMPP_VI_StartVi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_SYS_Exit();
        HIMPP_PRT("HIMPP_VI_StartVi failed with %d!\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static HI_S32 HIMPP_VENC_VPSS_CreateGrp(VPSS_GRP VpssGrp, HIMPP_VPSS_CHN_ATTR_S *pParam)
{
    HI_S32          s32Ret;
    // PIC_SIZE_E      enSnsSize;
    SIZE_S          stSnsSize;
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};

    // s32Ret = HIMPP_VI_GetSizeBySensor(pParam->enSnsType, &enSnsSize);
    // if (HI_SUCCESS != s32Ret)
    // {
    //     HIMPP_PRT("HIMPP_VI_GetSizeBySensor failed!\n");
    //     return s32Ret;
    // }

    s32Ret = HIMPP_SYS_GetPicSize(pParam->enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    stVpssGrpAttr.enDynamicRange          = pParam->enDynamicRange;
    stVpssGrpAttr.enPixelFormat           = pParam->enPixelFormat;
    stVpssGrpAttr.u32MaxW                 = stSnsSize.u32Width;
    stVpssGrpAttr.u32MaxH                 = stSnsSize.u32Height;
    stVpssGrpAttr.bNrEn                   = HI_TRUE;
    stVpssGrpAttr.stNrAttr.enNrType       = VPSS_NR_TYPE_VIDEO;
    stVpssGrpAttr.stNrAttr.enNrMotionMode = NR_MOTION_MODE_NORMAL;
    stVpssGrpAttr.stNrAttr.enCompressMode = COMPRESS_MODE_FRAME;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, &stVpssGrpAttr);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

static HI_S32 HIMPP_VENC_VPSS_DestoryGrp(VPSS_GRP VpssGrp)
{
    HI_S32          s32Ret;

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

static HI_S32 HIMPP_VENC_VPSS_StartGrp(VPSS_GRP VpssGrp)
{
    HI_S32          s32Ret;

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

static HI_S32 HIMPP_VENC_VPSS_ChnEnable(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HIMPP_VPSS_CHN_ATTR_S *pParam, HI_BOOL bWrapEn)
{
    HI_S32 s32Ret;
    VPSS_CHN_ATTR_S     stVpssChnAttr;
    VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;
    VPSS_CHN_BUF_WRAP_S stVpssChnBufWrap;

    memset(&stVpssChnAttr, 0, sizeof(VPSS_CHN_ATTR_S));
    stVpssChnAttr.u32Width                     = pParam->stOutPutSize[VpssChn].u32Width;
    stVpssChnAttr.u32Height                    = pParam->stOutPutSize[VpssChn].u32Height;
    stVpssChnAttr.enChnMode                    = VPSS_CHN_MODE_USER;
    stVpssChnAttr.enCompressMode               = pParam->enCompressMode[VpssChn];
    stVpssChnAttr.enDynamicRange               = pParam->enDynamicRange;
    stVpssChnAttr.enPixelFormat                = pParam->enPixelFormat;
    if (stVpssChnAttr.u32Width * stVpssChnAttr.u32Height > 2688 * 1520 ) {
        stVpssChnAttr.stFrameRate.s32SrcFrameRate  = 30;
        stVpssChnAttr.stFrameRate.s32DstFrameRate  = 20;
    } else {
        stVpssChnAttr.stFrameRate.s32SrcFrameRate  = pParam->stFrameRate[VpssChn].s32SrcFrameRate;
        stVpssChnAttr.stFrameRate.s32DstFrameRate  = pParam->stFrameRate[VpssChn].s32DstFrameRate;
    }
    stVpssChnAttr.u32Depth                     = 0;
    stVpssChnAttr.bMirror                      = pParam->bMirror[VpssChn];
    stVpssChnAttr.bFlip                        = pParam->bFlip[VpssChn];
    stVpssChnAttr.enVideoFormat                = VIDEO_FORMAT_LINEAR;
    stVpssChnAttr.stAspectRatio.enMode         = ASPECT_RATIO_NONE;


	// my edit 扩展通道全部绑定物理通道0，方便用物理通道做一些统一的预处理
	memset(&stVpssExtChnAttr, 0, sizeof(VPSS_EXT_CHN_ATTR_S));
	stVpssExtChnAttr.s32BindChn                  = 0;
    stVpssExtChnAttr.u32Width                    = pParam->stOutPutSize[VpssChn].u32Width;
    stVpssExtChnAttr.u32Height                   = pParam->stOutPutSize[VpssChn].u32Height;
    stVpssExtChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
    stVpssExtChnAttr.enPixelFormat               = pParam->enPixelFormat;
    stVpssExtChnAttr.enDynamicRange              = pParam->enDynamicRange;
    stVpssExtChnAttr.enCompressMode              = pParam->enCompressMode[VpssChn];
    stVpssExtChnAttr.u32Depth                    = 0;
	if (stVpssExtChnAttr.u32Width * stVpssExtChnAttr.u32Height > 2688 * 1520 ) {
        stVpssExtChnAttr.stFrameRate.s32SrcFrameRate  = 30;
        stVpssExtChnAttr.stFrameRate.s32DstFrameRate  = 20;
    } else {
        stVpssExtChnAttr.stFrameRate.s32SrcFrameRate  = pParam->stFrameRate[VpssChn].s32SrcFrameRate;
        stVpssExtChnAttr.stFrameRate.s32DstFrameRate  = pParam->stFrameRate[VpssChn].s32DstFrameRate;
    }
	// my edit TODO 如果是缩略图通道，特殊设置
	if (0) {
    	stVpssExtChnAttr.u32Depth = 2;
	}


	if (VpssChn < VPSS_MAX_PHY_CHN_NUM) {
    	s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, &stVpssChnAttr);
	}
	else {
    	s32Ret = HI_MPI_VPSS_SetExtChnAttr(VpssGrp, VpssChn, &stVpssExtChnAttr);
	}
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VPSS_Set(Ext)ChnAttr chan %d failed with %#x\n", VpssChn, s32Ret);
        goto exit0;
    }


    if (bWrapEn)
    {
        if (VpssChn != 0)   //vpss limit! just vpss chan0 support wrap
        {
            HIMPP_PRT("Error:Just vpss chan 0 support wrap! Current chan %d\n", VpssChn);
            goto exit0;
        }


        HI_U32 WrapBufLen = 0;
        VPSS_VENC_WRAP_PARAM_S WrapParam;

        memset(&WrapParam, 0, sizeof(VPSS_VENC_WRAP_PARAM_S));
        WrapParam.bAllOnline      = (pParam->ViVpssMode == VI_ONLINE_VPSS_ONLINE) ? 1 : 0;
        WrapParam.u32FrameRate    = GetFrameRateFromSensorType(pParam->enSnsType);
        WrapParam.u32FullLinesStd = GetFullLinesStdFromSensorType(pParam->enSnsType);
        WrapParam.stLargeStreamSize.u32Width = pParam->stOutPutSize[pParam->BigStreamId].u32Width;
        WrapParam.stLargeStreamSize.u32Height= pParam->stOutPutSize[pParam->BigStreamId].u32Height;
        WrapParam.stSmallStreamSize.u32Width = pParam->stOutPutSize[pParam->SmallStreamId].u32Width;
        WrapParam.stSmallStreamSize.u32Height= pParam->stOutPutSize[pParam->SmallStreamId].u32Height;

        if (HI_MPI_SYS_GetVPSSVENCWrapBufferLine(&WrapParam, &WrapBufLen) == HI_SUCCESS)
        {
            AdjustWrapBufLineBySnsType(pParam->enSnsType, &WrapBufLen);

            stVpssChnBufWrap.u32WrapBufferSize = VPSS_GetWrapBufferSize(WrapParam.stLargeStreamSize.u32Width,
                WrapParam.stLargeStreamSize.u32Height, WrapBufLen, pParam->enPixelFormat, DATA_BITWIDTH_8,
                COMPRESS_MODE_NONE, DEFAULT_ALIGN);
            stVpssChnBufWrap.bEnable = 1;
            stVpssChnBufWrap.u32BufLine = WrapBufLen;
            s32Ret = HI_MPI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &stVpssChnBufWrap);
            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("HI_MPI_VPSS_SetChnBufWrapAttr Chn %d failed with %#x\n", VpssChn, s32Ret);
                goto exit0;
            }
        }
        else
        {
            HIMPP_PRT("Current sensor type: %d, not support BigStream(%dx%d) and SmallStream(%dx%d) Ring!!\n",
                pParam->enSnsType,
                pParam->stOutPutSize[pParam->BigStreamId].u32Width, pParam->stOutPutSize[pParam->BigStreamId].u32Height,
                pParam->stOutPutSize[pParam->SmallStreamId].u32Width, pParam->stOutPutSize[pParam->SmallStreamId].u32Height);
        }

    }

    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VPSS_EnableChn (%d) failed with %#x\n", VpssChn, s32Ret);
        goto exit0;
    }

exit0:
    return s32Ret;
}

static HI_S32 HIMPP_VENC_VPSS_ChnDisable(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

static HI_S32 HIMPP_VENC_VPSS_Init(VPSS_GRP VpssGrp, HIMPP_VPSS_CHN_ATTR_S *pstParam)
{
    HI_S32 i,j;
    HI_S32 s32Ret;
    HI_BOOL bWrapEn;

    s32Ret = HIMPP_VENC_VPSS_CreateGrp(VpssGrp, pstParam);
    if (s32Ret != HI_SUCCESS)
    {
        goto exit0;
    }

	//my edit
    for (i = 0; i < VPSS_MAX_CHN_NUM; i++)
    {
        if (pstParam->bChnEnable[i] == HI_TRUE)
        {
            bWrapEn = (i==0)? pstParam->bWrapEn : 0;

            s32Ret = HIMPP_VENC_VPSS_ChnEnable(VpssGrp, i, pstParam, bWrapEn);
            if (s32Ret != HI_SUCCESS)
            {
                goto exit1;
            }
        }
    }

    i--; // for abnormal case 'exit1' prossess;

    s32Ret = HIMPP_VENC_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS)
    {
        goto exit1;
    }

    return s32Ret;

exit1:
    for (j = 0; j <= i; j++)
    {
        if (pstParam->bChnEnable[j] == HI_TRUE)
        {
            HIMPP_VENC_VPSS_ChnDisable(VpssGrp, i);
        }
    }

    HIMPP_VENC_VPSS_DestoryGrp(VpssGrp);
exit0:
    return s32Ret;
}

static HI_VOID HIMPP_VENC_GetCommVbAttr(const HIMPP_SNS_TYPE_E enSnsType, const HIMPP_VPSS_CHN_ATTR_S *pstParam,
    HI_BOOL bSupportDcf, HIMPP_VB_ATTR_S * pstcommVbAttr)
{
    if (pstParam->ViVpssMode != VI_ONLINE_VPSS_ONLINE)
    {
        SIZE_S snsSize = {0};
        GetSensorResolution(enSnsType, &snsSize);

        if (pstParam->ViVpssMode == VI_OFFLINE_VPSS_ONLINE || pstParam->ViVpssMode == VI_OFFLINE_VPSS_OFFLINE)
        {
            pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = VI_GetRawBufferSize(snsSize.u32Width, snsSize.u32Height,
                                                                                  PIXEL_FORMAT_RGB_BAYER_12BPP,
                                                                                  COMPRESS_MODE_NONE,
                                                                                  DEFAULT_ALIGN);
            pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 3;
            pstcommVbAttr->validNum++;
        }

        if (pstParam->ViVpssMode == VI_OFFLINE_VPSS_OFFLINE)
        {
            pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = COMMON_GetPicBufferSize(snsSize.u32Width, snsSize.u32Height,
                                                                                      PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                                                                      DATA_BITWIDTH_8,
                                                                                      COMPRESS_MODE_NONE,
                                                                                      DEFAULT_ALIGN);
            pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 2;
            pstcommVbAttr->validNum++;
        }

        if (pstParam->ViVpssMode == VI_ONLINE_VPSS_OFFLINE)
        {
            pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = COMMON_GetPicBufferSize(snsSize.u32Width, snsSize.u32Height,
                                                                                      PIXEL_FORMAT_YVU_SEMIPLANAR_420,
                                                                                      DATA_BITWIDTH_8,
                                                                                      COMPRESS_MODE_NONE,
                                                                                      DEFAULT_ALIGN);
            pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 3;
            pstcommVbAttr->validNum++;

        }
    }
    if(HI_TRUE == pstParam->bWrapEn)
    {
        pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = VPSS_GetWrapBufferSize(pstParam->stOutPutSize[pstParam->BigStreamId].u32Width,
                                                                                 pstParam->stOutPutSize[pstParam->BigStreamId].u32Height,
                                                                                 pstParam->WrapBufLine,
                                                                                 pstParam->enPixelFormat,DATA_BITWIDTH_8,COMPRESS_MODE_NONE,DEFAULT_ALIGN);
        pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 1;
        pstcommVbAttr->validNum++;
    }
    else
    {
        pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = COMMON_GetPicBufferSize(pstParam->stOutPutSize[0].u32Width, pstParam->stOutPutSize[0].u32Height,
                                                                                  pstParam->enPixelFormat,
                                                                                  DATA_BITWIDTH_8,
                                                                                  pstParam->enCompressMode[0],
                                                                                  DEFAULT_ALIGN);

        if (pstParam->ViVpssMode == VI_ONLINE_VPSS_ONLINE)
        {
            pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 3;
        }
        else
        {
            pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 2;
        }

        pstcommVbAttr->validNum++;
    }



    pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = COMMON_GetPicBufferSize(pstParam->stOutPutSize[1].u32Width, pstParam->stOutPutSize[1].u32Height,
                                                                              pstParam->enPixelFormat,
                                                                              DATA_BITWIDTH_8,
                                                                              pstParam->enCompressMode[1],
                                                                              DEFAULT_ALIGN);

    if (pstParam->ViVpssMode == VI_ONLINE_VPSS_ONLINE)
    {
        pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 3;
    }
    else
    {
        pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 2;
    }
    pstcommVbAttr->validNum++;


    //vgs dcf use
    if(HI_TRUE == bSupportDcf)
    {
        pstcommVbAttr->blkSize[pstcommVbAttr->validNum] = COMMON_GetPicBufferSize(160, 120,
                                                                                  pstParam->enPixelFormat,
                                                                                  DATA_BITWIDTH_8,
                                                                                  COMPRESS_MODE_NONE,
                                                                                  DEFAULT_ALIGN);
        pstcommVbAttr->blkCnt[pstcommVbAttr->validNum]  = 1;
        pstcommVbAttr->validNum++;
    }

}

HI_S32 HIMPP_VENC_CheckSensor(HIMPP_SNS_TYPE_E   enSnsType,SIZE_S  stSize)
{
    HI_S32 s32Ret;
    SIZE_S          stSnsSize;
    PIC_SIZE_E      enSnsSize;

    s32Ret = HIMPP_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }
    s32Ret = HIMPP_SYS_GetPicSize(enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    if((stSnsSize.u32Width < stSize.u32Width) || (stSnsSize.u32Height < stSize.u32Height))
    {
        //HIMPP_PRT("Sensor size is (%d,%d), but encode chnl is (%d,%d) !\n",
            //stSnsSize.u32Width,stSnsSize.u32Height,stSize.u32Width,stSize.u32Height);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VENC_ModifyResolution(HIMPP_SNS_TYPE_E   enSnsType,PIC_SIZE_E *penSize,SIZE_S *pstSize)
{
    HI_S32 s32Ret;
    SIZE_S          stSnsSize;
    PIC_SIZE_E      enSnsSize;

    s32Ret = HIMPP_VI_GetSizeBySensor(enSnsType, &enSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_GetSizeBySensor failed!\n");
        return s32Ret;
    }
    s32Ret = HIMPP_SYS_GetPicSize(enSnsSize, &stSnsSize);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
        return s32Ret;
    }

    *penSize = enSnsSize;
    pstSize->u32Width  = stSnsSize.u32Width;
    pstSize->u32Height = stSnsSize.u32Height;

    return HI_SUCCESS;
}

// my edit
static HIMPP_VI_CONFIG_S stViConfig;

/******************************************************************************
* function: H.265e + H264e@720P, H.265 Channel resolution adaptable with sensor
******************************************************************************/
HI_S32 HIMPP_VENC_H265_H264(void)
{
    HI_S32                i;
    HI_S32                s32Ret;
    SIZE_S                stSize[VPSS_MAX_CHN_NUM];
    VENC_GOP_MODE_E       enGopMode;
    VENC_GOP_ATTR_S       stGopAttr;
    HIMPP_RC_E            enRcMode;
    HI_BOOL               bRcnRefShareBuf = HI_TRUE;
    VI_DEV                ViDev           = 0;
	HIMPP_VPSS_CHN_ATTR_S stParam;
    
    // my edit
    HI_S32          vpssChnNum          = VPSS_MAX_CHN_NUM;
    SIZE_S          jpegSize;

    for(i=0; i<vpssChnNum; i++)
    {
        s32Ret = HIMPP_SYS_GetPicSize(g_vpssSize[i], &stSize[i]);
        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
            return s32Ret;
        }
    }

    HIMPP_VI_GetSensorInfo(&stViConfig);
    if(HIMPP_SNS_TYPE_BUTT == stViConfig.astViInfo[0].stSnsInfo.enSnsType)
    {
        HIMPP_PRT("Not set SENSOR%d_TYPE !\n",0);
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VENC_CheckSensor(stViConfig.astViInfo[0].stSnsInfo.enSnsType,stSize[0]);
    if(s32Ret != HI_SUCCESS)
    {
        s32Ret = HIMPP_VENC_ModifyResolution(stViConfig.astViInfo[0].stSnsInfo.enSnsType,&g_vpssSize[0],&stSize[0]);
        if(s32Ret != HI_SUCCESS)
        {
            return HI_FAILURE;
        }
    }

    stViConfig.s32WorkingViNum       = 1;
    stViConfig.astViInfo[0].stDevInfo.ViDev     = ViDev;
    stViConfig.astViInfo[0].stPipeInfo.aPipe[0] = g_ViPipe;
    stViConfig.astViInfo[0].stChnInfo.ViChn     = g_ViChn;
    stViConfig.astViInfo[0].stChnInfo.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stViConfig.astViInfo[0].stChnInfo.enPixFormat    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    // s32Ret = HIMPP_VENC_VI_Init(&stViConfig, stParam.ViVpssMode);
	// my edit 目前的情况如果VI输入YUV数据，那么 VI 必须在线，VPSS可以在线或离线，在线应该可以节省1帧内存
    s32Ret = HIMPP_VENC_VI_Init(&stViConfig, VI_ONLINE_VPSS_OFFLINE);
    // s32Ret = HIMPP_VENC_VI_Init(&stViConfig, VI_ONLINE_VPSS_ONLINE);
    if(s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("Init VI err for %#x!\n", s32Ret);
        return HI_FAILURE;
    }


	memset(&stParam, 0, sizeof(HIMPP_VPSS_CHN_ATTR_S));
	// 公共参数设置
    stParam.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stParam.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stParam.enSnsSize      = g_viSize;
    stParam.bWrapEn        = HI_FALSE;
    // stParam.ViVpssMode     = GetViVpssModeFromResolution(enSnsType);

	// 各通道独立参数设置
    for(i=0; i<VPSS_MAX_CHN_NUM; i++)
    {
        if(HI_TRUE == g_abChnEnable[i])
        {
			stParam.bChnEnable[i]                   = g_abChnEnable[i];
            stParam.enCompressMode[i]               = COMPRESS_MODE_NONE;
            stParam.stOutPutSize[i].u32Width        = stSize[i].u32Width;
            stParam.stOutPutSize[i].u32Height       = stSize[i].u32Height;
            stParam.stFrameRate[i].s32SrcFrameRate  = -1;
            stParam.stFrameRate[i].s32DstFrameRate  = -1;
			if (i < VPSS_MAX_PHY_CHN_NUM) {
				stParam.bMirror[i]                  = HI_FALSE;
				stParam.bFlip[i]                    = HI_FALSE;
			}
        }
    }

    s32Ret = HIMPP_VENC_VPSS_Init(g_VpssGrp, &stParam);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("Init VPSS err for %#x!\n", s32Ret);
        goto EXIT_VI_STOP;
    }

    s32Ret = HIMPP_VI_Bind_VPSS(g_ViPipe, g_ViChn, g_VpssGrp);
    if(s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("VI Bind VPSS err for %#x!\n", s32Ret);
        goto EXIT_VPSS_STOP;
    }

   /******************************************
    start stream venc
    ******************************************/

    //手动设置码率控制模式，参考 hiHIMPP_RC_E
    enRcMode = HIMPP_RC_CBR;
    // enRcMode = HIMPP_RC_VBR;
    // enRcMode = HIMPP_RC_CVBR;

    // 手动设置GOP模式 ，参考 VENC_GOP_MODE_E
    enGopMode = VENC_GOPMODE_NORMALP;


    s32Ret = HIMPP_VENC_GetGopAttr(enGopMode,&stGopAttr);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("Venc Get GopAttr for %#x!\n", s32Ret);
        goto EXIT_VI_VPSS_UNBIND;
    }

    // 启动视频编码器
    for (i = 0; i < ENABLE_VENC_CHN_NUM; i++) {
        if (g_enPayLoad[i] == PT_H264 || g_enPayLoad[i] == PT_H265) {
            s32Ret = HIMPP_VENC_Start(g_VencChn[i], g_enPayLoad[i], g_vencSize[i], enRcMode, g_u32Profile[i], bRcnRefShareBuf, &stGopAttr);
            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("Venc chn[%d] Start failed for %#x!\n", g_VencChn[i], s32Ret);
                goto EXIT_VPSS_VENC_UNBIND;
            }
            s32Ret = HIMPP_VPSS_Bind_VENC(g_VpssGrp, g_VpssChn[i], g_VencChn[i]);
            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("Venc chn[%d] bind Vpss grp[%d] chn[%d] failed for %#x!\n", g_VencChn[i], g_VpssGrp, g_VpssChn[i], s32Ret);
                goto EXIT_VPSS_VENC_UNBIND;
            }
        }

        if (g_enPayLoad[i] == PT_JPEG) {
            s32Ret = HIMPP_SYS_GetPicSize(g_vencSize[i], &jpegSize);
            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("HIMPP_SYS_GetPicSize failed!\n");
                return s32Ret;
            }

            s32Ret = HIMPP_VENC_SnapStart(g_VencChn[i], &jpegSize, HI_FALSE);
            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("Venc chn[%d] Start failed for %#x!\n", g_VencChn[i], s32Ret);
                goto EXIT_VPSS_VENC_UNBIND;
            }

            s32Ret = HIMPP_VPSS_Bind_VENC(g_VpssGrp, g_VpssChn[i], g_VencChn[i]);
            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("Venc chn[%d] bind Vpss grp[%d] chn[%d] failed for %#x!\n", g_VencChn[i], g_VpssGrp, g_VpssChn[i], s32Ret);
                goto EXIT_VPSS_VENC_UNBIND;
            }
        }
    }

    return HI_SUCCESS;

EXIT_VPSS_VENC_UNBIND:
    for (i = 0; i < ENABLE_VENC_CHN_NUM; i++) {
        HIMPP_VPSS_UnBind_VENC(g_VpssGrp, g_VpssChn[i], g_VencChn[i]);
        HIMPP_VENC_Stop(g_VencChn[i]);
    }
EXIT_VI_VPSS_UNBIND:
    HIMPP_VI_UnBind_VPSS(g_ViPipe,g_ViChn,g_VpssGrp);
EXIT_VPSS_STOP:
    HIMPP_VPSS_Stop(g_VpssGrp,g_abChnEnable);
EXIT_VI_STOP:
    HIMPP_VI_StopVi(&stViConfig);

    return s32Ret;
}

// my edit
HI_S32 HIMPP_VENC_H265_H264_stop(void)
{
    HI_S32 i;

    for (i = 0; i < ENABLE_VENC_CHN_NUM; i++) {
        HIMPP_VPSS_UnBind_VENC(g_VpssGrp, g_VpssChn[i], g_VencChn[i]);
        HIMPP_VENC_Stop(g_VencChn[i]);
    }

    HIMPP_VI_UnBind_VPSS(g_ViPipe,g_ViChn,g_VpssGrp);
    HIMPP_VPSS_Stop(g_VpssGrp,g_abChnEnable);
    HIMPP_VI_StopVi(&stViConfig);

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
