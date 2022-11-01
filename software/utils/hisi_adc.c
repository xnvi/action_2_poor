#include "hisi_adc.h"
#include "himm.h"

uint32_t hiadc_read_reg(uint32_t off)
{
    return himd(LSADC_REG_BASE + off);
}

void hiadc_write_reg(uint32_t off, uint32_t value)
{
    himm(LSADC_REG_BASE + off, value);
}

void hiadc_setbits(uint32_t off, uint32_t mask, uint32_t value)
{
    uint32_t read = 0;

    read = hiadc_read_reg(off);

    read &= ~mask;
    read |= value & mask;
    hiadc_write_reg(off, read);
}

void hiadc_cfg_ch0_io(void)
{
    // 引脚复用寄存器，对应PIN88
    himm(0x120C0000, 0x00000001);
}

void hiadc_cfg_ch1_io(void)
{
    // 引脚复用寄存器，对应PIN87
    himm(0x120C0004, 0x00000001);
}

// 清中断，bit0对应通道0，bit1对应通道1
void hiadc_clean_int(uint32_t n)
{
    hiadc_write_reg(LSADC_REG_CTRL6, n & 0x3);
}

// 查询并清除中断，bit0对应通道0，bit1对应通道1
uint32_t hiadc_get_int(void)
{
    uint32_t intr;

    intr = hiadc_read_reg(LSADC_REG_CTRL5);
    hiadc_clean_int(intr);
    return intr;
}

static void hiadc_start(void)
{
    hiadc_write_reg(LSADC_REG_CTRL7, 1);
}

static void hiadc_stop(void)
{
    hiadc_write_reg(LSADC_REG_CTRL8, 1);
}

uint32_t hiadc_get_ch0(void)
{
    return hiadc_read_reg(LSADC_REG_CTRL11) & LSADC_DATA_IN0_MASK;
}

uint32_t hiadc_get_ch1(void)
{
    return hiadc_read_reg(LSADC_REG_CTRL12) & LSADC_DATA_IN1_MASK;
}

void hiadc_enable_ch0(int32_t enable)
{
    if (enable)
    {
        hiadc_cfg_ch0_io();
        hiadc_setbits(LSADC_REG_CTRL0, CH_0_VLD, CH_0_VLD);
        hiadc_start();
    }
    else
    {
        hiadc_setbits(LSADC_REG_CTRL0, CH_0_VLD, 0);
        // 两个通道都关闭后停止adc
        if (0 == (hiadc_read_reg(LSADC_REG_CTRL0) & (CH_0_VLD | CH_1_VLD)))
            hiadc_stop();
    }
}

void hiadc_enable_ch1(int32_t enable)
{
    if (enable)
    {
        hiadc_cfg_ch1_io();
        hiadc_setbits(LSADC_REG_CTRL0, CH_1_VLD, CH_1_VLD);
        hiadc_start();
    }
    else
    {
        hiadc_setbits(LSADC_REG_CTRL0, CH_1_VLD, 0);
        // 两个通道都关闭后停止adc
        if (0 == (hiadc_read_reg(LSADC_REG_CTRL0) & (CH_0_VLD | CH_1_VLD)))
            hiadc_stop();
    }
}

// 设置连续模式采样周期，单位ms
int32_t hiadc_set_period(uint32_t t)
{
    if ((((uint32_t)(-1)) / 3000) <= t)
        return -1;

    hiadc_write_reg(LSADC_REG_CTRL2, t * 3000);
    return 0;
}

int32_t hiadc_init(void)
{
    uint32_t reg = 0;

    // 开启 ADC 时钟
    reg = himd(0x12010000 + 0x01BC);
    reg |= (1 << 23);
    himm(0x12010000 + 0x01BC, reg);

    hiadc_stop();

    // 默认配置
    hiadc_write_reg(LSADC_REG_CTRL0, DATA_DELTA(30) | DEGLITCH_BYPASS | MODEL_SEL); // 误差范围，不使能虑毛刺功能，连续扫描
    hiadc_write_reg(LSADC_REG_CTRL1, 3000);
    hiadc_set_period(100); // 采样周期1s
    hiadc_write_reg(LSADC_REG_CTRL9, ACTIVE_BIT(10)); // 10位精度
    // hiadc_clean_int(CLR_INT_FLAG_IN0 | CLR_INT_FLAG_IN1); // 关中断
    // hiadc_write_reg(LSADC_REG_CTRL4, INT_ENABLE); // 开中断

    return 0;
}

void hiadc_deinit(void)
{
    return;
}
