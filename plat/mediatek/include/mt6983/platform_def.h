/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <drivers/mtk_gicv3_def.h>
#include <arch_def.h>
#include <memory_layout.h>
#include <mtk_common.h>

#define PLAT_PRIMARY_CPU	0x0
#define IO_PHYS          (0x10000000)
#define TOPCKGEN_BASE		(IO_PHYS)
#define TOPCKGEN_REG_SIZE	0x1000
#define VER_BASE                (0x08000000)

/*******************************************************************************
 * APMIXEDSYS related constants
 ******************************************************************************/
#define APMIXEDSYS		(IO_PHYS + 0x0000C000)
#define APMIXEDSYS_REG_SIZE	0x1000

/*******************************************************************************
 * APUSYS related constants
 ******************************************************************************/
#define DEBUGSYS_APB                (0x0d000000)
#define APU_MD32_DEBUG_APB          (DEBUGSYS_APB + 0x00298000)
#define APUSYS_CTRL_DAPC_AO_BASE    (IO_PHYS + 0x090FC000)
#define APUSYS_CTRL_DAPC_RCX_BASE   (IO_PHYS + 0x09034000)
#define APUSYS_NOC_DAPC_RCX_BASE    (IO_PHYS + 0x09038000)
#define APU_MD32_SYSCTRL            (IO_PHYS + 0x09001000)
#define APU_CACHE_DUMP              (IO_PHYS + 0x09050000)
#define APU_MD32_TCM                (IO_PHYS + 0x0d000000)
#define APU_MD32_WDT                (IO_PHYS + 0x09002000)
#define APU_SCTRL_REVISER           (IO_PHYS + 0x0903c000)
#define APU_AO_CTRL                 (IO_PHYS + 0x090f2000)
#define APU_MBOX0                   (IO_PHYS + 0x090e1000)
#define APU_MBOX1                   (IO_PHYS + 0x090e2000)
#define APU_RPCTOP                  (IO_PHYS + 0x090F0000)
#define APU_PCUTOP                  (IO_PHYS + 0x090F1000)
#define APU_ARETOP_ARE0             (IO_PHYS + 0x090f6000)
#define APU_ARETOP_ARE1             (IO_PHYS + 0x090f7000)
#define APU_ARETOP_ARE2             (IO_PHYS + 0x090f8000)
#define APU_RCX_CONFIG              (IO_PHYS + 0x09020000)
#define APU_RCX_VCORE_CONFIG        (IO_PHYS + 0x090E0000)
#define APU_ACS_RCX                 (IO_PHYS + 0x09044000)
#define APU_RPCTOP_LITE_ACX0        (IO_PHYS + 0x09140000)
#define APU_RPCTOP_LITE_ACX1        (IO_PHYS + 0x09240000)
#define APU_ACX0_CONFIG             (IO_PHYS + 0x0913C000)
#define APU_ACX1_CONFIG             (IO_PHYS + 0x0923C000)
#define APU_ACS_ACX0                (IO_PHYS + 0x09112000)
#define APU_ACS_ACX1                (IO_PHYS + 0x09212000)
#define APU_SEC_CON                 (IO_PHYS + 0x090F5000)
#define APU_PLL_COMMON              (IO_PHYS + 0x090F3000)
#define APU_LOGTOP                  (IO_PHYS + 0x09024000)

#define APU_ACS_RCX_SZ 0x1000
#define APU_RPCTOP_LITE_ACX0_SZ 0x1000
#define APU_RPCTOP_LITE_ACX1_SZ 0x1000
#define APU_ACX0_CONFIG_SZ 0x1000
#define APU_ACX1_CONFIG_SZ 0x1000
#define APU_ACS_ACX0_SZ 0x1000
#define APU_ACS_ACX1_SZ 0x1000
#define APU_SEC_CON_SZ 0x1000
#define APU_RPCTOP_SZ 0x1000
#define APU_PCUTOP_SZ 0x1000
#define APU_ARETOP_ARE0_SZ 0x1000
#define APU_ARETOP_ARE1_SZ 0x1000
#define APU_ARETOP_ARE2_SZ 0x1000
#define APU_PLL_COMMON_SZ 0x1000
#define APU_LOGTOP_SZ 0x1000

/*******************************************************************************
 * DBGTOP_DRM related constants
 ******************************************************************************/
#define MTK_DRM_BASE		 (IO_PHYS + 0x01FF0000)

/*******************************************************************************
 * dvfsrc related constants
 ******************************************************************************/
#define DVFSRC_BASE		(IO_PHYS + 0x0C00F000)

/*******************************************************************************
 * CKSYS related constants
 ******************************************************************************/
#define CKSYS_BASE			(IO_PHYS)

/*******************************************************************************
 * CPU cores constants
 ******************************************************************************/
#define PLATFORM_LITTLE_CORE_COUNT  4

/*******************************************************************************
 * PERI related constants
 ******************************************************************************/
#define PERI_APB_BASE		(IO_PHYS + 0x01000000)
#define PERICFG_AO_BASE		(IO_PHYS + 0x01036000)

/*******************************************************************************
 * MDPSYS related constants
 ******************************************************************************/
#define MDPSYS_BASE		(IO_PHYS + 0x0F000000)
#define MDPSYS_BASE_SIZE	0x1000

#define MDPSYS1_BASE		(IO_PHYS + 0x0F800000)
#define MDPSYS1_BASE_SIZE	0x1000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE		 (IO_PHYS + 0x01001000)
#define UART1_BASE		 (IO_PHYS + 0x01002000)
/* The UART port numbers of this SoC */
#define HW_SUPPORT_UART_PORTS	(2)
/*******************************************************************************
 * RGU related constants
 ******************************************************************************/
#define RGU_BASE		 (IO_PHYS + 0x0c007000)

/*******************************************************************************
 * SYSTIMER related constants
 ******************************************************************************/
#define SYSTIMER_BASE		 (IO_PHYS + 0x0c011000)

/*******************************************************************************
 * Infra related constants
 ******************************************************************************/
#define INFRACFG_AO_BASE		(IO_PHYS + 0x00001000)
#define INFRACFG_AO_REG_SIZE	(0x1000)

/*******************************************************************************
 * VLP related constants
 ******************************************************************************/
#define SRAMRC_APB_BASE		(IO_PHYS + 0x0C01F000)
#define SRAMRC_REG_SIZE		(0x1000)

/*******************************************************************************
 * EINT registers
 ******************************************************************************/
#define EINT_W_BASE		(IO_PHYS + 0x0000d000)
#define EINT_E_BASE		(IO_PHYS + 0x01ce0000)
#define EINT_S_BASE		(IO_PHYS + 0x01de0000)
#define EINT_N_BASE		(IO_PHYS + 0x01fe0000)
#define EINT_C_BASE		(IO_PHYS + 0x0c01e000)

/*******************************************************************************
 * SCP registers
 ******************************************************************************/
#define SCP_CLK_CTRL_BASE           (IO_PHYS + 0x0C721000)
#define SCP_CLK_CTRL_SIZE           (0x1000)
#define MTK_SCP_L2TCM_BASE          (IO_PHYS + 0x0C400000)
#define MTK_SCP_L2TCM_SIZE          (0x200000)
#define MTK_SCP_REG_BASE            (IO_PHYS + 0x0C700000)
#define MTK_SCP_REG_BANK_SIZE       (0x1000)

/*******************************************************************************
 * SPM related constants
 ******************************************************************************/
#define SPM_BASE		(IO_PHYS + 0x0C001000)
#define SPM_REG_SIZE		(0x1000)

/*******************************************************************************
 * VLP AO related constants
 ******************************************************************************/
#define VLP_AO_REG_BASE		(IO_PHYS + 0x0C000000)
#define VLP_AO_REG_SIZE		(0x1000)

/*******************************************************************************
 * SSPM CFGREG related constants
 ******************************************************************************/
#define SSPM_CFGREG_BASE	(IO_PHYS + 0x0C340000)
#define SSPM_CFGREG_SIZE	(0x1000)

/*******************************************************************************
 * MMSYS related constants
 ******************************************************************************/
#define MMSYS_BASE		(IO_PHYS + 0x04000000)
#define MMSYS_REG_SIZE		0x1000

/*******************************************************************************
 * Display Port related constants
 ******************************************************************************/
#define DP_BASE         (MMSYS_BASE + 0x00800000)
#define DP_REG_SIZE     0x1000

/*******************************************************************************
 * SSPM_MBOX_3 related constants
 ******************************************************************************/
#define SSPM_MBOX_3_BASE        (IO_PHYS + 0x0C380000)
#define SSPM_MBOX_3_SIZE	0x1000

/******************************************************************************
 * LPM syssram related constants
 *****************************************************************************/
#define MTK_LPM_SRAM_BASE	0x11B000
#define MTK_LPM_SRAM_MAP_SIZE	0x1000

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_HZ		13000000
#define SYS_COUNTER_FREQ_IN_MHZ		13

/*******************************************************************************
 * Devapc related constants
 ******************************************************************************/
#define DEVAPC_INFRA_AO_BASE		(IO_PHYS + 0x00030000)
#define DEVAPC_INFRA_AO1_BASE		(IO_PHYS + 0x00034000)
#define DEVAPC_PERI_PAR_AO_BASE		(IO_PHYS + 0x0103C000)
#define DEVAPC_VLP_AO_BASE		(IO_PHYS + 0x0C018000)
#define DEVAPC_ADSP_AO_BASE		(IO_PHYS + 0x0E01C000)
#define DEVAPC_MMINFRA_AO_BASE		(IO_PHYS + 0x0E820000)
#define DEVAPC_MMUP_AO_BASE		(IO_PHYS + 0x0ECA0000)
#define SRAMROM_BASE			(IO_PHYS + 0x00214000)
#define DEVAPC_REG_SIZE			(0x4000)

/*******************************************************************************
 * HACC related constants
 ******************************************************************************/
#define HACC_BASE           (IO_PHYS + 0x0C009000)

/*******************************************************************************
 * TRNG related constants
 ******************************************************************************/
#define TRNG_BASE           (IO_PHYS + 0x0020F000)

/*******************************************************************************
 * CRYPTO related constants
 ******************************************************************************/
#define CRYPTO_BASE        (IO_PHYS + 0x00210000)
#define VLPCK_BASE         (IO_PHYS + 0x0C013000)
#define VLPCFG_BASE        (IO_PHYS + 0x0C00C000)

/*******************************************************************************
 * GIC & interrupt handling related constants
 * Base MTK_platform compatible GIC memory map
 ******************************************************************************/
#define MT_GIC_BASE		(0x0C400000)
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)
#define BASE_GICR_BASE		(MT_GIC_RDIST_BASE)
#define MTK_GIC_REG_SIZE	0x400000
#define MTK_GIC_MAX_NUM		840

/*******************************************************************************
 * MCUSYS handling related constants
 ******************************************************************************/
#define MCUCFG_BASE		(0x0C000000)
#define MCUCFG_REG_SIZE		(0x50000)

/*******************************************************************************
 * DFD MCU handling related constants
 ******************************************************************************/
#define DEBUG_TOP_WRAP_BASE	(0x0C020000)

/*******************************************************************************
 * CPU_EB TCM handling related constants
 ******************************************************************************/
#define CPU_EB_TCM_BASE         (0x0C0AF000)
#define CPU_EB_TCM_SIZE		(0x1000)
#define CPU_EB_MBOX3_OFFSET	(0xCE0)

/*******************************************************************************
 * MFGSYS related constants
 ******************************************************************************/
#define MFGSYS_BASE         (IO_PHYS + 0x03000000)

/*******************************************************************************
 * Utility bus handling related constants
 ******************************************************************************/
#define MT_UTILITYBUS_BASE	(0x0C800000)
#define MT_UTILITYBUS_SIZE	(0x800000)

/*******************************************************************************
 * Modem related
 ******************************************************************************/
/* MD BOOT EN */
#define MD_BOOT_EN_BASE        (IO_PHYS + 0x10000000)
/* BYPASS_BROM */
#define MD_PERI_MISC_BASE      (0x20061000)
/* MD BOOT STATUS */
#define MD_BOOT_STATUS_BASE    (IO_PHYS + 0x0020E000)
/* Debug SYS */
#define DBGSYS_BASE            (0x0D000000)
#define MD_DUMP_DBG_APB        (IO_PHYS + 0x0C803000)

/*******************************************************************************
 * EFUSE address
 ******************************************************************************/
#define EFUSE_BASE          (IO_PHYS + 0x1EE0000)
#define EFUSE_REG_SIZE      0x1000

/*******************************************************************************
 * Thermal address
 ******************************************************************************/
#define THERM_CTRL_AP_BASE   (IO_PHYS + 0x00315000)
#define THERM_AP_REG_SIZE     0x1000

#define THERM_CTRL_MCU_BASE  (IO_PHYS + 0x00316000)
#define THERM_MCU_REG_SIZE     0x1000

/*******************************************************************************
 * INTID definitions
 ******************************************************************************/
#define SYSTIMER_IRQ_BIT_ID	344
#define WDT_IRQ_BIT_ID		681
#define FIQ_SMP_CALL_SGI	13
#define MD_WDT_IRQ_BIT_ID	317

/*******************************************************************************
 * syscirq definitions
 ******************************************************************************/
#define SYS_CIRQ_BASE		(IO_PHYS + 0x204000)
#define CIRQ_REG_NUM		22
#define CIRQ_SPI_START		128
#define CIRQ_IRQ_NUM		681

/*******************************************************************************
 * Stack and buffer size for TEE OS
 ******************************************************************************/
#define PLATFORM_STACK_SIZE    0x800
#if defined(SPD_NONE)
#define BUFFER_FOR_TEE	U(0x8000)
#else
#define BUFFER_FOR_TEE	U(0x0)
#endif

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE   (1ULL << 39)
#define PLAT_VIRT_ADDR_SPACE_SIZE  (1ULL << 39)
#define MAX_XLAT_TABLES     128
#define MAX_MMAP_REGIONS    512

/* Platform cacheline size */
#define PLATFORM_CACHE_LINE_SIZE CACHE_WRITEBACK_GRANULE

/*******************************************************************************
 * SPMI address
 ******************************************************************************/
#define PMIF_SPMI_BASE      (IO_PHYS + 0x0C015000)
#define PMIF_SPMI_SIZE      0x1000

/*******************************************************************************
 * CCU address
 ******************************************************************************/
#define CCU_BASE       (IO_PHYS + 0x0B000000)
#define CCU_REG_BASE   (CCU_BASE + 0x80000)
#define CCU_REG_SIZE   (0x1000)
#define CCU_EXTREG_BASE   (CCU_BASE + 0x88000)
#define CCU_EXTREG_SIZE   (0x1000)

/*******************************************************************************
 * SECURE_AO related constants
 ******************************************************************************/
#define SECURE_AO_BASE      (IO_PHYS + 0x0C00B000)
#define SECURE_AO_REG_SIZE  0x1000

/*******************************************************************************
 * IOMMU related constants
 ******************************************************************************/
#define DISP_IOMMU_BK0_BASE			(IO_PHYS + 0x0E802000)
#define DISP_IOMMU_BK1_BASE			(IO_PHYS + 0x0E803000)
#define DISP_IOMMU_BK2_BASE			(IO_PHYS + 0x0E804000)
#define DISP_IOMMU_BK3_BASE			(IO_PHYS + 0x0E805000)
#define DISP_IOMMU_BK4_BASE			(IO_PHYS + 0x0E806000)

#define MDP_IOMMU_BK0_BASE			(IO_PHYS + 0x0E810000)
#define MDP_IOMMU_BK1_BASE			(IO_PHYS + 0x0E811000)
#define MDP_IOMMU_BK2_BASE			(IO_PHYS + 0x0E812000)
#define MDP_IOMMU_BK3_BASE			(IO_PHYS + 0x0E813000)
#define MDP_IOMMU_BK4_BASE			(IO_PHYS + 0x0E814000)

#define APU_IOMMU0_BK0_BASE			(IO_PHYS + 0x09010000)
#define APU_IOMMU0_BK1_BASE			(IO_PHYS + 0x09011000)
#define APU_IOMMU0_BK2_BASE			(IO_PHYS + 0x09012000)
#define APU_IOMMU0_BK3_BASE			(IO_PHYS + 0x09013000)
#define APU_IOMMU0_BK4_BASE			(IO_PHYS + 0x09014000)

#define APU_IOMMU1_BK0_BASE			(IO_PHYS + 0x09015000)
#define APU_IOMMU1_BK1_BASE			(IO_PHYS + 0x09016000)
#define APU_IOMMU1_BK2_BASE			(IO_PHYS + 0x09017000)
#define APU_IOMMU1_BK3_BASE			(IO_PHYS + 0x09018000)
#define APU_IOMMU1_BK4_BASE			(IO_PHYS + 0x09019000)

#define PERI_IOMMU_M4_BK0_BASE			(IO_PHYS + 0x00330000)
#define PERI_IOMMU_M4_BK1_BASE			(IO_PHYS + 0x00331000)
#define PERI_IOMMU_M4_BK2_BASE			(IO_PHYS + 0x00332000)
#define PERI_IOMMU_M4_BK3_BASE			(IO_PHYS + 0x00333000)
#define PERI_IOMMU_M4_BK4_BASE			(IO_PHYS + 0x00334000)

#define PERI_IOMMU_M6_BK0_BASE			(IO_PHYS + 0x00335000)
#define PERI_IOMMU_M6_BK1_BASE			(IO_PHYS + 0x00336000)
#define PERI_IOMMU_M6_BK2_BASE			(IO_PHYS + 0x00337000)
#define PERI_IOMMU_M6_BK3_BASE			(IO_PHYS + 0x00338000)
#define PERI_IOMMU_M6_BK4_BASE			(IO_PHYS + 0x00339000)

#define PERI_IOMMU_M7_BK0_BASE			(IO_PHYS + 0x0033A000)
#define PERI_IOMMU_M7_BK1_BASE			(IO_PHYS + 0x0033B000)
#define PERI_IOMMU_M7_BK2_BASE			(IO_PHYS + 0x0033C000)
#define PERI_IOMMU_M7_BK3_BASE			(IO_PHYS + 0x0033D000)
#define PERI_IOMMU_M7_BK4_BASE			(IO_PHYS + 0x0033E000)

#define IOMMU_BANK_SIZE				(0x5000)

#define INFRA_SECURITY_AO_BASE			(IO_PHYS + 0x0001C000)

/*******************************************************************************
 * UFS related
 ******************************************************************************/
#define UFS_AO_CONFIG_BASE      (IO_PHYS + 0x012B8000)
#define UFSHCI_BASE             (IO_PHYS + 0x012B0000)

/*******************************************************************************
 * CPU AMU related
 ******************************************************************************/
#define PLAT_AMU_GROUP1_COUNTERS_MASK U(0x7)

/*******************************************************************************
 * Booker related
 ******************************************************************************/
#define BOOKER_BASE	(0x0a000000)
#define HNI_CFG_CTL	(BOOKER_BASE + 0x550a00)

/*******************************************************************************
 * EMI related
 ******************************************************************************/
#define CHN0_EMI_APB_BASE		(IO_PHYS + 0x00235000)
#define CHN1_EMI_APB_BASE		(IO_PHYS + 0x00255000)
#define CHN2_EMI_APB_BASE		(IO_PHYS + 0x00245000)
#define CHN3_EMI_APB_BASE		(IO_PHYS + 0x00265000)
#define EMI_APB_BASE			(IO_PHYS + 0x00219000)
#define INFRA_EMI_DEBUG_CFG_BASE	(IO_PHYS + 0x0021C000)
#define NEMI_SMPU_BASE			(IO_PHYS + 0x00351000)
#define SEMI_SMPU_BASE			(IO_PHYS + 0x00355000)
#define SUB_EMI_APB_BASE		(IO_PHYS + 0x0021D000)
#define SUB_INFRA_EMI_DEBUG_CFG_BASE	(IO_PHYS + 0x0021E000)
#define INFRACFG_AO_MEM_BASE		(IO_PHYS + 0x00270000)
#define SUB_INFRACFG_AO_MEM_BASE	(IO_PHYS + 0x0030E000)

/*******************************************************************************
 * VCP related constants
 ******************************************************************************/
#define MTK_VCP_SRAM_BASE           (IO_PHYS + 0x0EA00000)
#define MTK_VCP_SRAM_SIZE           (0x40000)
#define MTK_VCP_REG_BASE            (IO_PHYS + 0x0EC00000)
#define MTK_VCP_REG_BANK_SIZE       (0x1000)

/*******************************************************************************
 * ADSP related
 ******************************************************************************/
#define ADSP_CFG_BASE               (IO_PHYS + 0x0E000000)
#define ADSP_CFG_SIZE               (0x10000)
#define ADSP0_DTCM_BASE             (IO_PHYS + 0x0E020000)
#define ADSP1_DTCM_BASE             (IO_PHYS + 0x0E090000)
#define ADSP_DTCM_SIZE              (0x8000)


/*******************************************************************************
 * AUDIO related constants
 ******************************************************************************/
#define CLK_CFG_8	(TOPCKGEN_BASE + 0x90)

#define AUDIO_BASE (0x1e100000)
#define AFE_SE_DOMAIN_SIDEBAND0  (AUDIO_BASE + 0x0d3c)
#define AFE_SE_DOMAIN_SIDEBAND1  (AUDIO_BASE + 0x0d54)
#define AFE_SE_DOMAIN_SIDEBAND2  (AUDIO_BASE + 0x0d58)
#define AFE_SE_DOMAIN_SIDEBAND3  (AUDIO_BASE + 0x0d5c)

/*******************************************************************************
 * BT related constants
 ******************************************************************************/
#define BT_SRAM_BANK2_BASE  (0x18840000)
#define BT_SRAM_BANK2_SIZE  (0x20000)
#define BT_RG_BASE          (0x18830000)
#define BT_RG_SIZE          (0x2000)
#define BT_RG_CTRL          (BT_RG_BASE + 0x148)

/*******************************************************************************
 * BCRM_INFRA_PDN_APB
 ******************************************************************************/
#define BCRM_INFRA_PDN_APB_BASE (IO_PHYS + 0x215000)
#define BCRM_INFRA_PDN_APB_SIZE (0x1000)

/*******************************************************************************
 * IMGSYS related constants
 ******************************************************************************/
#define IMGSYS_BASE (0x15000000)
#define IMGSYS_ADL_A_REG_BASE  (IMGSYS_BASE + 0x05000)

#ifdef SPM_BASE
#define EXT_INT_WAKEUP_REQ	(SPM_BASE + 0x210)
#define EXT_INT_WAKEUP_REQ_SET	(SPM_BASE + 0x214)
#define EXT_INT_WAKEUP_REQ_CLR	(SPM_BASE + 0x218)
#define CPU_BUCK_ISO_CON	(SPM_BASE + 0xF28)
#define CPU_BUCK_ISO_DEFAUT	(0x0)
#endif

/*******************************************************************************
 * GPS / Connsys related
 ******************************************************************************/
#define CONN_REG_GPIO_BASE	(0x10005000)
#define CONN_REG_GPIO_SIZE	(0x00001000)

#define CONN_INFRA_RGU_ON_DECLARE_BASE	(0x18000000)
#define CONN_INFRA_RGU_ON_DECLARE_SIZE	(0x00001000)

#define CONN_INFRA_CFG_ON_DECLARE_BASE	(0x18001000)
#define CONN_INFRA_CFG_ON_DECLARE_SIZE	(0x00001000)

#define CONN_WT_SLP_CTL_REG_DECLARE_BASE	(0x18003000)
#define CONN_WT_SLP_CTL_REG_DECLARE_SIZE	(0x00001000)

#define CONN_GPT2_CTRL_BASE	(0x18007000)
#define CONN_GPT2_CTRL_SIZE	(0x1000)

#define CONN_INFRA_BUS_CR_ON_BASE	(0x1800e000)
#define CONN_INFRA_BUS_CR_ON_SIZE	(0x00001000)

#define CONN_INFRA_CFG_DECLARE_BASE	(0x18011000)
#define CONN_INFRA_CFG_DECLARE_SIZE	(0x00001000)

#define CONN_INFRA_CLKGEN_TOP_DECLARE_BASE	(0x18012000)
#define CONN_INFRA_CLKGEN_TOP_DECLARE_SIZE	(0x00001000)

#define CONN_VON_BUS_BCRM_BASE	(0x18020000)
#define CONN_VON_BUS_BCRM_SIZE	(0x00001000)

#define CONN_INFRA_DBG_CTL_BASE	(0x18023000)
#define CONN_INFRA_DBG_CTL_SIZE	(0x00001000)

#define CONN_INFRA_ON_BUS_BCRM_BASE	(0x1803b000)
#define CONN_INFRA_ON_BUS_BCRM_SIZE	(0x00001000)

#define CONN_REG_CCIF_WF2AP_SWIRQ_BASE	(0x1803c000)
#define CONN_REG_CCIF_WF2AP_SWIRQ_SIZE	(0x1000)

#define CONN_REG_CCIF_BGF2AP_SWIRQ_BASE	(0x1803e000)
#define CONN_REG_CCIF_BGF2AP_SWIRQ_SIZE	(0x1000)

#define CONN_REG_CCIF_BGF2AP_BTSWIRQ_BASE	(0x1803f000)
#define CONN_REG_CCIF_BGF2AP_BTSWIRQ_SIZE	(0x1000)

#define CONN_THERM_CTL_BASE	(0x18040000)
#define CONN_THERM_CTL_SIZE	(0x00001000)

#define CONN_AFE_CTL_BASE	(0x18041000)
#define CONN_AFE_CTL_SIZE	(0x00001000)

#define CONN_RF_SPI_MST_REG_BASE	(0x18042000)
#define CONN_RF_SPI_MST_REG_SIZE	(0x00001000)

#define CONN_INFRA_BUS_CR_DECLARE_BASE	(0x1804b000)
#define CONN_INFRA_BUS_CR_DECLARE_SIZE	(0x00001000)

#define DEBUG_GEN_REG_BASE	(0x1804c000)
#define DEBUG_GEN_REG_SIZE	(0x1000)

#define CONN_INFRA_OFF_DEBUG_CTRL_AO_BASE	(0x1804d000)
#define CONN_INFRA_OFF_DEBUG_CTRL_AO_SIZE	(0x00001000)

#define CONN_INFRA_OFF_BUS_BCRM_BASE	(0x1804f000)
#define CONN_INFRA_OFF_BUS_BCRM_SIZE	(0x00001000)

#define CONN_INFRA_SYSRAM_SW_CR_BASE	(0x18050000)
#define CONN_INFRA_SYSRAM_SW_CR_SIZE	(0x00005000)

#define CONN_HOST_CSR_TOP_DECLARE_BASE	(0x18060000)
#define CONN_HOST_CSR_TOP_DECLARE_SIZE	(0x00001000)

#define CONN_SEMAPHORE_MAP_BASE	(0x18070000)
#define CONN_SEMAPHORE_MAP_SIZE	(0x00009000)

#define WIFI_REMAP_START_ADDR_BASE	(0x18400000)
#define WIFI_REMAP_START_ADDR_SIZE	(0x1000)

#define WIFI_REMAP_PHY_ADDR_BASE	(0x18500000)
#define WIFI_REMAP_PHY_ADDR_SIZE	(0x1000)

#define BGF_REG_ADDR_BASE	(0x18800000)
#define BGF_REG_ADDR_SIZE       (0x1000)

#define BGF_REG_INFO_ADDR_BASE	(0x18812000)
#define BGF_REG_INFO_ADDR_SIZE	(0x1000)

#define BT_REMAP_START_ADDR_BASE	(0x18816000)
#define BT_REMAP_START_ADDR_SIZE	(0x1000)

#define BGF_MCUSYS_DLY_CHAIN_CTL_ADDR_BASE	(0x18820000)
#define BGF_MCUSYS_DLY_CHAIN_CTL_ADDR_SIZE	(0x1000)

#define BT_REMAP_PHY_ADDR_BASE	(0x18900000)
#define BT_REMAP_PHY_ADDR_SIZE	(0x1000)

#define RCCENTRAL_REG_BASE	(0x1c00d000)
#define RCCENTRAL_REG_SIZE	(0x1000)

#endif /* PLATFORM_DEF_H */

