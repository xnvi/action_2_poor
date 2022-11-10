/*
 * The Flash Memory Controller v100 Device Driver for hisilicon
 *
 * Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*****************************************************************************/
#include <common.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <hifmc_common.h>
#include <spi_flash.h>
#include "../../hifmc_spi_ids.h"
#include "hifmc100.h"

/*****************************************************************************/
int start_up_addr_mode;
struct spi_nor_info *spiinfo;

/*****************************************************************************/
SET_READ_STD(0, INFINITE, 33);
SET_READ_STD(0, INFINITE, 40);
SET_READ_STD(0, INFINITE, 50);
SET_READ_STD4B(0, INFINITE, 50);
SET_READ_STD(0, INFINITE, 54);
SET_READ_STD4B(0, INFINITE, 54);
SET_READ_STD(0, INFINITE, 55);
SET_READ_STD4B(0, INFINITE, 55);
SET_READ_STD(0, INFINITE, 66);
SET_READ_STD4B(0, INFINITE, 66);
SET_READ_STD(0, INFINITE, 80);

SET_READ_FAST(1, INFINITE, 80);
SET_READ_FAST4B(1, INFINITE, 80);
SET_READ_FAST(1, INFINITE, 86);
SET_READ_FAST(1, INFINITE, 100);
SET_READ_FAST(1, INFINITE, 104);
SET_READ_FAST4B(1, INFINITE, 104);
SET_READ_FAST(1, INFINITE, 108);
SET_READ_FAST4B(1, INFINITE, 108);
SET_READ_FAST(1, INFINITE, 133);
SET_READ_FAST4B(1, INFINITE, 133);

SET_READ_DUAL(1, INFINITE, 64);
SET_READ_DUAL(1, INFINITE, 80);
SET_READ_DUAL4B(1, INFINITE, 80);
SET_READ_DUAL(1, INFINITE, 84);
SET_READ_DUAL(1, INFINITE, 104);
SET_READ_DUAL4B(1, INFINITE, 104);
SET_READ_DUAL(1, INFINITE, 108);
SET_READ_DUAL4B(1, INFINITE, 108);
SET_READ_DUAL(1, INFINITE, 133);
SET_READ_DUAL4B(1, INFINITE, 133);

SET_READ_DUAL_ADDR(1, INFINITE, 80);
SET_READ_DUAL_ADDR4B(1, INFINITE, 80);
SET_READ_DUAL_ADDR4B(1, INFINITE, 108);
SET_READ_DUAL_ADDR(2, INFINITE, 84);
SET_READ_DUAL_ADDR4B(2, INFINITE, 84);
SET_READ_DUAL_ADDR(2, INFINITE, 108);
SET_READ_DUAL_ADDR(1, INFINITE, 104);
SET_READ_DUAL_ADDR4B(1, INFINITE, 104);
SET_READ_DUAL_ADDR(1, INFINITE, 108);
SET_READ_DUAL_ADDR(1, INFINITE, 133);
SET_READ_DUAL_ADDR4B(1, INFINITE, 133);
SET_READ_DUAL_ADDR4B(2, INFINITE, 133);

SET_READ_QUAD(1, INFINITE, 80);
SET_READ_QUAD4B(1, INFINITE, 80);
SET_READ_QUAD(1, INFINITE, 84);
SET_READ_QUAD(1, INFINITE, 104);
SET_READ_QUAD4B(1, INFINITE, 104);
SET_READ_QUAD(1, INFINITE, 108);
SET_READ_QUAD4B(1, INFINITE, 108);
SET_READ_QUAD(1, INFINITE, 133);
SET_READ_QUAD4B(1, INFINITE, 133);

SET_READ_QUAD_ADDR(3, INFINITE, 80);
SET_READ_QUAD_ADDR4B(3, INFINITE, 80);
SET_READ_QUAD_ADDR(5, INFINITE, 84);
SET_READ_QUAD_ADDR4B(5, INFINITE, 84);
SET_READ_QUAD_ADDR(5, INFINITE, 108);
SET_READ_QUAD_ADDR(3, INFINITE, 104);
SET_READ_QUAD_ADDR4B(3, INFINITE, 104);
SET_READ_QUAD_ADDR(3, INFINITE, 108);
SET_READ_QUAD_ADDR4B(3, INFINITE, 108);
SET_READ_QUAD_ADDR4B(5, INFINITE, 125);
SET_READ_QUAD_ADDR(3, INFINITE, 133);
SET_READ_QUAD_ADDR(3, INFINITE, 72);
SET_READ_QUAD_ADDR4B(3, INFINITE, 133);

#ifdef CONFIG_DTR_MODE_SUPPORT
/* SET_READ_QUAD_DTR4B_WINBOND(8, INFINITE, 80); */
SET_READ_QUAD_DTR(8, INFINITE, 80);
SET_READ_QUAD_DTR4B(10, INFINITE, 80);
SET_READ_QUAD_DTR4B(10, INFINITE, 100);
SET_READ_QUAD_DTR(10, INFINITE, 100);
#endif
/*****************************************************************************/
SET_WRITE_STD(0, 256, 33);
SET_WRITE_STD(0, 256, 80);
SET_WRITE_STD4B(0, 256, 80);
SET_WRITE_STD(0, 256, 86);
SET_WRITE_STD(0, 256, 100);
SET_WRITE_STD(0, 256, 104);
SET_WRITE_STD4B(0, 256, 104);
SET_WRITE_STD(0, 256, 108);
SET_WRITE_STD(0, 256, 133);
SET_WRITE_STD4B(0, 256, 120);
SET_WRITE_STD4B(0, 256, 133);

SET_WRITE_DUAL(0, 256, 75);
SET_WRITE_DUAL4B(0, 256, 75);
SET_WRITE_DUAL(0, 256, 108);
SET_WRITE_DUAL4B(0, 256, 133);

SET_WRITE_DUAL_ADDR(0, 256, 75);
SET_WRITE_DUAL_ADDR4B(0, 256, 75);
SET_WRITE_DUAL_ADDR(0, 256, 108);
SET_WRITE_DUAL_ADDR4B(0, 256, 133);

SET_WRITE_QUAD(0, 256, 80);
SET_WRITE_QUAD4B(0, 256, 80);
SET_WRITE_QUAD(0, 256, 104);
SET_WRITE_QUAD(0, 256, 108);
SET_WRITE_QUAD(0, 256, 133);
SET_WRITE_QUAD4B(0, 256, 133);

/* FIXME: As Micron MT25Q(and MIXC) and N25Q have different QUAD I/O write code,
 * but they have the same ID, so we cannot compatiable it. User can open
 * by theirselves. */
SET_WRITE_QUAD_ADDR(0, 256, 33);
SET_WRITE_QUAD_ADDR(0, 256, 80);
/* SET_WRITE_QUAD_ADDR(0, 256, 64); */
SET_WRITE_QUAD_ADDR(0, 256, 104);
SET_WRITE_QUAD_ADDR4B(0, 256, 104);
SET_WRITE_QUAD_ADDR(0, 256, 108);
SET_WRITE_QUAD_ADDR4B(0, 256, 108);
SET_WRITE_QUAD_ADDR4B(0, 256, 120);
SET_WRITE_QUAD_ADDR(0, 256, 133);
/*****************************************************************************/
SET_ERASE_SECTOR_64K(0, _64K, 33);
SET_ERASE_SECTOR_64K(0, _64K, 50);
SET_ERASE_SECTOR_64K(0, _64K, 80);
SET_ERASE_SECTOR_64K4B(0, _64K, 80);
SET_ERASE_SECTOR_64K(0, _64K, 86);
SET_ERASE_SECTOR_64K(0, _64K, 100);
SET_ERASE_SECTOR_64K(0, _64K, 104);
SET_ERASE_SECTOR_64K4B(0, _64K, 104);
SET_ERASE_SECTOR_64K(0, _64K, 108);
SET_ERASE_SECTOR_64K4B(0, _64K, 108);
SET_ERASE_SECTOR_64K4B(0, _64K, 120);
SET_ERASE_SECTOR_64K(0, _64K, 133);
SET_ERASE_SECTOR_64K4B(0, _64K, 133);

/*****************************************************************************/
#include "hifmc100_spi_general.c"
static struct spi_drv spi_driver_general = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_general_qe_enable,
};

static struct spi_drv spi_driver_no_qe = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_do_not_qe_enable,
};

#include "hifmc100_spi_s25fl256s.c"
static struct spi_drv spi_driver_s25fl256s = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_s25fl256s_entry_4addr,
    .qe_enable = spi_general_qe_enable,
};

#include "hifmc100_spi_w25q256fv.c"
static struct spi_drv spi_driver_w25q256fv = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_w25q256fv_entry_4addr,
    .qe_enable = spi_w25q256fv_qe_enable,
};

#include "hifmc100_spi_mx25l25635e.c"
static struct spi_drv spi_driver_mx25l25635e = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_mx25l25635e_qe_enable,
#ifdef CONFIG_DTR_MODE_SUPPORT
    .dtr_set_device =
    spi_mxic_output_driver_strength_set,
#endif
};

static struct spi_drv spi_driver_f25l64q = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_mx25l25635e_qe_enable,
};

#include "hifmc100_spi_gd25qxxx.c"
static struct spi_drv spi_driver_gd25qxxx = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_gd25qxxx_qe_enable,
};

#include "hifmc100_spi_micron.c"
static struct spi_drv spi_driver_micron = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_micron_entry_4addr,
    .qe_enable = spi_do_not_qe_enable,
};

#include "hifmc100_spi_xtx.c"
static struct spi_drv spi_driver_xtx = {
    .wait_ready = spi_general_wait_ready,
    .write_enable = spi_general_write_enable,
    .entry_4addr = spi_general_entry_4addr,
    .qe_enable = spi_xtx_qe_enable,
};

/*****************************************************************************/
#define SPI_NOR_ID_TBL_VER     "1.0"

/* ****** SPI Nor ID Table *************************************************************************
 * 		Manufacturer       Chip Name           Chipsize    Block   Vol  Operation
 * 1.0  Macronix/MXIC   MX25L1606E      		2M      	64K 	3V3
 *      Macronix/MXIC   MX25V1635F      		2M      	64K 	3V3
 *      Macronix/MXIC   MX25L6436F      		8M      	64K 	3V3
 *      Macronix/MXIC   MX25R6435F      		8M      	64K 	1V8/3V3 Add 14chips
 *      Macronix/MXIC   MX25U6435F      		8M     	 	64K 	1V8
 *      Macronix/MXIC   MX25U12835F     		16M     	64K     1V8
 *      Macronix/MXIC   MX25F128XXX     		16M     	64K     3V3
 *      Macronix/MXIC   MX25U25635F/45G     	32M     	64K 	1V8     25645G-DTR
 *      Macronix/MXIC   MX25L(256/257)      	32M         64K     3V3 25645G-DTR
 *      Macronix/MXIC   MX25U51245G     		64M     	64K 	1V8     51245G-DTR
 *      Spansion    	S25FL129P1      		16M         64K 	3V3
 *      Spansion    	S25FL256S       		32M         64K 	3V3
 *      Micron      	N25Q064A        		8M          64K     3V3
 *      Micron      	N25QL064A       		8M          64K     3V3
 *      Micron      	N25Q128A11/MT25QU128AB  16M         64K     1V8
 *      Micron      	N25QL128A       		16M         64K     3V3
 *      Micron      	MT25QU256A      		32M         64K     1V8
 *      Micron      	MT25QL256A      		32M         64K     3V3
 *      Winbond     	W25Q16(B/C)V/S25FL016K  2M      	64K     3V3
 *      Winbond     	W25Q32(B/F)V        	4M      	64K     3V3
 *      Winbond     	W25Q32FW        		4M      	64K     1V8
 *      Winbond     	W25Q64FW        		8M      	64K     1V8
 *      Winbond     	W25Q64FV(SPI)/W25Q64JV_IQ   8M      64K     3V3
 *      Winbond     	W25Q128FW       		16M         64K     1V8
 *      Winbond     	W25Q128(B/F)V       	16M         64K     3V3
 *      Winbond     	W25Q128JV_IM        	16M         64K     3V3 DTR
 *      Winbond         W25Q256JWFIM        	32M         64K     1V8
 *      Winbond         W25Q256FV       		32M         64K     3V3
 *      ESMT/CFEON  	EN25Q32B        		4M          64K     3V3
 *      ESMT/CFEON  	EN25Q64         		8M          64K     3V3
 *      ESMT/CFEON  	EN25Q128        		16M         64K     3V3
 *      ESMT/CFEON  	F25L64QA        		8M          64K     3V3
 *      GD      		GD25Q64         		8M          64K     3V3
 *      GD      		GD25LQ128       		16M         64K     1V8
 *      GD      		GD25Q128        		16M         64K     3V3
 *      GD      		GD25LQ64C       		8M          64K     1V8
 *      GD      		GD25Q32         		4M          64K     3V3
 *      GD      		GD25Q16C        		2M      	64K     3V3
 *      Paragon     	PN25F16S        		2M      	64K     3V3
 *      Paragon     	PN25F32S        		4M          64K     3V3
 *      XMC     		XM25QH64AHIG        	8M      	64K 	3V3
 *      XMC     		XM25QH128A      		16M     	64K 	3V3
 *      XMC     		XM25QH128B      		16M     	64K 	3V3
 *		XTX				XT25F128BSSI/HGU		16M			64K		3V3
 *		XTX				XM25QH64				8M			64K		3V3
 ************************************************************************************************* */
static struct spi_nor_info hifmc_spi_nor_info_table[] = {
    /* name     id  id_len  chipsize(Bytes) erasesize  */
    {
        "MX25L1606E",  {0xc2, 0x20, 0x15}, 3, _2M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 86),
            &READ_DUAL(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 86),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 86),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "MX25L6436F",  {0xc2, 0x20, 0x17}, 3, _8M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 133),
            &READ_DUAL(1, INFINITE, 133),
            &READ_DUAL_ADDR(1, INFINITE, 133),
            &READ_QUAD(1, INFINITE, 133),
            &READ_QUAD_ADDR(3, INFINITE, 133),
            0
        },

        {
            &WRITE_STD(0, 256, 133),
            &WRITE_QUAD_ADDR(0, 256, 133),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 133),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25V1635F  3.3V */
    {
        "MX25V1635F",  {0xc2, 0x23, 0x15}, 3, _2M,    _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 33),
            &WRITE_QUAD_ADDR(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25R6435F Wide Voltage Range 1.65~3.6V */
    {
        "MX25R6435F", {0xc2, 0x28, 0x17}, 3, _8M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 33),
            &WRITE_QUAD_ADDR(0, 256, 33),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 33),
            0
        },
        &spi_driver_mx25l25635e,
    },


    /* MX25U1633F, 1.65-2.0V */
    {
        "MX25U1633F", {0xc2, 0x25, 0x35}, 3, _2M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 33),
            &WRITE_QUAD_ADDR(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_mx25l25635e,
    },


    /* MX25U6435F, 1.65-2.0V */
    {
        "MX25U6435F", {0xc2, 0x25, 0x37}, 3, _8M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(3, INFINITE, 108),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD_ADDR(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25U12835F, 1.65-2.0V */
    {
        "MX25U12835F", {0xc2, 0x25, 0x38}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 55),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(3, INFINITE, 108),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD_ADDR(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_mx25l25635e,
    },

    {
        "MX25L128XX", {0xc2, 0x20, 0x18}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
#ifdef CONFIG_DTR_MODE_SUPPORT
            &READ_QUAD_DTR(10, INFINITE, 100/* 84 */),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD_ADDR(0, 256, 104),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25U25635F/MX25U25645G, 1.65-2.0V */
    {
        "MX25U25635F/45G", {0xc2, 0x25, 0x39}, 3, _32M, _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 55),
            &READ_FAST4B(1, INFINITE, 108),
            &READ_DUAL4B(1, INFINITE, 108),
            &READ_DUAL_ADDR4B(1, INFINITE, 108),
            &READ_QUAD4B(1, INFINITE, 108),
            &READ_QUAD_ADDR4B(3, INFINITE, 108),
#ifdef CONFIG_DTR_MODE_SUPPORT
            &READ_QUAD_DTR4B(10, INFINITE, 100),
#endif
            0
        },

        {
            &WRITE_STD4B(0, 256, 80),
            &WRITE_QUAD_ADDR4B(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 108),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25L25635F/MX25L25645G/MX25L25735F/MX25L25635E, 3.3V */
    {
        "MX25L(256/257)XX",
        {0xc2, 0x20, 0x19}, 3, _32M, _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 50),
            &READ_FAST4B(1, INFINITE, 80/* 84 */),
            &READ_DUAL4B(1, INFINITE, 80/* 84 */),
            &READ_DUAL_ADDR4B(1, INFINITE, 80/* 84 */),
            &READ_QUAD4B(1, INFINITE, 80/* 84 */),
            &READ_QUAD_ADDR4B(3, INFINITE, 80/* 84 */),
#ifdef CONFIG_DTR_MODE_SUPPORT
            &READ_QUAD_DTR4B(10, INFINITE, 100/* 84 */),
#endif
            0
        },

        {
            &WRITE_STD4B(0, 256, 120/* 133 */),
            &WRITE_QUAD_ADDR4B(0, 256, 120/* 133 */),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 120/* 133 */),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* MX25U51245G, 1.65-2.0V */
    {
        "MX25U51245G", {0xc2, 0x25, 0x3a}, 3, _64M, _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 66),
            &READ_FAST4B(1, INFINITE, 104),
            &READ_DUAL4B(1, INFINITE, 104),
            &READ_DUAL_ADDR4B(1, INFINITE, 104),
            &READ_QUAD4B(1, INFINITE, 104),
            &READ_QUAD_ADDR4B(3, INFINITE, 104),
#ifdef CONFIG_DTR_MODE_SUPPORT
            &READ_QUAD_DTR4B(10, INFINITE, 100),
#endif
            0
        },

        {
            &WRITE_STD4B(0, 256, 104),
            &WRITE_QUAD_ADDR4B(0, 256, 104),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 104),
            0
        },
        &spi_driver_mx25l25635e,
    },

    /* Spansion/Cypress  S25FL129P1 3.3V */
    {
        "S25FL129P1",
        {0x01, 0x20, 0x18, 0x4d, 0x01, 0x80}, 6, (_64K * 256),  _64K,  3,
        {
            &READ_STD(0, INFINITE, 40),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 64),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_general,
    },

    /* Spansion/Cypress S25FL256S 3.3V */
    {
        "S25FL256S", {0x01, 0x02, 0x19, 0x4d, 0x01, 0x80}, 6, _32M,  _64K,  4,
        {
            &READ_STD4B(0, INFINITE, 50),
            &READ_FAST4B(1, INFINITE, 133),
            &READ_DUAL4B(1, INFINITE, 104),
            &READ_DUAL_ADDR4B(1, INFINITE, 104),
            &READ_QUAD4B(1, INFINITE, 104),
            &READ_QUAD_ADDR4B(3, INFINITE, 104),
            0
        },

        {
            &WRITE_STD4B(0, 256, 104),
            &WRITE_QUAD4B(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 104),
            0
        },
        &spi_driver_s25fl256s,
    },

    /* Micron  N25Q064A 1.8V */
    {
        "N25Q064A",   {0x20, 0xbb, 0x17}, 3, (_64K * 128), _64K, 3,
        {
            &READ_STD(0, INFINITE, 54),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(2, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_DUAL(0, 256, 108),
            &WRITE_DUAL_ADDR(0, 256, 108),
            &WRITE_QUAD(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_general,
    },

    /* Micron "N25Q128A11/MT25QU128AB" 1.8V */
    {
        "(MT)N25Q(U)128A",   {0x20, 0xbb, 0x18}, 3,
        (_64K * 256), _64K, 3,
        {
            &READ_STD(0, INFINITE, 54),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(2, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(5, INFINITE, 108),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_DUAL(0, 256, 108),
            &WRITE_DUAL_ADDR(0, 256, 108),
            &WRITE_QUAD(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_micron,
    },

    /* Micron  N25QL064A 3.3V */
    {
        "N25QL064A",   {0x20, 0xba, 0x17}, 3, (_64K * 128), _64K, 3,
        {
            &READ_STD(0, INFINITE, 54),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(2, INFINITE, 84),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(5, INFINITE, 84),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_DUAL(0, 256, 75),
            &WRITE_DUAL_ADDR(0, 256, 75),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 50),
            0
        },
        &spi_driver_micron,
    },

    /* Micron  MT(N)25QL128A 3.3V */
    {
        "N25QL128A",   {0x20, 0xba, 0x18}, 3, (_64K * 256), _64K, 3,
        {
            &READ_STD(0, INFINITE, 54),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 84),
            &READ_DUAL_ADDR(2, INFINITE, 84),
            &READ_QUAD(1, INFINITE, 84),
            &READ_QUAD_ADDR(5, INFINITE, 84),
            0
        },

        {
            &WRITE_STD(0, 256, 108),
            &WRITE_DUAL(0, 256, 108),
            &WRITE_DUAL_ADDR(0, 256, 108),
            &WRITE_QUAD(0, 256, 108),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_micron,
    },

    /* Micron MT25QL256A 3.3V */
    {
        "MT25QL256A",   {0x20, 0xba, 0x19, 0x10, 0x44}, 5, (_64K * 512), _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 54),
            &READ_FAST4B(1, INFINITE, 133),
            &READ_DUAL4B(1, INFINITE, 133),
            &READ_DUAL_ADDR4B(2, INFINITE, 133),
            &READ_QUAD4B(1, INFINITE, 133),
            &READ_QUAD_ADDR4B(5, INFINITE, 125),
            0
        },

        {
            &WRITE_STD4B(0, 256, 133),
            &WRITE_DUAL4B(0, 256, 133),
            &WRITE_DUAL_ADDR4B(0, 256, 133),
            &WRITE_QUAD4B(0, 256, 133),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 133),
            0
        },
        &spi_driver_micron,
    },

    /* Micron "MT25QU256A" 1.8V */
    {
        "MT25QU256A",   {0x20, 0xbb, 0x19}, 3, (_64K * 512), _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 54),
            &READ_FAST4B(1, INFINITE, 80),
            &READ_DUAL4B(1, INFINITE, 80),
            &READ_DUAL_ADDR4B(2, INFINITE, 84),
            &READ_QUAD4B(1, INFINITE, 80),
            &READ_QUAD_ADDR4B(5, INFINITE, 84),
            0
        },

        {
            &WRITE_STD4B(0, 256, 80),
            &WRITE_DUAL4B(0, 256, 75),
            &WRITE_DUAL_ADDR4B(0, 256, 75),
            &WRITE_QUAD4B(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 80),
            0
        },
        &spi_driver_micron,
    },

    /* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
    {
        "W25Q16JV-IQ/S25FL016K",
        {0xef, 0x40, 0x15}, 3, (_64K * 32), _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

    {
        "W25Q32(B/F)V",  {0xef, 0x40, 0x16}, 3, (_64K * 64),   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

    /* winbond w25q32fw is 1.8v */
    {
        "W25Q32FW",  {0xef, 0x60, 0x16}, 3, _4M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 133),
            &READ_DUAL(1, INFINITE, 133),
            &READ_QUAD(1, INFINITE, 133),
            &READ_QUAD_ADDR(3, INFINITE, 133),
            0
        },

        {
            &WRITE_STD(0, 256, 133),
            &WRITE_QUAD(0, 256, 133),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 133),
            0
        },
        &spi_driver_w25q256fv,
    },

    {
        "W25Q64FV(SPI)/W25Q64JV_IQ",  {0xef, 0x40, 0x17}, 3, _8M,   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_general,
    },

    /* winbond w25q64fw is 1.8v */
    {
        "W25Q64FW",  {0xef, 0x60, 0x17}, 3, _8M,   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* winbond w25q128fw is 1.8v */
    {
        "W25Q128FW",  {0xef, 0x60, 0x18}, 3, _16M,   _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    {
        "W25Q128(B/F)V", {0xEF, 0x40, 0x18}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            // &READ_FAST(1, INFINITE, 60),
            // &READ_FAST(1, INFINITE, 104),
            // &READ_DUAL(1, INFINITE, 104),
            // &READ_QUAD(1, INFINITE, /* 70 */80),
            0
        },

        {
            &WRITE_STD(0, 256, 33),
            // &WRITE_STD(0, 256, 104),
            // &WRITE_QUAD(0, 256, /* 70 */80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 33),
            // &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* "W25Q128JV_IM" can support DTR mode 80MHz */
    {
        "W25Q128JV_IM", {0xEF, 0x70, 0x18}, 3, _16M, _64K, 3,
        {
            &READ_STD(0, INFINITE, 33),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, /* 70 */80),
#ifdef CONFIG_DTR_MODE_SUPPORT
            &READ_QUAD_DTR(8, INFINITE, 80),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, /* 70 */80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* Winbond W25Q256FV */
    {
        "W25Q256FV", {0xEF, 0x40, 0x19}, 3, _32M, _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 50),
            &READ_FAST4B(1, INFINITE, 80),
            &READ_DUAL4B(1, INFINITE, 80),
            &READ_DUAL_ADDR4B(1, INFINITE, 80),
            &READ_QUAD4B(1, INFINITE, 80),
            &READ_QUAD_ADDR4B(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD4B(0, 256, 80),
            &WRITE_QUAD4B(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 104),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* Winbond W25Q256JWFIM  1.8V */
    {
        "W25Q256JWFIM", {0xEF, 0x80, 0x19}, 3, _32M, _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 50),
            &READ_FAST4B(1, INFINITE, 133),
            &READ_DUAL4B(1, INFINITE, 133),
            &READ_DUAL_ADDR4B(1, INFINITE, 133),
            &READ_QUAD4B(1, INFINITE, 133),
            &READ_QUAD_ADDR4B(3, INFINITE, 133),
            0
        },

        {
            &WRITE_STD4B(0, 256, 133),
            &WRITE_QUAD4B(0, 256, 133),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 133),
            0
        },
        &spi_driver_w25q256fv,
    },

    /* ESMT/CFEON */
    {
        "EN25Q32B", {0x1c, 0x30, 0x16}, 3, (_64K * 64),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 80/* 104 */),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            /* &READ_QUAD(3, INFINITE, 80), */
            0
        },

        {
            &WRITE_STD(0, 256, 80/* 104 */),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 80/* 104 */),
            0
        },
        &spi_driver_general,
    },

    {
        "EN25Q64", {0x1c, 0x30, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 100),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    /* CFEON EN25QH64A 3.3V */
    {
        "EN25QH64A", {0x1c, 0x70, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
#ifndef CONFIG_CLOSE_SPI_8PIN_4IO
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
#endif
            0
        },

        {
            &WRITE_STD(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    {
        "EN25Q128", {0x1c, 0x30, 0x18}, 3, (_64K * 256),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            0
        },

        {
            &WRITE_STD(0, 256, 104),
            0
        },

        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    /* ESMT */
    {
        "F25L64QA", {0x8C, 0x41, 0x17}, 3, (_64K * 128),  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_FAST(1, INFINITE, /* 66 */100),
            &READ_DUAL(1, INFINITE, /* 66 */80),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, /* 66 */100),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, /* 66 */100),
            0
        },
        &spi_driver_f25l64q,
    },

    /* GD GD25LQ128 1.8V */
    {
        "GD25LQ128", {0xC8, 0x60, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q256", {0xC8, 0x40, 0x19}, 3, _32M,  _64K, 4,
        {
            &READ_STD4B(0, INFINITE, 50),
            &READ_FAST4B(1, INFINITE, 80),
            &READ_DUAL4B(1, INFINITE, 80),
            &READ_DUAL_ADDR4B(1, INFINITE, 80),
            &READ_QUAD4B(1, INFINITE, 80),
            &READ_QUAD_ADDR4B(3, INFINITE, 80),
            0
        },

        {
            &WRITE_STD4B(0, 256, 80),
            &WRITE_QUAD4B(0, 256, 80),
            0
        },

        {
            &ERASE_SECTOR_64K4B(0, _64K, 104),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q128", {0xC8, 0x40, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 100),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q64", {0xC8, 0x40, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 100),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q16C", {0xC8, 0x40, 0x15}, 3, _2M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_gd25qxxx,
    },

    /* GD GD25LQ16C 1.65~2.1V */
    {
        "GD25LQ16C", {0xC8, 0x60, 0x15}, 3, _2M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_gd25qxxx,
    },

    /* GD GD25LQ64C 1.8V */
    {
        "GD25LQ64C", {0xC8, 0x60, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 80),
            &READ_DUAL_ADDR(1, INFINITE, 80),
            &READ_QUAD(1, INFINITE, 80),
            &READ_QUAD_ADDR(3, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_gd25qxxx,
    },

    {
        "GD25Q32", {0xC8, 0x40, 0x16}, 3, _4M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 66),
            &READ_QUAD(1, INFINITE, 80),
            0
        },
        {
            &WRITE_STD(0, 256, 100),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 100),
            0
        },
        &spi_driver_gd25qxxx,
    },

    /* Paragon 3.3V */
    {
        "PN25F16S", {0xe0, 0x40, 0x15}, 3, _2M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 55),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            0
        },
        {
            &WRITE_STD(0, 256, 108),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_general,
    },

    {
        "PN25F32S", {0xe0, 0x40, 0x16}, 3, _4M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 55),
            &READ_FAST(1, INFINITE, 108),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(3, INFINITE, 108),
            0
        },
        {
            &WRITE_STD(0, 256, 108),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 108),
            0
        },
        &spi_driver_general,
    },

    /* XMC */
    {
        "XM25QH64AHIG", {0x20, 0x70, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    /* XMC 3.3v */
    {
        "XM25QH128A", {0x20, 0x70, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

        /* XMC */
    {
        "XM25QH64B", {0x20, 0x60, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },

    /* XMC 3.3v */
    {
        "XM25QH128B", {0x20, 0x60, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 50),
            &READ_FAST(1, INFINITE, 104),
            &READ_DUAL(1, INFINITE, 104),
            &READ_DUAL_ADDR(1, INFINITE, 104),
            &READ_QUAD(1, INFINITE, 104),
            &READ_QUAD_ADDR(3, INFINITE, 104),
            0
        },
        {
            &WRITE_STD(0, 256, 104),
            &WRITE_QUAD(0, 256, 104),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 104),
            0
        },
        &spi_driver_no_qe,
    },
    
    /* XTX 3.3v */
    {
        "XT25F128BSSI/HGU", {0x0B, 0x40, 0x18}, 3, _16M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 108),
            &READ_QUAD_ADDR(3, INFINITE, 72),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_xtx,
    },

    {
        "XT25F64BSSI/HGU-S", {0x0B, 0x40, 0x17}, 3, _8M,  _64K, 3,
        {
            &READ_STD(0, INFINITE, 80),
            &READ_DUAL(1, INFINITE, 108),
            &READ_DUAL_ADDR(1, INFINITE, 108),
            &READ_QUAD(1, INFINITE, 84),
            &READ_QUAD_ADDR(3, INFINITE, 72),
            0
        },
        {
            &WRITE_STD(0, 256, 80),
            &WRITE_QUAD(0, 256, 80),
            0
        },
        {
            &ERASE_SECTOR_64K(0, _64K, 80),
            0
        },
        &spi_driver_xtx,
    },

    {0, {0}, 0, 0, 0, 0, {0}, {0}, {0}, NULL},
};

/*****************************************************************************/
static struct spi_nor_info *hifmc_spi_nor_serach_ids(u_char ids[8])
{
    struct spi_nor_info *info = hifmc_spi_nor_info_table;
    struct spi_nor_info *fit_info = NULL;

    for (; info->name; info++) {
        if (memcmp(info->id, ids, info->id_len)) {
            continue;
        }

        if ((fit_info == NULL) || (fit_info->id_len < info->id_len)) {
            fit_info = info;
        }
    }
    return fit_info;
}

/*****************************************************************************/
static void hifmc_spi_nor_search_rw(struct spi_nor_info *info,
                                    struct spi_op *spiop_rw, u_int iftype, u_int max_dummy, int rw_type)
{
    int ix = 0;
    struct spi_op **spiop, **fitspiop;

    for (fitspiop = spiop = (rw_type ? info->write : info->read);
            (*spiop) && ix < MAX_SPI_OP; spiop++, ix++) {
        if (((*spiop)->iftype & iftype)
                && ((*spiop)->dummy <= max_dummy)
                && (*fitspiop)->iftype < (*spiop)->iftype) {
            fitspiop = spiop;
        }
    }
    memcpy(spiop_rw, (*fitspiop), sizeof(struct spi_op));
}

/*****************************************************************************/
static void hifmc_map_iftype_and_clock(struct hifmc_spi *spi)
{
    int ix;
    const int iftype_read[] = {
        SPI_IF_READ_STD,    IF_TYPE_STD,
        SPI_IF_READ_FAST,   IF_TYPE_STD,
        SPI_IF_READ_DUAL,   IF_TYPE_DUAL,
        SPI_IF_READ_DUAL_ADDR,  IF_TYPE_DIO,
        SPI_IF_READ_QUAD,   IF_TYPE_QUAD,
        SPI_IF_READ_QUAD_ADDR,  IF_TYPE_QIO,
#ifdef CONFIG_DTR_MODE_SUPPORT
        SPI_IF_READ_QUAD_DTR,   IF_TYPE_DTR,
#endif
        0,          0,
    };
    const int iftype_write[] = {
        SPI_IF_WRITE_STD,   IF_TYPE_STD,
        SPI_IF_WRITE_DUAL,  IF_TYPE_DUAL,
        SPI_IF_WRITE_DUAL_ADDR, IF_TYPE_DIO,
        SPI_IF_WRITE_QUAD,  IF_TYPE_QUAD,
        SPI_IF_WRITE_QUAD_ADDR, IF_TYPE_QIO,
        0,          0,
    };

    for (ix = 0; iftype_write[ix]; ix += 2) {
        if (spi->write->iftype == iftype_write[ix]) {
            spi->write->iftype = iftype_write[ix + 1];
            break;
        }
    }
    hifmc_get_fmc_best_2x_clock(&spi->write->clock);

    for (ix = 0; iftype_read[ix]; ix += 2) {
        if (spi->read->iftype == iftype_read[ix]) {
            spi->read->iftype = iftype_read[ix + 1];
            break;
        }
    }
#ifdef CONFIG_DTR_MODE_SUPPORT
    if (spi->dtr_mode_support) {
        /* get the div4 clock */
        hifmc_get_fmc_best_4x_clock(&spi->read->clock);
    } else {
        hifmc_get_fmc_best_2x_clock(&spi->read->clock);
    }
#else
    hifmc_get_fmc_best_2x_clock(&spi->read->clock);
#endif

    hifmc_get_fmc_best_2x_clock(&spi->erase->clock);
    spi->erase->iftype = IF_TYPE_STD;
}

/*****************************************************************************/
void hifmc_spi_nor_get_erase(struct spi_nor_info *info,
                             struct spi_op *spiop_erase)
{
    int ix;

    spiop_erase->size = 0;
    for (ix = 0; ix < MAX_SPI_OP; ix++) {
        if (info->erase[ix] == NULL) {
            break;
        }
        if (info->erasesize == info->erase[ix]->size) {
            memcpy(&spiop_erase[ix], info->erase[ix],
                   sizeof(struct spi_op));
            break;
        }
    }
}

/*****************************************************************************/
int hifmc_spi_nor_probe(struct mtd_info_ex *mtd, struct hifmc_spi *spi)
{
    unsigned char cs, ids[MAX_SPI_NOR_ID_LEN] = {0}, ix, len = 0;
    unsigned int total = 0;
    char buffer[100];
#if BT_DBG
    const char *str[] = {"STD", "DUAL", "DIO", "QUAD", "QIO"};
#endif

    FMC_PR(BT_DBG, "\t||*-Start probe SPI nor flash\n");

    mtd->numchips = 0;

    FMC_PR(FMC_INFO, "SPI Nor ID Table Version %s\n", SPI_NOR_ID_TBL_VER);

    for (cs = 0; cs < CONFIG_SPI_NOR_MAX_CHIP_NUM; cs++) {
        if (hifmc_cs_user[cs]) {
            FMC_PR(BT_DBG, "\t|||-CS(%d) is occupied\n", cs);
            continue;
        }

        hifmc100_read_ids(spi, cs, ids);

        /* can't find spi flash device. */
        if (!(ids[0] | ids[1] | ids[2])
                || ((ids[0] & ids[1] & ids[2]) == 0xFF)) {
            continue;
        }

        len = sprintf(buffer, "SPI Nor(cs %d) ID: %#x %#x %#x", cs,
                      ids[0], ids[1], ids[2]);

        spiinfo = hifmc_spi_nor_serach_ids(ids);
        for (ix = 3; (spiinfo) && (ix < spiinfo->id_len); ix++) {
            len += sprintf(buffer + len, " %#x", ids[ix]);
        }

        if (spiinfo) {
            FMC_PR(FMC_INFO, "%s\n", buffer);

            FMC_PR(BT_DBG, "\t|||-CS-%d found SPI nor flash: %s\n",
                   cs, spiinfo->name);

            spi->name = spiinfo->name;
            spi->chipselect = cs;
            spi->chipsize = spiinfo->chipsize;
            spi->erasesize = spiinfo->erasesize;
            spi->addrcycle = spiinfo->addrcycle;
            spi->driver = spiinfo->driver;

#ifdef CONFIG_DTR_MODE_SUPPORT
            /* to check weather current device support DTR mode */
            hifmc_check_spi_dtr_support(spi, ids);
            if(spi->dtr_mode_support) {
                /* to match the best dummy/if_type/clock */
                hifmc_spi_nor_search_rw(spiinfo, spi->read,
                                        HIFMC_SPI_NOR_SUPPORT_READ,
                                        HIFMC_SPI_NOR_DTR_MAX_DUMMY, RW_OP_READ);
            } else {
                hifmc_spi_nor_search_rw(spiinfo, spi->read,
                                        HIFMC_SPI_NOR_SUPPORT_READ,
                                        HIFMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_READ);
            }
#else
            /* to match the best dummy/if_type/clock */
            hifmc_spi_nor_search_rw(spiinfo, spi->read,
                                    HIFMC_SPI_NOR_SUPPORT_READ,
                                    HIFMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_READ);
#endif
            hifmc_spi_nor_search_rw(spiinfo, spi->write,
                                    HIFMC_SPI_NOR_SUPPORT_WRITE,
                                    HIFMC_SPI_NOR_STR_MAX_DUMMY, RW_OP_WRITE);

            hifmc_spi_nor_get_erase(spiinfo, spi->erase);
            hifmc_map_iftype_and_clock(spi);

            spi->driver->qe_enable(spi);

            /* auto check fmc_addr_mode 3 bytes or 4 bytes */
            start_up_addr_mode = GET_FMC_BOOT_MODE;

            if ((spi->addrcycle == SPI_NOR_3BYTE_ADDR_LEN)
                    && (start_up_addr_mode == SPI_NOR_ADDR_MODE_4_BYTES)) {
                printf("\nError!!! the flash's addres len is 3bytes and start up address mode is 4bytes,");
                printf("please set the start up address mode to 3bytes mode\n");
            }
            if ((spi->addrcycle == SPI_NOR_4BYTE_ADDR_LEN)
                    && (start_up_addr_mode == SPI_NOR_ADDR_MODE_3_BYTES)) {
                switch (ids[0]) {
                    case MID_WINBOND:
                    case MID_MXIC:
                    case MID_MICRON:
                        FMC_PR(BT_DBG, "\t|||-4-Byte Command Operation\n");
                        break;
                    default:
                        FMC_PR(BT_DBG, "\t|||-start up: 3-Byte mode\n");
                        spi->driver->entry_4addr(spi, ENABLE);
                        break;
                }
            } else {
                FMC_PR(BT_DBG, "\t|||-start up: 4-Byte mode or 4-Byte Command\n");
            }

            FMC_PR(FMC_INFO, "Block:%sB ", ulltostr(spi->erasesize));
            FMC_PR(FMC_INFO, "Chip:%sB ", ulltostr(spi->chipsize));
            FMC_PR(FMC_INFO, "Name:\"%s\"\n", spi->name);
#if BT_DBG
            DB_MSG("\t|||-Read if: %s, cmd: %#X, clock reg: %#x\n",
                   str[spi->read->iftype],
                   spi->read->cmd, spi->read->clock);
            DB_MSG("\t|||-Write if: %s, cmd: %#X, clock reg: %#x\n",
                   str[spi->write->iftype],
                   spi->write->cmd, spi->write->clock);
            DB_MSG("\t|||-Erase if: %s, cmd: %#X, clock reg: %#x\n",
                   str[spi->erase[0].iftype],
                   spi->erase[0].cmd, spi->erase[0].clock);
#endif

            if (mtd->type == 0) {
                mtd->type = MTD_NORFLASH;
                mtd->chipsize = spi->chipsize;
                mtd->erasesize = spi->erasesize;
                mtd->pagesize = 1;
                mtd->addrcycle = spi->addrcycle;

                if (spiinfo->id_len > sizeof(mtd->ids)) {
                    printf("BUG! ID len out of range.\n");
                    BUG();
                }

                mtd->id_length = spiinfo->id_len;
                memcpy(mtd->ids, spiinfo->id, spiinfo->id_len);
                strncpy(mtd->name, spi->name,
                        sizeof(mtd->name));
                mtd->name[sizeof(mtd->name) - 1] = '\0';
            }

            mtd->numchips++;
            total += spi->chipsize;
            spi++;
            hifmc_cs_user[cs]++;
        } else
            printf("SPI Nor(cs %d) ID: %#x %#x %#x can't find"
                   " in the ID table !!!\n", cs, ids[0], ids[1], ids[2]);


    }

    FMC_PR(BT_DBG, "\t||*-End probe SPI nor flash, num: %d\n",
           mtd->numchips);

    return mtd->numchips;
}

/*****************************************************************************/
#ifdef CONFIG_DTR_MODE_SUPPORT
void spi_dtr_to_sdr_switch(struct hifmc_spi *spi)
{
    unsigned int ix = 0, spi_dtr_dummy;
    struct spi_op **spiop, **fitspiop;
    const int iftype_read[] = {
        SPI_IF_READ_QUAD,   IF_TYPE_QUAD,
        SPI_IF_READ_QUAD_ADDR,  IF_TYPE_QIO,
        0,          0,
    };

    /* the dummy in SDR mode is impossible equal to DTR */
    spi_dtr_dummy = spi->read->dummy;

    /* match the best clock and dummy value agian */
    for (fitspiop = spiop = spiinfo->read;
            (*spiop) && ix < MAX_SPI_OP; spiop++, ix++) {
        if (((*spiop)->iftype & HIFMC_SPI_NOR_SUPPORT_READ)
                && ((*spiop)->dummy != spi_dtr_dummy)
                && (*fitspiop)->iftype < (*spiop)->iftype) {
            fitspiop = spiop;
        }
    }
    memcpy(spi->read, (*fitspiop), sizeof(struct spi_op));

    /* to map the iftype and clock of SDR mode */
    for (ix = 0; iftype_read[ix]; ix += 2) {
        if (spi->read->iftype == iftype_read[ix]) {
            spi->read->iftype = iftype_read[ix + 1];
            break;
        }
    }
    hifmc_get_fmc_best_2x_clock(&spi->read->clock);
}
#endif /* CONFIG_DTR_MODE_SUPPORT */
