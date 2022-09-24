
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>

#include "hi_mipi.h"

#include "hi_common.h"
#include "himpp.h"

#define MIPI_DEV_NODE       "/dev/hi_mipi"

HIMPP_VI_DUMP_THREAD_INFO_S g_stViDumpRawThreadInfo;

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_SC4236_10BIT_3M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2304, 1296},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_SC3235_10BIT_3M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2304, 1296},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};


combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_SC4236_10BIT_3M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2304, 1296},
    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_SC3235_10BIT_3M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2304, 1296},
    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};


combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};


combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 1, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 2, -1, -1}
        }
    }
};


combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 4, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_SC2231_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_SC2231_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_F37_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_2lane_CHN1_SENSOR_F37_10BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_CMOS_SENSOR_SC2235_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_CMOS,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_DOL,
            {0, 1, 2, 3}
        }
    }
};

// my edit
combo_dev_attr_t MIPI_2lane_CHN0_SENSOR_OV5640_8BIT_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    // 手册中说明3516EV200不支持MIPI输入YUV
    // 自己摸索出来的，这里配置的宽度必须是实际宽度的2倍，否则图像缺失
    .img_rect = {0, 0, 3840, 1080},

    {
        .mipi_attr =
        {
   			DATA_TYPE_YUV422_8BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_5M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2592, 1944},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_5M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2592, 1944},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2592, 1520},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_4M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {12, 14, 2592, 1520},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_5M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2688, 1944},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_4M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2688, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_OS05A_10BIT_4M_WDR2TO1_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2688, 1536},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            HI_MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

combo_dev_attr_t BT1120_2M_30FPS_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_BT1120,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},
};

combo_dev_attr_t BT656_2M_30FPS_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_BT656,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},
};
combo_dev_attr_t BT601_2M_30FPS_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_BT601,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},
};
VI_DEV_ATTR_S DEV_ATTR_IMX327_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_IMX307_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_SC2231_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1920,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1080,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_F37_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1920,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1080,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_SC2235_2M_BASE =
{
    VI_MODE_DIGITAL_CAMERA,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_FIELD, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_NORM_PULSE, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1920,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1080,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_SC4236_3M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2304 , 1296},
    {
        {
            {2304 , 1296},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1296
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_SC3235_3M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2304 , 1296},
    {
        {
            {2304 , 1296},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1296
    },
    DATA_RATE_X1
};


VI_DEV_ATTR_S DEV_ATTR_GC2053_2M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};


VI_DEV_ATTR_S DEV_ATTR_IMX335_CUT_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {1920 , 1440},
    {
        {
            {1920, 1440},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1440
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_IMX335_5M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2592 , 1944},
    {
        {
            {2592 , 1944},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1944
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_IMX335_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2592 , 1520},
    {
        {
            {2592 , 1520},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1520
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_OS05A_5M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2688 , 1944},
    {
        {
            {2688 , 1944},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1944
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_OS05A_4M_BASE =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1280,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            720,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_RGB,
    HI_FALSE,
    {2688 , 1536},
    {
        {
            {2688 , 1536},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1536
    },
    DATA_RATE_X1
};
VI_DEV_ATTR_S DEV_ATTR_BT656_BASE =
{
    VI_MODE_BT656,
    VI_WORK_MODE_1Multiplex,
    {0x7f800000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_UYVY,
    {
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,
    {0,            1280,        0,
     0,            720,        0,
     0,            0,            0}
    },
    VI_DATA_TYPE_YUV,
    HI_FALSE,
    {720 , 288},
    {
        {
            {720 , 288},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_DEV_ATTR_S DEV_ATTR_BT601_BASE =
{
    VI_MODE_BT601,
    VI_WORK_MODE_1Multiplex,
    {0xFF000000,    0x0},
    VI_SCAN_PROGRESSIVE,
    {-1, -1, -1, -1},
    VI_DATA_SEQ_VYUY,

    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_VALID_SINGAL,VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    VI_DATA_TYPE_YUV,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};


VI_DEV_ATTR_S DEV_ATTR_BT1120_BASE =
{
    VI_MODE_BT1120_STANDARD,
    VI_WORK_MODE_1Multiplex,
    {0xFF000000,    0x00FF0000},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_UVUV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_YUV,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

// my edit
VI_DEV_ATTR_S DEV_ATTR_OV5640_8BIT_YUV_BASE =
{
    VI_MODE_MIPI_YUV422,            /* MIPI YUV422 mode */
    VI_WORK_MODE_1Multiplex,
    {0xFF000000,    0x00FF0000},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_UVUV,
    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_FIELD, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1920,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1080,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_YUV,
    HI_FALSE,
    {1920 , 1080},
    {
        {
            {1920, 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420 =
{
    /* bBindDev bYuvSkip */
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW12_420_3DNR_CHN0 =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_CHN0,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_1920x1440_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1440,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1944_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1944,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1944_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1944,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1520_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1520,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2592x1520_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2592, 1520,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2304x1296_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2304, 1296,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

VI_PIPE_ATTR_S PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

VI_PIPE_ATTR_S PIPE_ATTR_1280x720_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    1280, 720,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    HI_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2688x1944_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2688, 1944,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2688x1536_RAW12_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2688, 1536,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_12,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_2688x1536_RAW10_420_3DNR_RFR =
{
    VI_PIPE_BYPASS_NONE, HI_FALSE, HI_FALSE,
    2688, 1536,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_LINE,
    DATA_BITWIDTH_10,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

static VI_PIPE_ATTR_S PIPE_ATTR_BT1120_BASE =
{
    VI_PIPE_BYPASS_NONE, HI_TRUE, HI_TRUE,
    1920, 1080,
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_8,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    {-1, -1}
};

// my edit
static VI_PIPE_ATTR_S PIPE_ATTR_OV5640_1920x1080_YUV_8BIT =
{
    VI_PIPE_BYPASS_NONE, HI_TRUE, HI_TRUE,
    1920, 1080,
	PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_8,
    HI_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    HI_FALSE,
    { -1, -1}
};

VI_CHN_ATTR_S CHN_ATTR_1920x1080_422_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

VI_CHN_ATTR_S CHN_ATTR_1920x1080_420_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

VI_CHN_ATTR_S CHN_ATTR_1920x1080_400_SDR8_LINEAR =
{
    {1920, 1080},
    PIXEL_FORMAT_YUV_400,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_1920x1440_420_SDR8_LINEAR =
{
    {1920, 1440},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2592x1944_420_SDR8_LINEAR =
{
    {2592, 1944},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2592x1520_420_SDR8_LINEAR =
{
    {2592, 1520},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};


static VI_CHN_ATTR_S CHN_ATTR_2304x1296_420_SDR8_LINEAR =
{
    {2304, 1296},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2688x1944_420_SDR8_LINEAR =
{
    {2688, 1944},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S CHN_ATTR_2688x1536_420_SDR8_LINEAR =
{
    {2688, 1536},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

HI_BOOL IsSensorInput(HIMPP_SNS_TYPE_E enSnsType)
{
    HI_BOOL bRet = HI_TRUE;

    switch (enSnsType)
    {
        case HIMPP_SNS_TYPE_BUTT:
            bRet = HI_FALSE;
            break;

        default:
            break;
    }

    return bRet;
}


HI_S32 HIMPP_VI_ResetSns(sns_rst_source_t SnsDev)
{
    HI_S32 fd;
    HI_S32 s32Ret;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return -1;
    }

    s32Ret = ioctl(fd, HI_MIPI_RESET_SENSOR, &SnsDev);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HI_MIPI_SET_HS_MODE failed\n");
    }

    close(fd);
    return s32Ret;
}

HI_S32 HIMPP_VI_SetMipiHsMode(lane_divide_mode_t enHsMode)
{
    HI_S32 fd;
    HI_S32 s32Ret;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return -1;
    }

    s32Ret = ioctl(fd, HI_MIPI_SET_HS_MODE, &enHsMode);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HI_MIPI_SET_HS_MODE failed\n");
    }

    close(fd);
    return s32Ret;
}

HI_S32 HIMPP_VI_EnableMipiClock(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i = 0;
    HI_S32              s32ViNum = 0;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    combo_dev_t           devno = 0;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        devno = pstViInfo->stSnsInfo.MipiDev;

        s32Ret = ioctl(fd, HI_MIPI_ENABLE_MIPI_CLOCK, &devno);


        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("MIPI_ENABLE_CLOCK %d failed\n", devno);
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_DisableMipiClock(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i = 0;
    HI_S32              s32ViNum = 0;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    combo_dev_t           devno = 0;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        devno = pstViInfo->stSnsInfo.MipiDev;


        s32Ret = ioctl(fd, HI_MIPI_DISABLE_MIPI_CLOCK, &devno);


        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("MIPI_DISABLE_CLOCK %d failed\n", devno);
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_ResetMipi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i = 0;
    HI_S32              s32ViNum = 0;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    combo_dev_t           devno = 0;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        devno = pstViInfo->stSnsInfo.MipiDev;


        s32Ret = ioctl(fd, HI_MIPI_RESET_MIPI, &devno);


        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("RESET_MIPI %d failed\n", devno);
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_UnresetMipi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i = 0;
    HI_S32              s32ViNum = 0;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    combo_dev_t           devno = 0;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        devno = pstViInfo->stSnsInfo.MipiDev;


        s32Ret = ioctl(fd, HI_MIPI_UNRESET_MIPI, &devno);


        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("UNRESET_MIPI %d failed\n", devno);
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;

}

HI_S32 HIMPP_VI_EnableSensorClock(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    sns_clk_source_t       SnsDev = 0;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (SnsDev = 0; SnsDev < SNS_MAX_CLK_SOURCE_NUM; SnsDev++)
    {
        s32Ret = ioctl(fd, HI_MIPI_ENABLE_SENSOR_CLOCK, &SnsDev);

        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("HI_MIPI_ENABLE_SENSOR_CLOCK failed\n");
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_DisableSensorClock(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    sns_clk_source_t       SnsDev = 0;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (SnsDev = 0; SnsDev < SNS_MAX_CLK_SOURCE_NUM; SnsDev++)
    {
        s32Ret = ioctl(fd, HI_MIPI_DISABLE_SENSOR_CLOCK, &SnsDev);

        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("HI_MIPI_DISABLE_SENSOR_CLOCK failed\n");
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_ResetSensor(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    sns_rst_source_t       SnsDev = 0;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (SnsDev = 0; SnsDev < SNS_MAX_RST_SOURCE_NUM; SnsDev++)
    {
        s32Ret = ioctl(fd, HI_MIPI_RESET_SENSOR, &SnsDev);

        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("HI_MIPI_RESET_SENSOR failed\n");
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

HI_S32 HIMPP_VI_UnresetSensor(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    sns_rst_source_t       SnsDev = 0;

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (SnsDev = 0; SnsDev < SNS_MAX_RST_SOURCE_NUM; SnsDev++)
    {
        s32Ret = ioctl(fd, HI_MIPI_UNRESET_SENSOR, &SnsDev);

        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("HI_MIPI_UNRESET_SENSOR failed\n");
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}

/*
HIMPP_VI_GetMipiLaneMode

Hi3516EV200 Hi3518EV300 : return 0
Hi3516EV300             : return 1
InValid Chip            : return -1
*/
HI_S32 HIMPP_VI_GetMipiLaneMode(HI_VOID)
{
    HI_S32  s32Ret    = HI_SUCCESS;
    HI_U32  u32ChipId = 0;
    s32Ret = HI_MPI_SYS_GetChipId(&u32ChipId);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("not support this chip\n");
        return -1;
    }

    if (u32ChipId == HI3516E_V200 || u32ChipId == HI3518E_V300)
    {
        HIMPP_PRT("support this chip %x\n", u32ChipId);
        return 0;
    }
    else if (u32ChipId == HI3516E_V300 || u32ChipId == HI3516D_V200)
    {
        HIMPP_PRT("support this chip %x\n", u32ChipId);
        return 1;
    }
    else
    {
        HIMPP_PRT("not support this chip\n");
        return -1;
    }
}

/*
* Function name:HIMPP_VI_GetComboAttrBySns
* Description: Get combo_dev_attr_t by enSnsType and MipiDev
* The lane_id need reference hardware connection schematic of sensor and mipi.
* e.g. MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR
* If the hardware connection is that, sensor_lane0 is connected mipi_lane0 and
* sensor_lane1 is connected mipi_lane1 ...,  the array of lane_id is as follows.
combo_dev_attr_t MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            HI_MIPI_WDR_MODE_NONE,
//index  sensor_lane0      sensor_lane1    sensor_lane2    sensor_lane3
//             |                 |                |               |
             { 0,                1,               2,              3}
        }
    }
};
*/
HI_S32 HIMPP_VI_GetComboAttrBySns(HIMPP_SNS_TYPE_E enSnsType, combo_dev_t MipiDev, combo_dev_attr_t *pstComboAttr)
{
    switch (enSnsType)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }

            break;

        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            }

            break;

        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            break;
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_IMX327_10BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            }

            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX307_12BIT_2M_WDR2to1_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_5M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_5M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_12BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX335_10BIT_4M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_SC4236_10BIT_3M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_SC4236_10BIT_3M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            break;

        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_SC2231_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_SC2231_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }

            break;

        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_F37_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_F37_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }

            break;

        case SMART_SC2235_DC_2M_30FPS_10BIT:
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_CMOS_SENSOR_SC2235_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;

        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            if (HIMPP_VI_GetMipiLaneMode() == 1)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_SC3235_10BIT_3M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            else if (HIMPP_VI_GetMipiLaneMode() == 0)
            {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_SC3235_10BIT_3M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            break;


        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            if (HIMPP_VI_GetMipiLaneMode() == 1) {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            } else if (HIMPP_VI_GetMipiLaneMode() == 0) {
                hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN1_SENSOR_GC2053_10BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            }
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_5M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;
        }
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS05A_12BIT_4M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;
        }
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_OS05A_10BIT_4M_WDR2TO1_ATTR, sizeof(combo_dev_attr_t));
            break;
        }
		// my edit
        case OV5640_YUV_MIPI_2M_30FPS_8BIT: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_2lane_CHN0_SENSOR_OV5640_8BIT_NOWDR_ATTR, sizeof(combo_dev_attr_t));
            break;
		}
        case BT1120_2M_30FPS_8BIT: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &BT1120_2M_30FPS_ATTR, sizeof(combo_dev_attr_t));
            break;
        }
        case BT656_2M_30FPS_8BIT: {
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &BT656_2M_30FPS_ATTR, sizeof(combo_dev_attr_t));
            break;
        }

        default:
            HIMPP_PRT("not support enSnsType: %d\n", enSnsType);
            hi_memcpy(pstComboAttr, sizeof(combo_dev_attr_t), &MIPI_4lane_CHN0_SENSOR_IMX327_12BIT_2M_NOWDR_ATTR, sizeof(combo_dev_attr_t));
    }


    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_SetMipiAttr(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i = 0;
    HI_S32              s32ViNum = 0;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_S32              fd;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;
    combo_dev_attr_t    stcomboDevAttr;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    fd = open(MIPI_DEV_NODE, O_RDWR);

    if (fd < 0)
    {
        HIMPP_PRT("open hi_mipi dev failed\n");
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        HIMPP_VI_GetComboAttrBySns(pstViInfo->stSnsInfo.enSnsType, pstViInfo->stSnsInfo.MipiDev, &stcomboDevAttr);
        stcomboDevAttr.devno = pstViInfo->stSnsInfo.MipiDev;

        HIMPP_PRT("============= MipiDev %d, SetMipiAttr enWDRMode: %d\n", pstViInfo->stSnsInfo.MipiDev, pstViInfo->stDevInfo.enWDRMode);

        s32Ret = ioctl(fd, HI_MIPI_SET_DEV_ATTR, &stcomboDevAttr);

        if (HI_SUCCESS != s32Ret)
        {
            HIMPP_PRT("MIPI_SET_DEV_ATTR failed\n");
            goto EXIT;
        }
    }

EXIT:
    close(fd);

    return s32Ret;
}


HI_U32 HIMPP_VI_GetMipiLaneDivideMode(HIMPP_VI_CONFIG_S *pstViConfig)
{
    lane_divide_mode_t lane_divide_mode;
    lane_divide_mode = LANE_DIVIDE_MODE_0;

    return (HI_U32)lane_divide_mode;
}

/*****************************************************************************
* function : init mipi
*****************************************************************************/
HI_S32 HIMPP_VI_StartMIPI(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
    lane_divide_mode_t lane_divide_mode = LANE_DIVIDE_MODE_0;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    lane_divide_mode = HIMPP_VI_GetMipiLaneDivideMode(pstViConfig);

    s32Ret = HIMPP_VI_SetMipiHsMode(lane_divide_mode);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_SetMipiHsMode failed!\n");

        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_EnableMipiClock(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_EnableMipiClock failed!\n");

        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_ResetMipi(pstViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_ResetMipi failed!\n");

        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_EnableSensorClock(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_EnableSensorClock failed!\n");

        return HI_FAILURE;
    }

    if (pstViConfig->astViInfo[0].stSnsInfo.enSnsType != BT656_2M_30FPS_8BIT)
    {
        s32Ret = HIMPP_VI_ResetSensor(pstViConfig);
        if (HI_SUCCESS != s32Ret) {
            HIMPP_PRT("HIMPP_VI_ResetSensor failed!\n");

            return HI_FAILURE;
        }
    }
    s32Ret = HIMPP_VI_SetMipiAttr(pstViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_SetMipiAttr failed!\n");

        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_UnresetMipi(pstViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_UnresetMipi failed!\n");

        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_UnresetSensor(pstViConfig);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_UnresetSensor failed!\n");

        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_StopMIPI(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
    if (pstViConfig->astViInfo[0].stSnsInfo.enSnsType != BT656_2M_30FPS_8BIT) {
        s32Ret = HIMPP_VI_ResetSensor(pstViConfig);
        if (HI_SUCCESS != s32Ret) {
            HIMPP_PRT("HIMPP_VI_ResetSensor failed!\n");

            return HI_FAILURE;
        }

        s32Ret = HIMPP_VI_DisableSensorClock(pstViConfig);
        if (HI_SUCCESS != s32Ret) {
            HIMPP_PRT("HIMPP_VI_DisableSensorClock failed!\n");

            return HI_FAILURE;
        }
    }
    s32Ret = HIMPP_VI_ResetMipi(pstViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("HIMPP_VI_ResetMipi failed!\n");

        return HI_FAILURE;
    }
    s32Ret = HIMPP_VI_DisableMipiClock(pstViConfig);
    if (HI_SUCCESS != s32Ret) {
        HIMPP_PRT("HIMPP_VI_DisableMipiClock failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_UpdateVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
    HI_S32   i = 0;

    if (VI_OFFLINE_VPSS_OFFLINE == pstVIVPSSMode->aenMode[0])
    {
        for (i = 0; i < VI_MAX_PIPE_NUM; i++)
        {
            if (VI_OFFLINE_VPSS_ONLINE == pstVIVPSSMode->aenMode[i])
            {
                for (i = 0; i < VI_MAX_PIPE_NUM; i++)
                {
                    pstVIVPSSMode->aenMode[i] = VI_OFFLINE_VPSS_ONLINE;
                }

                break;
            }
        }
    }
    else if (VI_OFFLINE_VPSS_ONLINE == pstVIVPSSMode->aenMode[0])
    {
        for (i = 0; i < VI_MAX_PIPE_NUM; i++)
        {
            pstVIVPSSMode->aenMode[i] = pstVIVPSSMode->aenMode[0];
        }
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_SetParam(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i;
    HI_S32              s32ViNum;
    HI_S32              s32Ret;
    VI_PIPE             ViPipe;
    VI_VPSS_MODE_S      stVIVPSSMode;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_SYS_GetVIVPSSMode(&stVIVPSSMode);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("Get VI-VPSS mode Param failed with %#x!\n", s32Ret);

        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];
        ViPipe    = pstViInfo->stPipeInfo.aPipe[0];
        stVIVPSSMode.aenMode[ViPipe] = pstViInfo->stPipeInfo.enMastPipeMode;

        if ((pstViInfo->stPipeInfo.bMultiPipe == HI_TRUE)
            || (VI_OFFLINE_VPSS_ONLINE == pstViInfo->stPipeInfo.enMastPipeMode))
        {
            HIMPP_VI_UpdateVIVPSSMode(&stVIVPSSMode);

            ViPipe = pstViInfo->stPipeInfo.aPipe[1];
            if (ViPipe != -1)
            {
                stVIVPSSMode.aenMode[ViPipe] = pstViInfo->stPipeInfo.enMastPipeMode;
            }
        }

        if ((pstViInfo->stSnapInfo.bSnap) && (pstViInfo->stSnapInfo.bDoublePipe))
        {
            ViPipe    = pstViInfo->stPipeInfo.aPipe[1];
            if (ViPipe != -1)
            {
                stVIVPSSMode.aenMode[ViPipe] = pstViInfo->stSnapInfo.enSnapPipeMode;
            }
        }
    }
    s32Ret = HI_MPI_SYS_SetVIVPSSMode(&stVIVPSSMode);

    if (HI_SUCCESS != s32Ret)
    {
        HIMPP_PRT("Set VI-VPSS mode Param failed with %#x!\n", s32Ret);

        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


HI_S32 HIMPP_VI_GetDevAttrBySns(HIMPP_SNS_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr)
{
    switch (enSnsType)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX307_2M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX307_2M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_5M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_5M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_4M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX335_4M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_SC4236_3M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_SC2231_2M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_F37_2M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_SC3235_3M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;

        case SMART_SC2235_DC_2M_30FPS_10BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_SC2235_2M_BASE, sizeof(VI_DEV_ATTR_S));
            break;

        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_GC2053_2M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS05A_5M_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS05A_4M_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OS05A_4M_BASE, sizeof(VI_DEV_ATTR_S));
            pstViDevAttr->au32ComponentMask[0] = 0xFFC00000;
            break;
		// my edit
        case OV5640_YUV_MIPI_2M_30FPS_8BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_OV5640_8BIT_YUV_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        case BT1120_2M_30FPS_8BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_BT1120_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        case BT656_2M_30FPS_8BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_BT656_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        case BT601_2M_30FPS_8BIT:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_BT601_BASE, sizeof(VI_DEV_ATTR_S));
            break;
        default:
            hi_memcpy(pstViDevAttr, sizeof(VI_DEV_ATTR_S), &DEV_ATTR_IMX327_2M_BASE, sizeof(VI_DEV_ATTR_S));
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_GetPipeAttrBySns(HIMPP_SNS_TYPE_E enSnsType, VI_PIPE_ATTR_S *pstPipeAttr)
{
    switch (enSnsType)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_10BPP;
            pstPipeAttr->enBitWidth = DATA_BITWIDTH_10;
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            pstPipeAttr->enPixFmt = PIXEL_FORMAT_RGB_BAYER_10BPP;
            pstPipeAttr->enBitWidth = DATA_BITWIDTH_10;
            break;

        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1944_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1944_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1520_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2592x1520_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2304x1296_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2304x1296_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case SMART_SC2235_DC_2M_30FPS_10BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;

        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2688x1944_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2688x1536_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_2688x1536_RAW10_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
            break;
		
		// my edit
        case OV5640_YUV_MIPI_2M_30FPS_8BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_OV5640_1920x1080_YUV_8BIT, sizeof(VI_PIPE_ATTR_S));
            break;

        case BT1120_2M_30FPS_8BIT:
        case BT601_2M_30FPS_8BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_BT1120_BASE, sizeof(VI_PIPE_ATTR_S));
            break;
        case BT656_2M_30FPS_8BIT:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_BT1120_BASE, sizeof(VI_PIPE_ATTR_S));
            pstPipeAttr->u32MaxH = 288;
            pstPipeAttr->u32MaxW = 720;
            break;
        default:
            hi_memcpy(pstPipeAttr, sizeof(VI_PIPE_ATTR_S), &PIPE_ATTR_1920x1080_RAW12_420_3DNR_RFR, sizeof(VI_PIPE_ATTR_S));
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_GetChnAttrBySns(HIMPP_SNS_TYPE_E enSnsType, VI_CHN_ATTR_S *pstChnAttr)
{
    switch (enSnsType)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case BT1120_2M_30FPS_8BIT:
        case BT601_2M_30FPS_8BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;

        case BT656_2M_30FPS_8BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            pstChnAttr->stSize.u32Width = 720;
            pstChnAttr->stSize.u32Height= 288;
            break;
        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
        case SMART_SC2235_DC_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;

        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2592x1944_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;

        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2592x1520_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;
        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2592x1520_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;

        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2304x1296_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;

        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2304x1296_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2688x1944_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_2688x1536_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;
		// my edit
        case OV5640_YUV_MIPI_2M_30FPS_8BIT:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
            break;
        default:
            hi_memcpy(pstChnAttr, sizeof(VI_CHN_ATTR_S), &CHN_ATTR_1920x1080_420_SDR8_LINEAR, sizeof(VI_CHN_ATTR_S));
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_StartDev(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              s32Ret;
    VI_DEV              ViDev;
    HIMPP_SNS_TYPE_E    enSnsType;
    VI_DEV_ATTR_S       stViDevAttr;

    ViDev       = pstViInfo->stDevInfo.ViDev;
    enSnsType    = pstViInfo->stSnsInfo.enSnsType;

    HIMPP_VI_GetDevAttrBySns(enSnsType, &stViDevAttr);
    stViDevAttr.stWDRAttr.enWDRMode = pstViInfo->stDevInfo.enWDRMode;
    if (VI_PARALLEL_VPSS_OFFLINE == pstViInfo->stPipeInfo.enMastPipeMode || VI_PARALLEL_VPSS_PARALLEL == pstViInfo->stPipeInfo.enMastPipeMode)
    {
        stViDevAttr.enDataRate = DATA_RATE_X2;
    }

    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_StopDev(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32 s32Ret;
    VI_DEV ViDev;

    ViDev   = pstViInfo->stDevInfo.ViDev;
    s32Ret  = HI_MPI_VI_DisableDev(ViDev);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_DisableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_BindPipeDev(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              i;
    HI_S32              s32PipeCnt = 0;
    HI_S32              s32Ret;
    VI_DEV_BIND_PIPE_S  stDevBindPipe = {0};

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            stDevBindPipe.PipeId[s32PipeCnt] = pstViInfo->stPipeInfo.aPipe[i];
            s32PipeCnt++;
            stDevBindPipe.u32Num = s32PipeCnt;
        }
    }

    s32Ret = HI_MPI_VI_SetDevBindPipe(pstViInfo->stDevInfo.ViDev, &stDevBindPipe);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_SetDevBindPipe failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}

static HI_S32 HIMPP_VI_ModeSwitchCreateSingleViPipe(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VI_CreatePipe(ViPipe, pstPipeAttr);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_CreatePipe failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


static HI_S32 HIMPP_VI_ModeSwitch_EnableSingleViPipe(VI_PIPE ViPipe, VI_PIPE_ATTR_S *pstPipeAttr)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VI_StartPipe(ViPipe);

    if (s32Ret != HI_SUCCESS)
    {
        HI_MPI_VI_DestroyPipe(ViPipe);
        HIMPP_PRT("HI_MPI_VI_StartPipe failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


static HI_S32 HIMPP_VI_StopSingleViPipe(VI_PIPE ViPipe)
{
    HI_S32  s32Ret;

    s32Ret = HI_MPI_VI_StopPipe(ViPipe);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_StopPipe failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_DestroyPipe(ViPipe);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HI_MPI_VI_DestroyPipe failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return s32Ret;
}


HI_S32 HIMPP_VI_ModeSwitch_StartViPipe(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32          i, j;
    HI_S32          s32Ret = HI_SUCCESS;
    VI_PIPE         ViPipe;
    VI_PIPE_ATTR_S  stPipeAttr;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            HIMPP_VI_GetPipeAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPipeAttr);
            {
                s32Ret = HIMPP_VI_ModeSwitchCreateSingleViPipe(ViPipe, &stPipeAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HIMPP_VI_StartSingleViPipe  %d failed!\n", ViPipe);
                    goto EXIT;
                }
            }

        }
    }

    return s32Ret;

EXIT:

    for (j = 0; j < i; j++)
    {
        ViPipe = j;
        HIMPP_VI_StopSingleViPipe(ViPipe);
    }

    return s32Ret;
}


HI_S32 HIMPP_VI_ModeSwitch_EnableViPipe(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32          i, j;
    HI_S32          s32Ret = HI_SUCCESS;
    VI_PIPE         ViPipe;
    VI_PIPE_ATTR_S  stPipeAttr;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            HIMPP_VI_GetPipeAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPipeAttr);
            {
                s32Ret = HIMPP_VI_ModeSwitch_EnableSingleViPipe(ViPipe, &stPipeAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HIMPP_VI_StartSingleViPipe  %d failed!\n", ViPipe);
                    goto EXIT;
                }
            }

        }
    }

    return s32Ret;

EXIT:

    for (j = 0; j < i; j++)
    {
        ViPipe = j;
        HIMPP_VI_StopSingleViPipe(ViPipe);
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_StartViPipe(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32          i, j;
    HI_S32          s32Ret = HI_SUCCESS;
    VI_PIPE         ViPipe;
    VI_PIPE_ATTR_S  stPipeAttr;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            HIMPP_VI_GetPipeAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPipeAttr);
            if (HI_TRUE == pstViInfo->stPipeInfo.bIspBypass)
            {
                stPipeAttr.bIspBypass = HI_TRUE;
                stPipeAttr.enPixFmt   = pstViInfo->stPipeInfo.enPixFmt;
                stPipeAttr.enBitWidth = DATA_BITWIDTH_8;
            }

            stPipeAttr.enCompressMode = COMPRESS_MODE_NONE;
            if ((pstViInfo->stSnapInfo.bSnap) && (pstViInfo->stSnapInfo.bDoublePipe) && (ViPipe == pstViInfo->stSnapInfo.SnapPipe))
            {
                s32Ret = HI_MPI_VI_CreatePipe(ViPipe, &stPipeAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HI_MPI_VI_CreatePipe failed with %#x!\n", s32Ret);
                    goto EXIT;
                }
            }
            else
            {
                s32Ret = HI_MPI_VI_CreatePipe(ViPipe, &stPipeAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HI_MPI_VI_CreatePipe failed with %#x!\n", s32Ret);
                    return HI_FAILURE;
                }

                if (HI_TRUE == pstViInfo->stPipeInfo.bVcNumCfged)
                {
                    s32Ret = HI_MPI_VI_SetPipeVCNumber(ViPipe, pstViInfo->stPipeInfo.u32VCNum[i]);
                    if (s32Ret != HI_SUCCESS)
                    {
                        HI_MPI_VI_DestroyPipe(ViPipe);
                        HIMPP_PRT("HI_MPI_VI_SetPipeVCNumber failed with %#x!\n", s32Ret);
                        return HI_FAILURE;
                    }
                }

                s32Ret = HI_MPI_VI_StartPipe(ViPipe);
                if (s32Ret != HI_SUCCESS)
                {
                    HI_MPI_VI_DestroyPipe(ViPipe);
                    HIMPP_PRT("HI_MPI_VI_StartPipe failed with %#x!\n", s32Ret);
                    return HI_FAILURE;
                }
            }

        }
    }

    return s32Ret;

EXIT:

    for (j = 0; j < i; j++)
    {
        ViPipe = j;
        HIMPP_VI_StopSingleViPipe(ViPipe);
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_StopViPipe(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32  i;
    VI_PIPE ViPipe;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            HIMPP_VI_StopSingleViPipe(ViPipe);
        }
    }

    return HI_SUCCESS;
}


HI_S32 HIMPP_VI_ModeSwitch_StartViChn(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              i;
    HI_BOOL             bNeedChn;
    HI_S32              s32Ret = HI_SUCCESS;
    VI_PIPE             ViPipe;
    VI_CHN              ViChn;
    VI_CHN_ATTR_S       stChnAttr;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            ViChn  = pstViInfo->stChnInfo.ViChn;

            HIMPP_VI_GetChnAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stChnAttr);
            stChnAttr.enDynamicRange = pstViInfo->stChnInfo.enDynamicRange;
            stChnAttr.enVideoFormat  = pstViInfo->stChnInfo.enVideoFormat;
            stChnAttr.enPixelFormat  = pstViInfo->stChnInfo.enPixFormat;
            stChnAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;

            if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode)
            {
                bNeedChn = HI_TRUE;
            }
            else
            {
                bNeedChn = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (bNeedChn)
            {
                s32Ret = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HI_MPI_VI_SetChnAttr failed with %#x!\n", s32Ret);
                    return HI_FAILURE;
                }
            }
        }
    }

    return s32Ret;
}
HI_S32 HIMPP_VI_StartViChn(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              i;
    HI_BOOL             bNeedChn;
    HI_S32              s32Ret = HI_SUCCESS;
    VI_PIPE             ViPipe;
    VI_CHN              ViChn;
    VI_CHN_ATTR_S       stChnAttr;
    VI_VPSS_MODE_E      enMastPipeMode;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            ViChn  = pstViInfo->stChnInfo.ViChn;

            HIMPP_VI_GetChnAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stChnAttr);
            stChnAttr.enDynamicRange = pstViInfo->stChnInfo.enDynamicRange;
            stChnAttr.enVideoFormat  = pstViInfo->stChnInfo.enVideoFormat;
            stChnAttr.enPixelFormat  = pstViInfo->stChnInfo.enPixFormat;
            stChnAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;

            if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode)
            {
                bNeedChn = HI_TRUE;
            }
            else
            {
                bNeedChn = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (bNeedChn)
            {
                s32Ret = HI_MPI_VI_SetChnAttr(ViPipe, ViChn, &stChnAttr);

                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("HI_MPI_VI_SetChnAttr failed with %#x!\n", s32Ret);
                    return HI_FAILURE;
                }

                enMastPipeMode = pstViInfo->stPipeInfo.enMastPipeMode;

                if (VI_OFFLINE_VPSS_OFFLINE == enMastPipeMode
                    || VI_ONLINE_VPSS_OFFLINE == enMastPipeMode
                    || VI_PARALLEL_VPSS_OFFLINE == enMastPipeMode)
                {
                    s32Ret = HI_MPI_VI_EnableChn(ViPipe, ViChn);

                    if (s32Ret != HI_SUCCESS)
                    {
                        HIMPP_PRT("HI_MPI_VI_EnableChn failed with %#x!\n", s32Ret);
                        return HI_FAILURE;
                    }
                }
            }
        }
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_StopViChn(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              i;
    HI_BOOL             bNeedChn;
    HI_S32              s32Ret = HI_SUCCESS;
    VI_PIPE             ViPipe;
    VI_CHN              ViChn;
    VI_VPSS_MODE_E      enMastPipeMode;


    for (i = 0; i < 4; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe = pstViInfo->stPipeInfo.aPipe[i];
            ViChn  = pstViInfo->stChnInfo.ViChn;

            if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode)
            {
                bNeedChn = HI_TRUE;
            }
            else
            {
                bNeedChn = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (bNeedChn)
            {
                enMastPipeMode = pstViInfo->stPipeInfo.enMastPipeMode;

                if (VI_OFFLINE_VPSS_OFFLINE == enMastPipeMode
                    || VI_ONLINE_VPSS_OFFLINE == enMastPipeMode
                    || VI_PARALLEL_VPSS_OFFLINE == enMastPipeMode)
                {
                    s32Ret = HI_MPI_VI_DisableChn(ViPipe, ViChn);

                    if (s32Ret != HI_SUCCESS)
                    {
                        HIMPP_PRT("HI_MPI_VI_DisableChn failed with %#x!\n", s32Ret);
                        return HI_FAILURE;
                    }
                }
            }
        }
    }

    return s32Ret;
}

static HI_S32 HIMPP_VI_CreateSingleVi(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HIMPP_VI_StartDev(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartDev failed !\n");
        return HI_FAILURE;
    }

    //we should bind pipe,then creat pipe
    s32Ret = HIMPP_VI_BindPipeDev(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_BindPipeDev failed !\n");
        goto EXIT1;
    }

    s32Ret = HIMPP_VI_StartViPipe(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViPipe failed !\n");
        goto EXIT1;
    }

    s32Ret = HIMPP_VI_StartViChn(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViChn failed !\n");
        goto EXIT2;
    }

    return HI_SUCCESS;

EXIT2:
    HIMPP_VI_StopViPipe(pstViInfo);

EXIT1:
    HIMPP_VI_StopDev(pstViInfo);


    return s32Ret;
}



static HI_S32 HIMPP_ModeSwitch_VI_CreateSingleVi(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HIMPP_VI_StartDev(pstViInfo);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartDev failed !\n");
        return HI_FAILURE;
    }

    //we should bind pipe,then creat pipe
    s32Ret = HIMPP_VI_BindPipeDev(pstViInfo);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_BindPipeDev failed !\n");
        goto EXIT1;
    }


    s32Ret = HIMPP_VI_ModeSwitch_StartViPipe(pstViInfo);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViPipe failed !\n");
        goto EXIT1;
    }

    s32Ret = HIMPP_VI_ModeSwitch_StartViChn(pstViInfo);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViChn failed !\n");
        goto EXIT2;
    }


    return HI_SUCCESS;

EXIT2:
    HIMPP_VI_StopViPipe(pstViInfo);

EXIT1:
    HIMPP_VI_StopDev(pstViInfo);


    return s32Ret;
}


static HI_S32 HIMPP_VI_StartPipe_Chn(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;


    s32Ret = HIMPP_VI_ModeSwitch_EnableViPipe(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViPipe failed !\n");
        goto EXIT1;
    }

    s32Ret = HIMPP_VI_StartViChn(pstViInfo);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartViChn failed !\n");
        goto EXIT2;
    }

    return HI_SUCCESS;

EXIT2:
    HIMPP_VI_StopViPipe(pstViInfo);

EXIT1:
    HIMPP_VI_StopDev(pstViInfo);


    return s32Ret;
}




static HI_S32 HIMPP_VI_DestroySingleVi(HIMPP_VI_INFO_S *pstViInfo)
{
    HIMPP_VI_StopViChn(pstViInfo);

    HIMPP_VI_StopViPipe(pstViInfo);

    HIMPP_VI_StopDev(pstViInfo);

    return HI_SUCCESS;
}


static HI_S32 HIMPP_VI_DestroySinglePipe_Chn(HIMPP_VI_INFO_S *pstViInfo)
{
    HIMPP_VI_StopViChn(pstViInfo);

    HIMPP_VI_StopViPipe(pstViInfo);

    return HI_SUCCESS;
}



HI_S32 HIMPP_VI_CreateVi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i, j;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        s32Ret = HIMPP_VI_CreateSingleVi(pstViInfo);

        if (s32Ret != HI_SUCCESS)
        {
            HIMPP_PRT("HIMPP_VI_CreateSingleVi failed !\n");
            goto EXIT;
        }
    }

    return HI_SUCCESS;
EXIT:

    for (j = 0; j < i; j++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[j];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        HIMPP_VI_DestroySingleVi(pstViInfo);
    }

    return s32Ret;
}


HI_S32 HIMPP_ModeSwitch_VI_CreateVi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i, j;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        s32Ret = HIMPP_ModeSwitch_VI_CreateSingleVi(pstViInfo);

        if (s32Ret != HI_SUCCESS)
        {
            HIMPP_PRT("HIMPP_VI_CreateSingleVi failed !\n");
            goto EXIT;
        }
    }

    return HI_SUCCESS;
EXIT:

    for (j = 0; j < i; j++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[j];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        HIMPP_VI_DestroySingleVi(pstViInfo);
    }

    return s32Ret;
}

HI_S32 HIMPP_ModeSwitch_VI_StartPipe_Chn(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i, j;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        s32Ret = HIMPP_VI_StartPipe_Chn(pstViInfo);

        if (s32Ret != HI_SUCCESS)
        {
            HIMPP_PRT("HIMPP_VI_CreateSingleVi failed !\n");
            goto EXIT;
        }
    }

    return HI_SUCCESS;
EXIT:

    for (j = 0; j < i; j++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[j];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        HIMPP_VI_DestroySinglePipe_Chn(pstViInfo);
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_DestroyVi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32            i;
    HI_S32            s32ViNum;
    HIMPP_VI_INFO_S *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        HIMPP_VI_DestroySingleVi(pstViInfo);
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_StartIsp(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32              i;
    HI_BOOL             bNeedPipe;
    HI_S32              s32Ret = HI_SUCCESS;
    VI_PIPE             ViPipe;
    HI_U32              u32SnsId;
    ISP_PUB_ATTR_S      stPubAttr;
    VI_PIPE_ATTR_S      stPipeAttr;

    HIMPP_VI_GetPipeAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPipeAttr);
    if (VI_PIPE_BYPASS_BE == stPipeAttr.enPipeBypassMode)
    {
        return HI_SUCCESS;
    }

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe      = pstViInfo->stPipeInfo.aPipe[i];
            u32SnsId    = pstViInfo->stSnsInfo.s32SnsId;

            HIMPP_ISP_GetIspAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPubAttr);
            stPubAttr.enWDRMode = pstViInfo->stDevInfo.enWDRMode;

            if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode)
            {
                bNeedPipe = HI_TRUE;
            }
            else
            {
                bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (HI_TRUE != bNeedPipe)
            {
                continue;
            }

            s32Ret = HIMPP_ISP_Sensor_Regiter_callback(ViPipe, u32SnsId);

            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("register sensor %d to ISP %d failed\n", u32SnsId, ViPipe);
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            if (((pstViInfo->stSnapInfo.bDoublePipe) && (pstViInfo->stSnapInfo.SnapPipe == ViPipe))
                || (pstViInfo->stPipeInfo.bMultiPipe && i > 0))
            {
                s32Ret = HIMPP_ISP_BindSns(ViPipe, u32SnsId, pstViInfo->stSnsInfo.enSnsType, -1);

                if (HI_SUCCESS != s32Ret)
                {
                    HIMPP_PRT("register sensor %d bus id %d failed\n", u32SnsId, pstViInfo->stSnsInfo.s32BusId);
                    HIMPP_ISP_Stop(ViPipe);
                    return HI_FAILURE;
                }
            }
            else
            {
                s32Ret = HIMPP_ISP_BindSns(ViPipe, u32SnsId, pstViInfo->stSnsInfo.enSnsType, pstViInfo->stSnsInfo.s32BusId);

                if (HI_SUCCESS != s32Ret)
                {
                    HIMPP_PRT("register sensor %d bus id %d failed\n", u32SnsId, pstViInfo->stSnsInfo.s32BusId);
                    HIMPP_ISP_Stop(ViPipe);
                    return HI_FAILURE;
                }
            }
            s32Ret = HIMPP_ISP_Aelib_Callback(ViPipe);

            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("HIMPP_ISP_Aelib_Callback failed\n");
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            s32Ret = HIMPP_ISP_Awblib_Callback(ViPipe);

            if (HI_SUCCESS != s32Ret)
            {
                HIMPP_PRT("HIMPP_ISP_Awblib_Callback failed\n");
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_MemInit(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("Init Ext memory failed with %#x!\n", s32Ret);
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);

            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("SetPubAttr failed with %#x!\n", s32Ret);
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            s32Ret = HI_MPI_ISP_Init(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("ISP Init failed with %#x!\n", s32Ret);
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }

            s32Ret = HIMPP_ISP_Run(ViPipe);

            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("ISP Run failed with %#x!\n", s32Ret);
                HIMPP_ISP_Stop(ViPipe);
                return HI_FAILURE;
            }
        }
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_StopIsp(HIMPP_VI_INFO_S *pstViInfo)
{
    HI_S32  i;
    HI_BOOL bNeedPipe;
    VI_PIPE ViPipe;

    for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
    {
        if (pstViInfo->stPipeInfo.aPipe[i] >= 0  && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
        {
            ViPipe    = pstViInfo->stPipeInfo.aPipe[i];

            if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode)
            {
                bNeedPipe = HI_TRUE;
            }
            else
            {
                bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
            }

            if (HI_TRUE != bNeedPipe)
            {
                continue;
            }

            HIMPP_ISP_Stop(ViPipe);
        }
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_CreateIsp(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;

    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];



        s32Ret = HIMPP_VI_StartIsp(pstViInfo);

        if (s32Ret != HI_SUCCESS)
        {
            HIMPP_PRT("HIMPP_VI_StartIsp failed !\n");
            return HI_FAILURE;
        }



    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_DestroyIsp(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];



        s32Ret = HIMPP_VI_StopIsp(pstViInfo);

        if (s32Ret != HI_SUCCESS)
        {
            HIMPP_PRT("HIMPP_VI_StopIsp failed !\n");
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_S32 HIMPP_VI_StartVi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_StartMIPI(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartMIPI failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_SetParam(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_SetParam failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_CreateVi(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_CreateVi failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_CreateIsp(pstViConfig);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_VI_DestroyVi(pstViConfig);
        HIMPP_PRT("HIMPP_VI_CreateIsp failed!\n");
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_StopVi(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HIMPP_VI_DestroyIsp(pstViConfig);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_DestroyIsp failed !\n");
        return HI_FAILURE;
    }

#if 0
    else
    {
        if (HI_FALSE == pstViConfig->bSwitch)
        {
            s32Ret = HIMPP_VI_DestroyIsp(pstViConfig);

            if (s32Ret != HI_SUCCESS)
            {
                HIMPP_PRT("HIMPP_VI_DestroyIsp failed !\n");
                return HI_FAILURE;
            }
        }
    }
#endif

    s32Ret = HIMPP_VI_DestroyVi(pstViConfig);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_DestroyVi failed !\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_StopMIPI(pstViConfig);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StopMIPI failed !\n");
        return HI_FAILURE;
    }

    return s32Ret;
}

HI_S32 HIMPP_VI_SwitchISPMode(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32              i;
    HI_S32              s32ViNum;
    HI_S32              s32Ret = HI_SUCCESS;
    HI_BOOL             bNeedPipe;
    VI_PIPE             ViPipe = 0;
    ISP_PUB_ATTR_S      stPubAttr;
    HIMPP_VI_INFO_S   *pstViInfo = HI_NULL;
    ISP_INNER_STATE_INFO_S stInnerStateInfo;

    if (!pstViConfig)
    {
        HIMPP_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    for (i = 0; i < pstViConfig->s32WorkingViNum; i++)
    {
        s32ViNum  = pstViConfig->as32WorkingViId[i];
        pstViInfo = &pstViConfig->astViInfo[s32ViNum];

        for (i = 0; i < WDR_MAX_PIPE_NUM; i++)
        {
            if ( pstViInfo->stPipeInfo.aPipe[i] >= 0 && pstViInfo->stPipeInfo.aPipe[i] < VI_MAX_PIPE_NUM)
            {

                HIMPP_ISP_GetIspAttrBySns(pstViInfo->stSnsInfo.enSnsType, &stPubAttr);

                stPubAttr.enWDRMode =  pstViInfo->stDevInfo.enWDRMode;

                HIMPP_PRT("HIMPP_VI_CreateIsp enWDRMode is %d!\n", stPubAttr.enWDRMode);

                if (WDR_MODE_NONE == pstViInfo->stDevInfo.enWDRMode )
                {
                    bNeedPipe = HI_TRUE;
                }
                else
                {
                    bNeedPipe = (i > 0) ? HI_FALSE : HI_TRUE;
                }

                if (HI_TRUE != bNeedPipe)
                {
                    continue;
                }

                ViPipe = pstViInfo->stPipeInfo.aPipe[i];

                s32Ret = HI_MPI_ISP_SetPubAttr(ViPipe, &stPubAttr);
                if (s32Ret != HI_SUCCESS)
                {
                    HIMPP_PRT("SetPubAttr failed with %#x!\n", s32Ret);
                    HIMPP_ISP_Stop(ViPipe);
                    return HI_FAILURE;
                }

            }
        }
    }

    while (1)
    {
        HI_MPI_ISP_QueryInnerStateInfo(ViPipe, &stInnerStateInfo);
        if ((HI_TRUE == stInnerStateInfo.bResSwitchFinish)
            || (HI_TRUE == stInnerStateInfo.bWDRSwitchFinish))
        {
            HIMPP_PRT("Switch finish!\n");
            break;
        }
        usleep(1000);
    }

    HIMPP_VI_StartPipe_Chn(pstViInfo);

    return HI_SUCCESS;
}


HI_S32  HIMPP_VI_SwitchMode_StopVI(HIMPP_VI_CONFIG_S *pstViConfigSrc)
{

    HI_S32              s32Ret = HI_SUCCESS;

    s32Ret = HIMPP_VI_DestroyVi(pstViConfigSrc);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_DestroyVi failed !\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_StopMIPI(pstViConfigSrc);

    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StopMIPI failed !\n");
        return HI_FAILURE;
    }
    // HIMPP_VI_StopIsp(pstViConfigSrc);

    return HI_SUCCESS;

}


HI_S32  HIMPP_VI_SwitchMode(HIMPP_VI_CONFIG_S *pstViConfigDes)
{

    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HIMPP_VI_StartMIPI(pstViConfigDes);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_StartMIPI failed!\n");
        return HI_FAILURE;
    }

    /*   create vi without enable chn and enable pipe. */
    s32Ret =  HIMPP_ModeSwitch_VI_CreateVi(pstViConfigDes);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_VI_CreateVi failed!\n");
        return HI_FAILURE;
    }

    s32Ret = HIMPP_VI_SwitchISPMode(pstViConfigDes);
    //HIMPP_VI_StartIsp(pstViConfigDes);
    if (s32Ret != HI_SUCCESS)
    {
        HIMPP_PRT("HIMPP_ModeSwitch_VI_CreateIsp!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


int HIMPP_VI_ExitMpp( int s32poolId)
{
    if (s32poolId < 0)
    {
        if (HI_MPI_SYS_Exit())
        {
            HIMPP_PRT("sys exit fail\n");
            return -1;
        }

        if (HI_MPI_VB_Exit())
        {
            HIMPP_PRT("vb exit fail\n");
            return -1;
        }

        return -1;
    }

    return 0;
}

/******************************************************************************
* funciton : Get enWDRMode by diffrent sensor
******************************************************************************/
HI_S32 HIMPP_VI_GetWDRModeBySensor(HIMPP_SNS_TYPE_E enMode, WDR_MODE_E *penWDRMode)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!penWDRMode)
    {
        return HI_FAILURE;
    }

    switch (enMode)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            *penWDRMode = WDR_MODE_2To1_LINE;
            break;

        default:
            *penWDRMode = WDR_MODE_2To1_LINE;
            break;
    }

    return s32Ret;
}

/******************************************************************************
* funciton : Get Pipe by diffrent sensor
******************************************************************************/
HI_S32 HIMPP_VI_GetPipeBySensor(HIMPP_SNS_TYPE_E enMode, HIMPP_PIPE_INFO_S *pstPipeInfo)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!pstPipeInfo)
    {
        return HI_FAILURE;
    }

    switch (enMode)
    {
        default:
            pstPipeInfo->enMastPipeMode = VI_ONLINE_VPSS_OFFLINE;
            pstPipeInfo->aPipe[0] = 0;
            pstPipeInfo->aPipe[1] = 1;
            break;
    }

    return s32Ret;
}

/******************************************************************************
* funciton : Get enSize by diffrent sensor
******************************************************************************/
HI_S32 HIMPP_VI_GetSizeBySensor(HIMPP_SNS_TYPE_E enMode, PIC_SIZE_E *penSize)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!penSize)
    {
        return HI_FAILURE;
    }

    switch (enMode)
    {
        case BT1120_2M_30FPS_8BIT:
        case BT656_2M_30FPS_8BIT:
        case BT601_2M_30FPS_8BIT:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
            *penSize = PIC_1080P;
            break;

        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
            *penSize = PIC_1080P;
            break;

        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
        case SMART_SC2235_DC_2M_30FPS_10BIT:
            *penSize = PIC_1080P;
            break;

        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
            *penSize = PIC_1080P;
            break;

		 // my edit:
        case OV5640_YUV_MIPI_2M_30FPS_8BIT:
            *penSize = PIC_1080P;
            break;

        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
            *penSize = PIC_2592x1944;
            break;

        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
            *penSize = PIC_2592x1520;
            break;

        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
            *penSize = PIC_2304x1296;
            break;
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
            *penSize = PIC_2688x1944;
            break;
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
            *penSize = PIC_2688x1536;
            break;

        default:
            *penSize = PIC_1080P;
            break;
    }

    return s32Ret;
}



/******************************************************************************
* funciton : Get enSize by diffrent sensor
******************************************************************************/
HI_S32 HIMPP_VI_GetFrameRateBySensor(HIMPP_SNS_TYPE_E enMode, HI_U32 *pu32FrameRate)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (!pu32FrameRate)
    {
        return HI_FAILURE;
    }

    switch (enMode)
    {
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT:
        case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
        case SONY_IMX335_MIPI_5M_30FPS_12BIT:
        case SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SONY_IMX335_MIPI_4M_30FPS_12BIT:
        case SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case SMART_SC4236_MIPI_3M_30FPS_10BIT:
        case SMART_SC2231_MIPI_2M_30FPS_10BIT:
        case SOI_JXF37_MIPI_2M_30FPS_10BIT:
        case SMART_SC3235_MIPI_3M_30FPS_10BIT:
        case SMART_SC2235_DC_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR:
        case OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case BT1120_2M_30FPS_8BIT:
        case BT656_2M_30FPS_8BIT:
        case BT601_2M_30FPS_8BIT:
            *pu32FrameRate = 30;
            break;
        case SMART_SC4236_MIPI_3M_20FPS_10BIT:
            *pu32FrameRate = 20;
            break;
        default:
            *pu32FrameRate = 30;
            break;

    }

    return s32Ret;
}

HI_VOID HIMPP_VI_GetSensorInfo(HIMPP_VI_CONFIG_S *pstViConfig)
{
    HI_S32 i;

    for (i = 0; i < VI_MAX_DEV_NUM; i++)
    {
        pstViConfig->astViInfo[i].stSnsInfo.s32SnsId = i;
        pstViConfig->astViInfo[i].stSnsInfo.s32BusId = i;
        pstViConfig->astViInfo[i].stSnsInfo.MipiDev  = i;
        hi_memset(&pstViConfig->astViInfo[i].stSnapInfo, sizeof(HIMPP_SNAP_INFO_S), 0, sizeof(HIMPP_SNAP_INFO_S));
        pstViConfig->astViInfo[i].stPipeInfo.bMultiPipe = HI_FALSE;
        pstViConfig->astViInfo[i].stPipeInfo.bVcNumCfged = HI_FALSE;
    }

    pstViConfig->astViInfo[0].stSnsInfo.enSnsType = SENSOR0_TYPE;
}

combo_dev_t HIMPP_VI_GetComboDevBySensor(HIMPP_SNS_TYPE_E enMode, HI_S32 s32SnsIdx)
{
    combo_dev_t dev = 0;
    return dev;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
