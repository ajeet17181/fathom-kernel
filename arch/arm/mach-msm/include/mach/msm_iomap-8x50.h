/* arch/arm/mach-msm/include/mach/msm_iomap-8x50.h
 *
 * Copyright (C) 2007 Google, Inc.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * The MSM peripherals are spread all over across 768MB of physical
 * space, which makes just having a simple IO_ADDRESS macro to slide
 * them into the right virtual location rough.  Instead, we will
 * provide a master phys->virt mapping for peripherals here.
 *
 */

#ifndef __ASM_ARCH_MSM_IOMAP_8X50_H
#define __ASM_ARCH_MSM_IOMAP_8X50_H

#define MSM_VIC_BASE          IOMEM(0xF8000000)
#define MSM_VIC_PHYS          0xAC000000
#define MSM_VIC_SIZE          SZ_4K

#define MSM_CSR_BASE          IOMEM(0xF8001000)
#define MSM_CSR_PHYS          0xAC100000
#define MSM_CSR_SIZE          SZ_4K

#define MSM_GPT_PHYS          MSM_CSR_PHYS
#define MSM_GPT_BASE          MSM_CSR_BASE
#define MSM_GPT_SIZE          SZ_4K

#define MSM_DMOV_BASE         IOMEM(0xF8002000)
#define MSM_DMOV_PHYS         0xA9700000
#define MSM_DMOV_SIZE         SZ_4K

#define MSM_GPIO1_BASE        IOMEM(0xF8003000)
#define MSM_GPIO1_PHYS        0xA9000000
#define MSM_GPIO1_SIZE        SZ_4K

#define MSM_GPIO2_BASE        IOMEM(0xF8004000)
#define MSM_GPIO2_PHYS        0xA9100000
#define MSM_GPIO2_SIZE        SZ_4K

#define MSM_GPIOCFG1_BASE IOMEM(0xF9004000)
#define MSM_GPIOCFG1_PHYS 0xA8E00000
#define MSM_GPIOCFG1_SIZE SZ_4K

#define MSM_GPIOCFG2_BASE IOMEM(0xF9005000)
#define MSM_GPIOCFG2_PHYS 0xA8F00000
#define MSM_GPIOCFG2_SIZE SZ_4K

#define MSM_CLK_CTL_BASE      IOMEM(0xF8005000)
#define MSM_CLK_CTL_PHYS      0xA8600000
#define MSM_CLK_CTL_SIZE      SZ_4K

#define MSM_CLK_CTL_SH2_BASE  IOMEM(0xF8006000)
#define MSM_CLK_CTL_SH2_PHYS  0xABA01000
#define MSM_CLK_CTL_SH2_SIZE  SZ_4K

#define MSM_TS_BASE           IOMEM(0xF9006000)
#define MSM_TS_PHYS           0xAA300000
#define MSM_TS_SIZE           SZ_4K

#define MSM_SSBI_BASE         IOMEM(0xF9008000)
#define MSM_SSBI_PHYS         0xA8100000
#define MSM_SSBI_SIZE         SZ_4K

#define MSM_TSSC_BASE         IOMEM(0xF9009000)
#define MSM_TSSC_PHYS         0xAA300000
#define MSM_TSSC_SIZE         SZ_4K

#define MSM_SHARED_RAM_BASE   IOMEM(0xF8100000)
#define MSM_SHARED_RAM_PHYS   0x00100000
#define MSM_SHARED_RAM_SIZE   SZ_1M

#define MSM_UART1_PHYS        0xA9A00000
#define MSM_UART1_SIZE        SZ_4K

#define MSM_UART2_PHYS        0xA9B00000
#define MSM_UART2_SIZE        SZ_4K

#define MSM_UART3_PHYS        0xA9C00000
#define MSM_UART3_SIZE        SZ_4K

#ifdef CONFIG_MSM_DEBUG_UART
#define MSM_DEBUG_UART_BASE   0xF9000000
#if CONFIG_MSM_DEBUG_UART == 1
#define MSM_DEBUG_UART_PHYS   MSM_UART1_PHYS
#elif CONFIG_MSM_DEBUG_UART == 2
#define MSM_DEBUG_UART_PHYS   MSM_UART2_PHYS
#elif CONFIG_MSM_DEBUG_UART == 3
#define MSM_DEBUG_UART_PHYS   MSM_UART3_PHYS
#endif
#define MSM_DEBUG_UART_SIZE   SZ_4K
#endif

#define MSM_SDC1_PHYS         0xA0300000
#define MSM_SDC1_SIZE         SZ_4K
#define MSM_SDC2_BASE         IOMEM(0xF800C000)
#define MSM_SDC2_PHYS         0xA0400000
#define MSM_SDC2_SIZE         SZ_4K
#define MSM_SDC3_PHYS         0xA0500000
#define MSM_SDC3_SIZE         SZ_4K
#define MSM_SDC4_PHYS         0xA0600000
#define MSM_SDC4_SIZE         SZ_4K

#define MSM_I2C_PHYS          0xA9900000
#define MSM_I2C_SIZE          SZ_4K

#define MSM_HSUSB_PHYS        0xA0800000
#define MSM_HSUSB_SIZE        SZ_4K

#define MSM_PMDH_PHYS         0xAA600000
#define MSM_PMDH_SIZE         SZ_4K

#define MSM_EMDH_PHYS         0xAA700000
#define MSM_EMDH_SIZE         SZ_4K

#define MSM_MDP_PHYS          0xAA200000
#define MSM_MDP_SIZE          0x000F0000

#define MSM_MDC_BASE	      IOMEM(0xF8200000)
#define MSM_MDC_PHYS	      0xAA500000
#define MSM_MDC_SIZE	      SZ_1M

#define MSM_AD5_BASE          IOMEM(0xF8300000)
#define MSM_AD5_PHYS          0xAC000000
#define MSM_AD5_SIZE          (SZ_1M*13)

#define MSM_VFE_PHYS          0xA0F00000
#define MSM_VFE_SIZE          SZ_1M

#define MSM_UART1DM_PHYS      0xA0200000
#define MSM_UART2DM_PHYS      0xA0300000

#define MSM_SIRC_BASE         IOMEM(0xF8006000)
#define MSM_SIRC_PHYS         0xAC200000
#define MSM_SIRC_SIZE         SZ_4K

#define MSM_SCPLL_BASE        IOMEM(0xF8007000)
#define MSM_SCPLL_PHYS        0xA8800000
#define MSM_SCPLL_SIZE        SZ_4K

#if defined(CONFIG_CACHE_L2X0)
#define MSM_L2CC_BASE         IOMEM(0xF8008000)
#define MSM_L2CC_PHYS         0xC0400000
#define MSM_L2CC_SIZE         SZ_4K
#endif

#define MSM_GPU_REG_PHYS      0xA0000000
#define MSM_GPU_REG_SIZE      0x00020000

#define MSM_SPI_PHYS          0xA1200000
#define MSM_SPI_SIZE          SZ_4K

// Originally this does not need to be defined,
// but is required to make early_ramconsole work.
// These values must match the values used in
// the defconfig.
#if 0
#define MSM_RAM_CONSOLE_BASE IOMEM(0xF9100000)
#define MSM_RAM_CONSOLE_PHYS 0x2FFC0000
#define MSM_RAM_CONSOLE_SIZE 0x00040000
#else
#define MSM_RAM_CONSOLE_BASE    IOMEM(0xF9100000)
#define MSM_RAM_CONSOLE_PHYS    0x03C00000
#define MSM_RAM_CONSOLE_SIZE    0x00040000
#endif


#define MSM_TCSR_BASE  IOMEM(0xF8008000)
#define MSM_TCSR_PHYS  0xA8700000
#define MSM_TCSR_SIZE  SZ_4K

#ifdef CONFIG_BUILD_CIQ
#define SMD_CIQ_BASE		0xFC11C
#endif

// For reading the real WiFi MAC address
#define MSM_SPLHOOD_BASE     IOMEM(0xF9200000)
#define MSM_SPLHOOD_PHYS     0x0
#define MSM_SPLHOOD_SIZE     SZ_1M

#endif
