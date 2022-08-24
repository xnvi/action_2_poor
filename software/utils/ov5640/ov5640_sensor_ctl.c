#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctype.h>
#include <sys/mman.h>
#include <memory.h>

#include "hi_comm_video.h"
#include "hi_sns_ctrl.h"
#include "hi_i2c.h"

// const unsigned char ov5640_i2c_addr  = 0x3c;
const unsigned char ov5640_i2c_addr  = 0x78;
const unsigned int  ov5640_addr_byte = 2;
const unsigned int  ov5640_data_byte = 1;
static int g_fd[ISP_MAX_PIPE_NUM] = {[0 ...(ISP_MAX_PIPE_NUM - 1)] = -1};


typedef struct {
    uint16_t addr;
    uint8_t  value;
} SCCB_REG_CFG;

SCCB_REG_CFG ov5640_init_settings[] = {
    // {0x3103, 0x11}, // SCCB system control
    // {0x3008, 0x82}, // software reset

    // delay 5ms
    {0x3008, 0x42}, // software power down
    {0x3103, 0x03}, // SCCB system control
    {0x3017, 0x00}, // set Frex, Vsync, Href, PCLK, D[9:6] input
    {0x3018, 0x00}, // set d[5:0], GPIO[1:0] input
    {0x3034, 0x18}, // MIPI 8-bit mode
    {0x3037, 0x13}, // PLL
    {0x3108, 0x01}, // system divider
    {0x3630, 0x36},
    {0x3631, 0x0e},
    {0x3632, 0xe2},
    {0x3633, 0x12},
    {0x3621, 0xe0},
    {0x3704, 0xa0},
    {0x3703, 0x5a},
    {0x3715, 0x78},
    {0x3717, 0x01},
    {0x370b, 0x60},
    {0x3705, 0x1a},
    {0x3905, 0x02},
    {0x3906, 0x10},
    {0x3901, 0x0a},
    {0x3731, 0x12},
    {0x3600, 0x08}, // VCM debug mode
    {0x3601, 0x33}, // VCM debug mode
    {0x302d, 0x60}, // system control
    {0x3620, 0x52},
    {0x371b, 0x20},
    {0x471c, 0x50},
    {0x3a13, 0x43}, // AGC pre-gain, 0x40 = 1x
    {0x3a18, 0x00}, // gain ceiling
    {0x3a19, 0xf8}, // gain ceiling
    {0x3635, 0x13},
    {0x3636, 0x03},
    {0x3634, 0x40},
    {0x3622, 0x01},
    // 50Hz/60Hz
    {0x3c01, 0x34}, // 50/60Hz
    {0x3c04, 0x28}, // threshold for low sum
    {0x3c05, 0x98}, // threshold for high sum
    {0x3c06, 0x00}, // light meter 1 threshold high
    {0x3c08, 0x00}, // light meter 2 threshold high
    {0x3c09, 0x1c}, // light meter 2 threshold low
    {0x3c0a, 0x9c}, // sample number high
    {0x3c0b, 0x40}, // sample number low
    // timing
    {0x3800, 0x00}, // HS
    {0x3801, 0x00}, // HS
    {0x3802, 0x00}, // VS
    {0x3804, 0x0a}, // HW
    {0x3805, 0x3f}, // HW
    {0x3810, 0x00}, // H offset high
    {0x3811, 0x10}, // H offset low
    {0x3812, 0x00}, // V offset high
    {0x3708, 0x64},
    {0x3a08, 0x01}, // B50
    {0x4001, 0x02}, // BLC start line
    {0x4005, 0x1a}, // BLC always update
    {0x3000, 0x00}, // system reset 0
    {0x3002, 0x1c}, // system reset 2
    {0x3004, 0xff}, // clock enable 00
    {0x3006, 0xc3}, // clock enable 2
    {0x300e, 0x45}, // MIPI control, 2 lane, MIPI enable
    {0x302e, 0x08},
    {0x4300, 0x30}, // YUV 422, YUYV
    // {0x4300, 0x31}, // YUV 422, YVYU
    // {0x4300, 0x32}, // YUV 422, UYVY
    // {0x4300, 0x33}, // YUV 422, VYUY
    {0x501f, 0x00}, // ISP YUV 422
    {0x4407, 0x04}, // JPEG QS
    {0x440e, 0x00},
    {0x5000, 0xa7}, // ISP control, Lenc on, gamma on, BPC on, WPC on, CIP on
    // AWB
    {0x5180, 0xff},
    {0x5181, 0xf2},
    {0x5182, 0x00},
    {0x5183, 0x14},
    {0x5184, 0x25},
    {0x5185, 0x24},
    {0x5186, 0x09},
    {0x5187, 0x09},
    {0x5188, 0x09},
    {0x5189, 0x75},
    {0x518a, 0x54},
    {0x518b, 0xe0},
    {0x518c, 0xb2},
    {0x518d, 0x42},
    {0x518e, 0x3d},
    {0x518f, 0x56},
    {0x5190, 0x46},
    {0x5191, 0xf8},
    {0x5192, 0x04},
    {0x5193, 0x70},
    {0x5194, 0xf0},
    {0x5195, 0xf0},
    {0x5196, 0x03},
    {0x5197, 0x01},
    {0x5198, 0x04},
    {0x5199, 0x12},
    {0x519a, 0x04},
    {0x519b, 0x00},
    {0x519c, 0x06},
    {0x519d, 0x82},
    {0x519e, 0x38},
    // color matrix
    {0x5381, 0x1e},
    {0x5382, 0x5b},
    {0x5383, 0x08},
    {0x5384, 0x0a},
    {0x5385, 0x7e},
    {0x5386, 0x88},
    {0x5387, 0x7c},
    {0x5388, 0x6c},
    {0x5389, 0x10},
    {0x538a, 0x01},
    {0x538b, 0x98},
    // CIP
    {0x5300, 0x08}, // sharpen MT th1
    {0x5301, 0x30}, // sharpen MT th2
    {0x5302, 0x10}, // sharpen MT offset 1
    {0x5303, 0x00}, // sharpen MT offset 2
    {0x5304, 0x08}, // DNS threshold 1
    {0x5305, 0x30}, // DNS threshold 2
    {0x5306, 0x08}, // DNS offset 1
    {0x5307, 0x16}, // DNS offset 2
    {0x5309, 0x08}, // sharpen TH th1
    {0x530a, 0x30}, // sharpen TH th2
    {0x530b, 0x04}, // sharpen TH offset 1
    {0x530c, 0x06}, // sharpen Th offset 2
    // gamma
    {0x5480, 0x01},
    {0x5481, 0x08},
    {0x5482, 0x14},
    {0x5483, 0x28},
    {0x5484, 0x51},
    {0x5485, 0x65},
    {0x5486, 0x71},
    {0x5487, 0x7d},
    {0x5488, 0x87},
    {0x5489, 0x91},
    {0x548a, 0x9a},
    {0x548b, 0xaa},
    {0x548c, 0xb8},
    {0x548d, 0xcd},
    {0x548e, 0xdd},
    {0x548f, 0xea},
    {0x5490, 0x1d},
    // UV adjust
    {0x5580, 0x06}, // sat on, contrast on
    {0x5583, 0x40}, // sat U
    {0x5584, 0x10}, // sat V
    {0x5589, 0x10}, // UV adjust th1
    {0x558a, 0x00}, // UV adjust th2[8]
    {0x558b, 0xf8}, // UV adjust th2[7:0]
    {0x501d, 0x04}, // enable manual offset of contrast
    // lens correction
    {0x5800, 0x23},
    {0x5801, 0x14},
    {0x5802, 0x0f},
    {0x5803, 0x0f},
    {0x5804, 0x12},
    {0x5805, 0x26},
    {0x5806, 0x0c},
    {0x5807, 0x08},
    {0x5808, 0x05},
    {0x5809, 0x05},
    {0x580a, 0x08},
    {0x580b, 0x0d},
    {0x580c, 0x08},
    {0x580d, 0x03},
    {0x580e, 0x00},
    {0x580f, 0x00},
    {0x5810, 0x03},
    {0x5811, 0x09},
    {0x5812, 0x07},
    {0x5813, 0x03},
    {0x5814, 0x00},
    {0x5815, 0x01},
    {0x5816, 0x03},
    {0x5817, 0x08},
    {0x5818, 0x0d},
    {0x5819, 0x08},
    {0x581a, 0x05},
    {0x581b, 0x06},
    {0x581c, 0x08},
    {0x581d, 0x0e},
    {0x581e, 0x29},
    {0x581f, 0x17},
    {0x5820, 0x11},
    {0x5821, 0x11},
    {0x5822, 0x15},
    {0x5823, 0x28},
    {0x5824, 0x46},
    {0x5825, 0x26},
    {0x5826, 0x08},
    {0x5827, 0x26},
    {0x5828, 0x64},
    {0x5829, 0x26},
    {0x582a, 0x24},
    {0x582b, 0x22},
    {0x582c, 0x24},
    {0x582d, 0x24},
    {0x582e, 0x06},
    {0x582f, 0x22},
    {0x5830, 0x40},
    {0x5831, 0x42},
    {0x5832, 0x24},
    {0x5833, 0x26},
    {0x5834, 0x24},
    {0x5835, 0x22},
    {0x5836, 0x22},
    {0x5837, 0x26},
    {0x5838, 0x44},
    {0x5839, 0x24},
    {0x583a, 0x26},
    {0x583b, 0x28},
    {0x583c, 0x42},
    {0x583d, 0xce},
    {0x5025, 0x00},
    {0x3a0f, 0x30}, // stable in high
    {0x3a10, 0x28}, // stable in low
    {0x3a1b, 0x30}, // stable out high
    {0x3a1e, 0x26}, // stable out low
    {0x3a11, 0x60}, // fast zone high
    {0x3a1f, 0x14}, // fast zone low
    {0x3008, 0x02}, // wake up
};

SCCB_REG_CFG ov5640_vga_30fps_settings[] = {
    //input 24M
    //output vga 30fps bit rate 224M bps
    {0x3035, 0x14}, // pll
    {0x3036, 0x38}, // pll
    {0x3c07, 0x08}, // light meter 1 threshold
    {0x3820, 0x41}, // ISP flip off, sensor flip off
    {0x3821, 0x07}, // ISP mirror on, sensor mirror on
    // timing
    {0x3814, 0x31}, // X inc
    {0x3815, 0x31}, // Y inc
    {0x3803, 0x04}, // VS
    {0x3806, 0x07}, // VH
    {0x3807, 0x9b}, // VH
    {0x3808, 0x02}, // DVPHO
    {0x3809, 0x80}, // DVPHO
    {0x380a, 0x01}, // DVPVO
    {0x380b, 0xe0}, // DVPVO
    {0x380c, 0x07}, // HTS
    {0x380d, 0x68}, // HTS
    {0x380e, 0x03}, // VTS
    {0x380f, 0xd8}, // VTS
    {0x3813, 0x06}, // V offset
    {0x3618, 0x00},
    {0x3612, 0x29},
    {0x3709, 0x52},
    {0x370c, 0x03},
    {0x3a02, 0x03}, // 60Hz max exposure
    {0x3a03, 0xd8}, // 60Hz max exposure
    {0x3a09, 0x27}, // B50 low
    {0x3a0a, 0x00}, // B60 high
    {0x3a0b, 0xf6}, // B60 low
    {0x3a0e, 0x03}, // B50 max
    {0x3a0d, 0x04}, // B60 max
    {0x3a14, 0x03}, // 50Hz max exposure
    {0x3a15, 0xd8}, // 50Hz max exposure
    {0x4004, 0x02}, // BLC line number
    {0x4713, 0x03}, // JPEG mode 3
    {0x460b, 0x35}, // debug
    {0x460c, 0x22}, // VFIFO, PCLK manual
    {0x4837, 0x44}, // MIPI global timing
    {0x3824, 0x02}, // PCLK divider
    {0x5001, 0xa3}, // SDE on, scale on, UV average off, CMX on, AWB on
};

SCCB_REG_CFG ov5640_1080p_30fps_settings[] = {
    //input 24M
    {0x3035, 0x11}, // pll
    {0x3036, 0x54}, // pll
    {0x3c07, 0x07}, // light meter 1 threshold
    {0x3820, 0x42}, // ISP flip off, sensor flip on
    {0x3821, 0x00}, // ISP mirror off, sensor mirror off
    // timing
    {0x3814 ,0x11},
    {0x3815 ,0x11},
    {0x3800 ,0x01},
    {0x3801 ,0x50},
    {0x3802 ,0x01},
    {0x3803 ,0xb2},
    {0x3804 ,0x08},
    {0x3805 ,0xef},
    {0x3806 ,0x05},
    {0x3807 ,0xf2},
    {0x3808 ,0x07},
    {0x3809 ,0x80},
    {0x380a ,0x04},
    {0x380b ,0x38},
    {0x380c ,0x09},
    {0x380d ,0xc4},
    {0x380e ,0x04},
    {0x380f ,0x60},
    {0x3810 ,0x00},
    {0x3811 ,0x10},
    {0x3812 ,0x00},
    {0x3813 ,0x04},

    {0x3618, 0x04},
    {0x3612, 0x2b},
    {0x3709, 0x12},
    {0x370c, 0x00},
    // banding filter
    {0x3a02, 0x07}, // 60Hz max exposure
    {0x3a03, 0xb0}, // 60Hz max exposure
    {0x3a09, 0x27}, // B50 low
    {0x3a0a, 0x00}, // B60 high
    {0x3a0b, 0xf6}, // B60 low
    {0x3a0e, 0x06}, // B50 max
    {0x3a0d, 0x08}, // B60 max
    {0x3a14, 0x07}, // 50Hz max exposure
    {0x3a15, 0xb0}, // 50Hz max exposure
    {0x4004, 0x06}, // BLC line number
    {0x4713, 0x00}, // JPEG mode
    {0x460b, 0x35},
    {0x460c, 0x22}, // VFIFO, PCLK manual
    {0x4837, 0x0a}, // MIPI global timing
    {0x3824, 0x01}, // PCLK divider
    {0x5001, 0x83}, // SDE on, scale off, UV average off, CMX on, AWB on
};

int OV5640_i2c_init(VI_PIPE ViPipe)
{
    char acDevFile[16] = {0};
    HI_U8 u8DevNum = 0; // 目前硬件设计OV5640使用海思的 I2C-0 通道
    int ret;

    if (g_fd[ViPipe] >= 0) {
        return HI_SUCCESS;
    }

    snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", u8DevNum);

    g_fd[ViPipe] = open(acDevFile, O_RDWR, S_IRUSR | S_IWUSR);

    if (g_fd[ViPipe] < 0) {
        ISP_TRACE(HI_DBG_ERR, "Open /dev/hi_i2c_drv-%u error!\n", u8DevNum);
        return HI_FAILURE;
    }

    ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, (ov5640_i2c_addr >> 1));
    if (ret < 0) {
        ISP_TRACE(HI_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
        close(g_fd[ViPipe]);
        g_fd[ViPipe] = -1;
        return ret;
    }

    return HI_SUCCESS;
}

int OV5640_i2c_exit(VI_PIPE ViPipe)
{
    if (g_fd[ViPipe] >= 0) {
        close(g_fd[ViPipe]);
        g_fd[ViPipe] = -1;
        return HI_SUCCESS;
    }
    return HI_FAILURE;
}

int OV5640_read_register(VI_PIPE ViPipe, int addr)
{
    // 目前isp模块没必要读寄存器，此函数略
    return HI_SUCCESS;
}

int OV5640_write_register(VI_PIPE ViPipe, HI_U32 addr, HI_U32 data)
{
    if (g_fd[ViPipe] < 0) {
        return HI_SUCCESS;
    }

    int idx = 0;
    int ret;
    char buf[8];

    if (ov5640_addr_byte == 2) {
        buf[idx] = (addr >> 8) & 0xff;
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    } else {
    }

    if (ov5640_data_byte == 2) {
    } else {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_fd[ViPipe], buf, ov5640_addr_byte + ov5640_data_byte);
    if (ret < 0) {
        ISP_TRACE(HI_DBG_ERR, "I2C_WRITE reg(%#x) val(#%#x) error!\n", addr, data);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

void OV5640_standby(VI_PIPE ViPipe)
{
    return;
}

void OV5640_restart(VI_PIPE ViPipe)
{
    return;
}

static void delay_ms(int ms)
{
    usleep(ms * 1000);
}

// 640*480 分辨率仅测试使用
void OV5640_linear_480P_YUV_8bit_init(VI_PIPE ViPipe)
{
    int i;
    delay_ms(5);
    for (i = 0; i < sizeof(ov5640_vga_30fps_settings) / sizeof(ov5640_vga_30fps_settings[0]); i++) {
        OV5640_write_register(ViPipe, ov5640_vga_30fps_settings[i].addr, ov5640_vga_30fps_settings[i].value);
    }
    return;
}

void OV5640_linear_1080P_YUV_8bit_init(VI_PIPE ViPipe)
{
    int i;
    delay_ms(5);
    for (i = 0; i < sizeof(ov5640_1080p_30fps_settings) / sizeof(ov5640_1080p_30fps_settings[0]); i++) {
        OV5640_write_register(ViPipe, ov5640_1080p_30fps_settings[i].addr, ov5640_1080p_30fps_settings[i].value);
    }
    return;
}

void OV5640_base_init(VI_PIPE ViPipe)
{
    int i;
    OV5640_write_register(ViPipe, 0x3103, 0x11); // SCCB system control
    OV5640_write_register(ViPipe, 0x3008, 0x82); // software reset
    delay_ms(5);

    for (i = 0; i < sizeof(ov5640_init_settings) / sizeof(ov5640_init_settings[0]); i++) {
        OV5640_write_register(ViPipe, ov5640_init_settings[i].addr, ov5640_init_settings[i].value);
    }
    return;
}

void OV5640_init(VI_PIPE ViPipe)
{
    OV5640_i2c_init(ViPipe);
    OV5640_base_init(ViPipe);
    // OV5640_linear_480P_YUV_8bit_init(ViPipe);
    OV5640_linear_1080P_YUV_8bit_init(ViPipe);

    // 输出测试图像
    // OV5640_write_register(ViPipe, 0x503d, 0x80);
    // OV5640_write_register(ViPipe, 0x4741, 0x00);

    printf("------- OV5640 Initial OK -------\n");
    return;
}

void OV5640_exit(VI_PIPE ViPipe)
{
    OV5640_i2c_exit(ViPipe);
    return;
}
