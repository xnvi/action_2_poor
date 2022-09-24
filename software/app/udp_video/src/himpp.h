

#ifndef __HIMPP_H__
#define __HIMPP_H__

#include <pthread.h>

#include "hi_common.h"
#include "hi_buffer.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_defines.h"
#include "hi_mipi.h"
#include "hi_comm_vgs.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_region.h"
#include "mpi_audio.h"
#include "mpi_isp.h"
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "hi_math.h"
#include "hi_sns_ctrl.h"
#include "mpi_vgs.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*******************************************************
    macro define
*******************************************************/
#define FILE_NAME_LEN               128

#define CHECK_CHN_RET(express,Chn,name)\
    do{\
        HI_S32 Ret;\
        Ret = express;\
        if (HI_SUCCESS != Ret)\
        {\
            printf("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn, __FUNCTION__, __LINE__, Ret);\
            fflush(stdout);\
            return Ret;\
        }\
    }while(0)

#define CHECK_RET(express,name)\
    do{\
        HI_S32 Ret;\
        Ret = express;\
        if (HI_SUCCESS != Ret)\
        {\
            printf("\033[0;31m%s failed at %s: LINE: %d with %#x!\033[0;39m\n", name, __FUNCTION__, __LINE__, Ret);\
            return Ret;\
        }\
    }while(0)
#define HIMPP_PIXEL_FORMAT         PIXEL_FORMAT_YVU_SEMIPLANAR_420

#define TLV320_FILE "/dev/tlv320aic31"
#define COLOR_RGB_RED      0xFF0000
#define COLOR_RGB_GREEN    0x00FF00
#define COLOR_RGB_BLUE     0x0000FF
#define COLOR_RGB_BLACK    0x000000
#define COLOR_RGB_YELLOW   0xFFFF00
#define COLOR_RGB_CYN      0x00ffff
#define COLOR_RGB_WHITE    0xffffff

#define HIMPP_VO_DEV_DHD0 0                  /* VO's device HD0 */
#define HIMPP_VO_DEV_UHD  HIMPP_VO_DEV_DHD0 /* VO's ultra HD device:HD0 */
#define HIMPP_VO_LAYER_VHD0 0

#define HIMPP_AUDIO_EXTERN_AI_DEV 0
#define HIMPP_AUDIO_EXTERN_AO_DEV 0
#define HIMPP_AUDIO_INNER_AI_DEV 0
#define HIMPP_AUDIO_INNER_AO_DEV 0
#define HIMPP_AUDIO_INNER_HDMI_AO_DEV 1

#define HIMPP_AUDIO_PTNUMPERFRM   480

#define WDR_MAX_PIPE_NUM        2


#define PAUSE()  do {\
        printf("---------------press Enter key to exit!---------------\n");\
        getchar();\
    } while (0)


#define HIMPP_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
    }while(0)

#define CHECK_NULL_PTR(ptr)\
    do{\
        if(NULL == ptr)\
        {\
            printf("func:%s,line:%d, NULL pointer\n",__FUNCTION__,__LINE__);\
            return HI_FAILURE;\
        }\
    }while(0)


/*******************************************************
    enum define
*******************************************************/

typedef enum hiPIC_SIZE_E
{
	PIC_PREVIEW,
    PIC_CIF,
    PIC_360P,      /* 640 * 360 */
    PIC_VGA,       /* 640 * 480 */
    PIC_640x360,
    PIC_640x480,
    PIC_D1_PAL,    /* 720 * 576 */
    PIC_D1_NTSC,   /* 720 * 480 */
    PIC_720P,      /* 1280 * 720  */
    PIC_1080P,     /* 1920 * 1080 */
	PIC_1600x1200,
	PIC_1920x1440,
    PIC_2304x1296,
	PIC_2560x1440,
    PIC_2592x1520,
    PIC_2592x1536,
    PIC_2592x1944,
    PIC_2688x1520,
    PIC_2688x1536,
    PIC_2688x1944,
    PIC_2716x1524,
    PIC_3840x2160,
    PIC_4096x2160,
    PIC_3000x3000,
    PIC_4000x3000,
    PIC_7680x4320,
    PIC_3840x8640,
    PIC_BUTT
} PIC_SIZE_E;

typedef enum hiHIMPP_SNS_TYPE_E
{
    SONY_IMX327_MIPI_2M_30FPS_12BIT,
    SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX327_2L_MIPI_2M_30FPS_12BIT,
    SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX307_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX307_2L_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX335_MIPI_5M_30FPS_12BIT,
    SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1,
    SONY_IMX335_MIPI_4M_30FPS_12BIT,
    SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1,
    SMART_SC4236_MIPI_3M_30FPS_10BIT,
    SMART_SC4236_MIPI_3M_20FPS_10BIT,
    SMART_SC2231_MIPI_2M_30FPS_10BIT,
    SOI_JXF37_MIPI_2M_30FPS_10BIT,
    SMART_SC2235_DC_2M_30FPS_10BIT,
    SMART_SC3235_MIPI_3M_30FPS_10BIT,
    GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT,
    GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR,
    OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT,
    OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT,
    OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1,
	OV5640_YUV_MIPI_2M_30FPS_8BIT, // my edit
    BT1120_2M_30FPS_8BIT,
    BT656_2M_30FPS_8BIT,
    BT601_2M_30FPS_8BIT,
    HIMPP_SNS_TYPE_BUTT,
} HIMPP_SNS_TYPE_E;

typedef enum hiHIMPP_RC_E
{
    HIMPP_RC_CBR = 0,
    HIMPP_RC_VBR,
    HIMPP_RC_CVBR,
    HIMPP_RC_AVBR,
    HIMPP_RC_QPMAP,
    HIMPP_RC_FIXQP,
    HIMPP_RC_QVBR
} HIMPP_RC_E;


/*******************************************************
    structure define
*******************************************************/
typedef struct hiHIMPP_VENC_GETSTREAM_PARA_S
{
    HI_BOOL bThreadStart;
    VENC_CHN VeChn[VENC_MAX_CHN_NUM];
    HI_S32  s32Cnt;
} HIMPP_VENC_GETSTREAM_PARA_S;

typedef struct hiHIMPP_VENC_QPMAP_SENDFRAME_PARA_S
{
    HI_BOOL  bThreadStart;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn[VPSS_MAX_PHY_CHN_NUM];
    VENC_CHN VeChn[VENC_MAX_CHN_NUM];
    HI_S32   s32Cnt;
    SIZE_S   stSize[VENC_MAX_CHN_NUM];
} HIMPP_VENC_QPMAP_SENDFRAME_PARA_S;


typedef struct hiHIMPP_VI_DUMP_THREAD_INFO_S
{
    VI_PIPE     ViPipe;
    HI_S32      s32Cnt;
    HI_BOOL     bDump;
    HI_CHAR     aszName[128];
    pthread_t   ThreadId;
} HIMPP_VI_DUMP_THREAD_INFO_S;

typedef struct hiHIMPP_SENSOR_INFO_S
{
    HIMPP_SNS_TYPE_E   enSnsType;
    HI_S32              s32SnsId;
    HI_S32              s32BusId;
    combo_dev_t           MipiDev;
} HIMPP_SENSOR_INFO_S;

typedef struct hiHIMPP_SNAP_INFO_S
{
    HI_BOOL  bSnap;
    HI_BOOL  bDoublePipe;
    VI_PIPE    VideoPipe;
    VI_PIPE    SnapPipe;
    VI_VPSS_MODE_E  enVideoPipeMode;
    VI_VPSS_MODE_E  enSnapPipeMode;
} HIMPP_SNAP_INFO_S;

typedef struct hiHIMPP_DEV_INFO_S
{
    VI_DEV      ViDev;
    WDR_MODE_E  enWDRMode;
} HIMPP_DEV_INFO_S;

typedef struct hiHIMPP_PIPE_INFO_S
{
    VI_PIPE         aPipe[WDR_MAX_PIPE_NUM];
    VI_VPSS_MODE_E  enMastPipeMode;
    HI_BOOL         bMultiPipe;
    HI_BOOL         bVcNumCfged;
    HI_BOOL         bIspBypass;
    PIXEL_FORMAT_E  enPixFmt;
    HI_U32          u32VCNum[WDR_MAX_PIPE_NUM];
} HIMPP_PIPE_INFO_S;

typedef struct hiHIMPP_CHN_INFO_S
{
    VI_CHN              ViChn;
    PIXEL_FORMAT_E      enPixFormat;
    DYNAMIC_RANGE_E     enDynamicRange;
    VIDEO_FORMAT_E      enVideoFormat;
    COMPRESS_MODE_E     enCompressMode;
} HIMPP_CHN_INFO_S;

typedef struct hiHIMPP_VI_INFO_S
{
    HIMPP_SENSOR_INFO_S    stSnsInfo;
    HIMPP_DEV_INFO_S       stDevInfo;
    HIMPP_PIPE_INFO_S      stPipeInfo;
    HIMPP_CHN_INFO_S       stChnInfo;
    HIMPP_SNAP_INFO_S      stSnapInfo;
} HIMPP_VI_INFO_S;

typedef struct hiHIMPP_VI_CONFIG_S
{
    HIMPP_VI_INFO_S    astViInfo[VI_MAX_DEV_NUM];
    HI_S32              as32WorkingViId[VI_MAX_DEV_NUM];
    HI_S32              s32WorkingViNum;
} HIMPP_VI_CONFIG_S;

typedef struct hiHIMPP_VI_FRAME_CONFIG_S
{
    HI_U32                  u32Width;
    HI_U32                  u32Height;
    HI_U32                  u32ByteAlign;
    PIXEL_FORMAT_E          enPixelFormat;
    VIDEO_FORMAT_E          enVideoFormat;
    COMPRESS_MODE_E         enCompressMode;
    DYNAMIC_RANGE_E         enDynamicRange;
} HIMPP_VI_FRAME_CONFIG_S;

typedef struct hiHIMPP_VI_FRAME_INFO_S
{
    VB_BLK             VbBlk;
    HI_U32             u32Size;
    VIDEO_FRAME_INFO_S stVideoFrameInfo;
} HIMPP_VI_FRAME_INFO_S;

typedef struct hiHIMPP_VI_FPN_CALIBRATE_INFO_S
{
    HI_U32                  u32Threshold;
    HI_U32                  u32FrameNum;
    ISP_FPN_TYPE_E          enFpnType;
    PIXEL_FORMAT_E          enPixelFormat;
    COMPRESS_MODE_E         enCompressMode;
} HIMPP_VI_FPN_CALIBRATE_INFO_S;

typedef struct hiHIMPP_VI_FPN_CORRECTION_INFO_S
{
    ISP_OP_TYPE_E           enOpType;
    ISP_FPN_TYPE_E          enFpnType;
    HI_U32                  u32Strength;
    PIXEL_FORMAT_E          enPixelFormat;
    COMPRESS_MODE_E         enCompressMode;
    HIMPP_VI_FRAME_INFO_S  stViFrameInfo;
} HIMPP_VI_FPN_CORRECTION_INFO_S;

typedef struct tag_HIMPP_VO_WBC_CONFIG
{
    VO_WBC_SOURCE_TYPE_E    enSourceType;
    DYNAMIC_RANGE_E         enDynamicRange;
    COMPRESS_MODE_E         enCompressMode;
    HI_S32 s32Depth;

    HI_S32                  VoWbc;
    VO_WBC_ATTR_S           stWbcAttr;
    VO_WBC_SOURCE_S         stWbcSource;
    VO_WBC_MODE_E           enWbcMode;

} HIMPP_VO_WBC_CONFIG;

typedef struct hiHIMPP_VO_CONFIG_S
{
    /* for device */
    VO_DEV                  VoDev;
    VO_INTF_TYPE_E          enVoIntfType;
    VO_INTF_SYNC_E          enIntfSync;
    PIC_SIZE_E              enPicSize;
    HI_U32                  u32BgColor;

    /* for layer */
    PIXEL_FORMAT_E          enPixFormat;
    RECT_S                  stDispRect;
    SIZE_S                  stImageSize;
    VO_PART_MODE_E          enVoPartMode;

    HI_U32                  u32DisBufLen;
    DYNAMIC_RANGE_E         enDstDynamicRange;
} HIMPP_VO_CONFIG_S;


typedef enum hiTHREAD_CONTRL_E
{
    THREAD_CTRL_START,
    THREAD_CTRL_PAUSE,
    THREAD_CTRL_STOP,
} THREAD_CONTRL_E;

typedef struct hiVDEC_THREAD_PARAM_S
{
    HI_S32 s32ChnId;
    PAYLOAD_TYPE_E enType;
    HI_CHAR cFilePath[128];
    HI_CHAR cFileName[128];
    HI_S32 s32StreamMode;
    HI_S32 s32MilliSec;
    HI_S32 s32MinBufSize;
    HI_S32 s32IntervalTime;
    THREAD_CONTRL_E eThreadCtrl;
    HI_U64  u64PtsInit;
    HI_U64  u64PtsIncrease;
    HI_BOOL bCircleSend;
} VDEC_THREAD_PARAM_S;

typedef struct hiHIMPP_VDEC_BUF
{
    HI_U32  u32PicBufSize;
    HI_U32  u32TmvBufSize;
    HI_BOOL bPicBufAlloc;
    HI_BOOL bTmvBufAlloc;
} HIMPP_VDEC_BUF;



typedef struct hiHIMPP_VB_BASE_INFO_S
{
    PIXEL_FORMAT_E      enPixelFormat;
    HI_U32              u32Width;
    HI_U32              u32Height;
    HI_U32              u32Align;
    COMPRESS_MODE_E     enCompressMode;
} HIMPP_VB_BASE_INFO_S;

typedef struct hiHIMPP_LINE_S {
    POINT_S stPoint1;
    POINT_S stPoint2;
    HI_U32  u32Thick;
    HI_BOOL bDisplay;
} HIMPP_LINE_S;


/*******************************************************
    function announce
*******************************************************/

HI_VOID *HIMPP_SYS_IOMmap(HI_U64 u64PhyAddr, HI_U32 u32Size);
HI_S32 HIMPP_SYS_Munmap(HI_VOID *pVirAddr, HI_U32 u32Size);
HI_S32 HIMPP_SYS_SetReg(HI_U64 u64Addr, HI_U32 u32Value);
HI_S32 HIMPP_SYS_GetReg(HI_U64 u64Addr, HI_U32 *pu32Value);

HI_S32 HIMPP_SYS_GetPicSize(PIC_SIZE_E enPicSize, SIZE_S *pstSize);
HI_S32 HIMPP_SYS_MemConfig(HI_VOID);
HI_VOID HIMPP_SYS_Exit(void);
HI_S32 HIMPP_SYS_Init(VB_CONFIG_S *pstVbConfig);
HI_S32 HIMPP_SYS_InitWithVbSupplement(VB_CONFIG_S *pstVbConf, HI_U32 u32SupplementConfig);

HI_S32 HIMPP_VI_Bind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
HI_S32 HIMPP_VI_UnBind_VO(VI_PIPE ViPipe, VI_CHN ViChn, VO_LAYER VoLayer, VO_CHN VoChn);
HI_S32 HIMPP_VI_Bind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
HI_S32 HIMPP_VI_UnBind_VPSS(VI_PIPE ViPipe, VI_CHN ViChn, VPSS_GRP VpssGrp);
HI_S32 HIMPP_VI_Bind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
HI_S32 HIMPP_VI_UnBind_VENC(VI_PIPE ViPipe, VI_CHN ViChn, VENC_CHN VencChn);
HI_S32 HIMPP_VPSS_Bind_AVS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, AVS_GRP AvsGrp, AVS_PIPE AvsPipe);
HI_S32 HIMPP_VPSS_UnBind_AVS(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, AVS_GRP AvsGrp, AVS_PIPE AvsPipe);
HI_S32 HIMPP_VPSS_Bind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
HI_S32 HIMPP_VPSS_UnBind_VO(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VO_LAYER VoLayer, VO_CHN VoChn);
HI_S32 HIMPP_VPSS_Bind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);
HI_S32 HIMPP_VPSS_UnBind_VENC(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VENC_CHN VencChn);

HI_VOID HIMPP_ISP_Stop(ISP_DEV IspDev);
HI_VOID HIMPP_All_ISP_Stop(HI_VOID);
HI_S32 HIMPP_ISP_Run(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_BindSns(ISP_DEV IspDev, HI_U32 u32SnsId, HIMPP_SNS_TYPE_E enSnsType, HI_S8 s8SnsDev);
HI_S32 HIMPP_ISP_Sensor_Regiter_callback(ISP_DEV IspDev, HI_U32 u32SnsId);
HI_S32 HIMPP_ISP_Sensor_UnRegiter_callback(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_Aelib_Callback(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_Aelib_UnCallback(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_Awblib_Callback(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_Awblib_UnCallback(ISP_DEV IspDev);
HI_S32 HIMPP_ISP_GetIspAttrBySns(HIMPP_SNS_TYPE_E enSnsType, ISP_PUB_ATTR_S *pstPubAttr);

HI_S32 HIMPP_VI_GetWDRModeBySensor(HIMPP_SNS_TYPE_E enMode, WDR_MODE_E *penWDRMode);
HI_S32 HIMPP_VI_GetPipeBySensor(HIMPP_SNS_TYPE_E enMode, HIMPP_PIPE_INFO_S *pstPipeInfo);
HI_S32 HIMPP_VI_GetSizeBySensor(HIMPP_SNS_TYPE_E enMode, PIC_SIZE_E *penSize);
HI_S32 HIMPP_VI_GetFrameRateBySensor(HIMPP_SNS_TYPE_E enMode, HI_U32 *pu32FrameRate);
HI_S32 HIMPP_VI_StartDev(HIMPP_VI_INFO_S *pstViInfo);
HI_S32 HIMPP_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32 HIMPP_VI_StartMIPI(HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32 HIMPP_VI_StartVi(HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32 HIMPP_VI_StopVi(HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32 HIMPP_VI_SetMipiAttr(HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32 HIMPP_VI_GetDevAttrBySns(HIMPP_SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr);
HI_VOID HIMPP_VI_GetSensorInfo(HIMPP_VI_CONFIG_S *pstViConfig);


combo_dev_t HIMPP_VI_GetComboDevBySensor(HIMPP_SNS_TYPE_E enMode, HI_S32 s32SnsIdx);
HI_S32 HIMPP_VI_SaveRaw(VIDEO_FRAME_S *pVBuf, HI_U32 u32Nbit, FILE *pfd);
HI_VOID *HIMPP_VI_DumpRaw(HI_VOID *arg);
HI_S32 HIMPP_VI_StartDumpRawThread(VI_PIPE ViPipe, HI_S32 s32Cnt, const HI_CHAR *pzsName);
HI_S32 HIMPP_VI_StopDumpRawThread(HI_VOID);
HI_S32 HIMPP_VI_SetParam(HIMPP_VI_CONFIG_S *pstViConfig);
HI_S32  HIMPP_VI_SwitchMode_StopVI(HIMPP_VI_CONFIG_S *pstViConfigSrc);
HI_S32  HIMPP_VI_SwitchMode(HIMPP_VI_CONFIG_S *pstViConfigDes);

HI_S32 HIMPP_VI_FpnCalibrateConfig(VI_PIPE ViPipe, HIMPP_VI_FPN_CALIBRATE_INFO_S *pstViFpnCalibrateInfo);
HI_S32 HIMPP_VI_FpnCorrectionConfig(VI_PIPE ViPipe, HIMPP_VI_FPN_CORRECTION_INFO_S *pstViFpnCorrectionInfo);
HI_S32 HIMPP_VI_DisableFpnCorrection(VI_PIPE ViPipe, HIMPP_VI_FPN_CORRECTION_INFO_S *pstViFpnCorrectionInfo);

HI_S32 HIMPP_VI_Load_UserPic(const char *pszYuvFile, VI_USERPIC_ATTR_S *pstUsrPic, HIMPP_VI_FRAME_INFO_S *pstViFrameInfo);
HI_VOID HIMPP_VI_Release_UserPic(HIMPP_VI_FRAME_INFO_S *pstViFrameInfo);

HI_S32 HIMPP_VPSS_Start(VPSS_GRP VpssGrp, HI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr, VPSS_CHN_ATTR_S *pastVpssChnAttr);
HI_S32 HIMPP_VPSS_WRAP_Start(VPSS_GRP VpssGrp, HI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr, VPSS_CHN_ATTR_S *pastVpssChnAttr, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap);
HI_S32 HIMPP_VPSS_Stop(VPSS_GRP VpssGrp, HI_BOOL *pabChnEnable);

HI_S32 HIMPP_VENC_MemConfig(HI_VOID);
HI_S32 HIMPP_VENC_Creat(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,  PIC_SIZE_E enSize, HIMPP_RC_E enRcMode, HI_U32  u32Profile, HI_BOOL bRcnRefShareBuf, VENC_GOP_ATTR_S *pstGopAttr);
HI_S32 HIMPP_VENC_Start(VENC_CHN VencChn, PAYLOAD_TYPE_E enType, PIC_SIZE_E enSize, HIMPP_RC_E enRcMode, HI_U32 u32Profile, HI_BOOL bRcnRefShareBuf, VENC_GOP_ATTR_S *pstGopAttr);
HI_S32 HIMPP_VENC_Stop(VENC_CHN VencChn);
HI_S32 HIMPP_VENC_SnapStart(VENC_CHN VencChn, SIZE_S *pstSize, HI_BOOL bSupportDCF);
HI_S32 HIMPP_VENC_SnapProcess(VENC_CHN VencChn, HI_U32 SnapCnt, HI_BOOL bSaveJpg, HI_BOOL bSaveThm);
HI_S32 HIMPP_VENC_SaveJpeg(VENC_CHN VencChn, HI_U32 SnapCnt);
HI_S32 HIMPP_VENC_SnapStop(VENC_CHN VencChn);
HI_S32 HIMPP_VENC_StartGetStream(VENC_CHN VeChn[], HI_S32 s32Cnt);
HI_S32 HIMPP_VENC_StopGetStream(void);
HI_S32 HIMPP_VENC_StartGetStream_Svc_t(HI_S32 s32Cnt);
HI_S32 HIMPP_VENC_GetGopAttr(VENC_GOP_MODE_E enGopMode, VENC_GOP_ATTR_S *pstGopAttr);
HI_S32 HIMPP_VENC_QpmapSendFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn[], VENC_CHN VeChn[], HI_S32 s32Cnt, SIZE_S stSize[]);
HI_S32 HIMPP_VENC_StopSendQpmapFrame(void);

HI_S32 HIMPP_VENC_H265_H264(void);
HI_S32 HIMPP_VENC_H265_H264_stop(void);

HI_S32 HIMPP_AUDIO_CreatTrdAiAo(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
HI_S32 HIMPP_AUDIO_CreatTrdAiAenc(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
HI_S32 HIMPP_AUDIO_CreatTrdAencAdec(AENC_CHN AeChn, ADEC_CHN AdChn, FILE *pAecFd);
HI_S32 HIMPP_AUDIO_CreatTrdFileAdec(ADEC_CHN AdChn, FILE *pAdcFd);
HI_S32 HIMPP_AUDIO_CreatTrdAoVolCtrl(AUDIO_DEV AoDev);
HI_S32 HIMPP_AUDIO_DestoryTrdAi(AUDIO_DEV AiDev, AI_CHN AiChn);
HI_S32 HIMPP_AUDIO_DestoryTrdAencAdec(AENC_CHN AeChn);
HI_S32 HIMPP_AUDIO_DestoryTrdFileAdec(ADEC_CHN AdChn);
HI_S32 HIMPP_AUDIO_DestoryTrdAoVolCtrl(AUDIO_DEV AoDev);
HI_S32 HIMPP_AUDIO_DestoryAllTrd(void);
HI_S32 HIMPP_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
HI_S32 HIMPP_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
HI_S32 HIMPP_AUDIO_AoBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
HI_S32 HIMPP_AUDIO_AoUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
HI_S32 HIMPP_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
HI_S32 HIMPP_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
HI_S32 HIMPP_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr);
HI_S32 HIMPP_AUDIO_StartAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
                                 AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enOutSampleRate, HI_BOOL bResampleEn, HI_VOID *pstAiVqeAttr, HI_U32 u32AiVqeType, AUDIO_DEV AoDevId);
HI_S32 HIMPP_AUDIO_StopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt, HI_BOOL bResampleEn, HI_BOOL bVqeEn);
HI_S32 HIMPP_AUDIO_StartAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt,
                                 AIO_ATTR_S *pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate, HI_BOOL bResampleEn);
HI_S32 HIMPP_AUDIO_StopAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, HI_BOOL bResampleEn);
HI_S32 HIMPP_AUDIO_StartAenc(HI_S32 s32AencChnCnt, AIO_ATTR_S *pstAioAttr, PAYLOAD_TYPE_E enType);
HI_S32 HIMPP_AUDIO_StopAenc(HI_S32 s32AencChnCnt);
HI_S32 HIMPP_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType);
HI_S32 HIMPP_AUDIO_StopAdec(ADEC_CHN AdChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __HIMPP_H__ */
