/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCUCFG_V1_H
#define MCUCFG_V1_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif /*__ASSEMBLER__*/

#include <platform_def.h>

#define MP2_MISC_CONFIG_BOOT_ADDR_L(cpu)	(MCUCFG_BASE + 0x2290 + ((cpu) * 8))
#define MP2_MISC_CONFIG_BOOT_ADDR_H(cpu)	(MCUCFG_BASE + 0x2294 + ((cpu) * 8))

#define MP2_CPUCFG				(MCUCFG_BASE + 0x2208)

#define MP2_CPU0_STANDBYWFE			(1U << 4)
#define MP2_CPU1_STANDBYWFE			(1U << 5)

#define MP0_CPUTOP_SPMC_CTL			(MCUCFG_BASE + 0x788)
#define MP1_CPUTOP_SPMC_CTL			(MCUCFG_BASE + 0x78C)
#define MP1_CPUTOP_SPMC_SRAM_CTL		(MCUCFG_BASE + 0x790)

#define sw_spark_en				(1U << 0)
#define sw_no_wait_for_q_channel		(1U << 1)
#define sw_fsm_override				(1U << 2)
#define sw_logic_pre1_pdb			(1U << 3)
#define sw_logic_pre2_pdb			(1U << 4)
#define sw_logic_pdb				(1U << 5)
#define sw_iso					(1U << 6)
#define sw_sram_sleepb				(0x3FU << 7)
#define sw_sram_isointb				(1U << 13)
#define sw_clk_dis				(1U << 14)
#define sw_ckiso				(1U << 15)
#define sw_pd					(0x3FU << 16)
#define sw_hot_plug_reset			(1U << 22)
#define sw_pwr_on_override_en			(1U << 23)
#define sw_pwr_on				(1U << 24)
#define sw_coq_dis				(1U << 25)
#define logic_pdbo_all_off_ack			(1U << 26)
#define logic_pdbo_all_on_ack			(1U << 27)
#define logic_pre2_pdbo_all_on_ack		(1U << 28)
#define logic_pre1_pdbo_all_on_ack		(1U << 29)

#define CPUSYSx_CPUx_SPMC_CTL(cluster, cpu)	(MCUCFG_BASE + 0x1C30 + \
						 (cluster) * 0x2000 + (cpu) * 4)

#define CPUSYS0_CPU0_SPMC_CTL			(MCUCFG_BASE + 0x1C30)
#define CPUSYS0_CPU1_SPMC_CTL			(MCUCFG_BASE + 0x1C34)
#define CPUSYS0_CPU2_SPMC_CTL			(MCUCFG_BASE + 0x1C38)
#define CPUSYS0_CPU3_SPMC_CTL			(MCUCFG_BASE + 0x1C3C)

#define CPUSYS1_CPU0_SPMC_CTL			(MCUCFG_BASE + 0x3C30)
#define CPUSYS1_CPU1_SPMC_CTL			(MCUCFG_BASE + 0x3C34)
#define CPUSYS1_CPU2_SPMC_CTL			(MCUCFG_BASE + 0x3C38)
#define CPUSYS1_CPU3_SPMC_CTL			(MCUCFG_BASE + 0x3C3C)

#define cpu_sw_spark_en				(1U << 0)
#define cpu_sw_no_wait_for_q_channel		(1U << 1)
#define cpu_sw_fsm_override			(1U << 2)
#define cpu_sw_logic_pre1_pdb			(1U << 3)
#define cpu_sw_logic_pre2_pdb			(1U << 4)
#define cpu_sw_logic_pdb			(1U << 5)
#define cpu_sw_iso				(1U << 6)
#define cpu_sw_sram_sleepb			(1U << 7)
#define cpu_sw_sram_isointb			(1U << 8)
#define cpu_sw_clk_dis				(1U << 9)
#define cpu_sw_ckiso				(1U << 10)
#define cpu_sw_pd				(0x1FU << 11)
#define cpu_sw_hot_plug_reset			(1U << 16)
#define cpu_sw_powr_on_override_en		(1U << 17)
#define cpu_sw_pwr_on				(1U << 18)
#define cpu_spark2ldo_allswoff			(1U << 19)
#define cpu_pdbo_all_on_ack			(1U << 20)
#define cpu_pre2_pdbo_allon_ack			(1U << 21)
#define cpu_pre1_pdbo_allon_ack			(1U << 22)

/* CPC related registers */
#define CPC_MCUSYS_CPC_OFF_THRES		(MCUCFG_BASE + 0xA714)
#define CPC_MCUSYS_PWR_CTRL			(MCUCFG_BASE + 0xA804)
#define CPC_MCUSYS_CPC_FLOW_CTRL_CFG		(MCUCFG_BASE + 0xA814)
#define CPC_MCUSYS_LAST_CORE_REQ		(MCUCFG_BASE + 0xA818)
#define CPC_MCUSYS_MP_LAST_CORE_RESP		(MCUCFG_BASE + 0xA81C)
#define CPC_MCUSYS_LAST_CORE_RESP		(MCUCFG_BASE + 0xA824)
#define CPC_MCUSYS_PWR_ON_MASK			(MCUCFG_BASE + 0xA828)
#define CPC_SPMC_PWR_STATUS			(MCUCFG_BASE + 0xA840)
#define CPC_MCUSYS_CPU_ON_SW_HINT_SET		(MCUCFG_BASE + 0xA8A8)
#define CPC_MCUSYS_CPU_ON_SW_HINT_CLR		(MCUCFG_BASE + 0xA8AC)
#define CPC_MCUSYS_CPC_DBG_SETTING		(MCUCFG_BASE + 0xAB00)
#define CPC_MCUSYS_CPC_KERNEL_TIME_L_BASE	(MCUCFG_BASE + 0xAB04)
#define CPC_MCUSYS_CPC_KERNEL_TIME_H_BASE	(MCUCFG_BASE + 0xAB08)
#define CPC_MCUSYS_CPC_SYSTEM_TIME_L_BASE	(MCUCFG_BASE + 0xAB0C)
#define CPC_MCUSYS_CPC_SYSTEM_TIME_H_BASE	(MCUCFG_BASE + 0xAB10)
#define CPC_MCUSYS_TRACE_SEL			(MCUCFG_BASE + 0xAB14)
#define CPC_MCUSYS_TRACE_DATA			(MCUCFG_BASE + 0xAB20)
#define CPC_MCUSYS_CLUSTER_COUNTER		(MCUCFG_BASE + 0xAB70)
#define CPC_MCUSYS_CLUSTER_COUNTER_CLR		(MCUCFG_BASE + 0xAB74)

/* CPC_MCUSYS_CPC_FLOW_CTRL_CFG bit control */
#define CPC_CTRL_ENABLE				(1U << 16)
#define SSPM_CORE_PWR_ON_EN			(1U << 7) /* for cpu-hotplug */
#define SSPM_ALL_PWR_CTRL_EN			(1U << 13) /* for cpu-hotplug */
#define GIC_WAKEUP_IGNORE(cpu)			(1U << (21 + cpu))

#define CPC_MCUSYS_CPC_RESET_ON_KEEP_ON		(1U << 17)
#define CPC_MCUSYS_CPC_RESET_PWR_ON_EN		(1U << 20)

/* SPMC related registers */
#define SPM_MCUSYS_PWR_CON			(MCUCFG_BASE + 0xD200)
#define SPM_MP0_CPUTOP_PWR_CON			(MCUCFG_BASE + 0xD204)
#define SPM_MP0_CPU0_PWR_CON			(MCUCFG_BASE + 0xD208)
#define SPM_MP0_CPU1_PWR_CON			(MCUCFG_BASE + 0xD20C)
#define SPM_MP0_CPU2_PWR_CON			(MCUCFG_BASE + 0xD210)
#define SPM_MP0_CPU3_PWR_CON			(MCUCFG_BASE + 0xD214)
#define SPM_MP0_CPU4_PWR_CON			(MCUCFG_BASE + 0xD218)
#define SPM_MP0_CPU5_PWR_CON			(MCUCFG_BASE + 0xD21C)
#define SPM_MP0_CPU6_PWR_CON			(MCUCFG_BASE + 0xD220)
#define SPM_MP0_CPU7_PWR_CON			(MCUCFG_BASE + 0xD224)

/* bit fields of SPM_*_PWR_CON */
#define PWR_ON_ACK				(1U << 31)
#define VPROC_EXT_OFF				(1U << 7)
#define DORMANT_EN				(1U << 6)
#define RESETPWRON_CONFIG			(1U << 5)
#define PWR_CLK_DIS				(1U << 4)
#define PWR_ON					(1U << 2)
#define PWR_RST_B				(1U << 0)

#define SPARK2LDO				(MCUCFG_BASE + 0x2700)
/* APB Module mcucfg */
#define MP0_CA7_CACHE_CONFIG			(MCUCFG_BASE + 0x000)
#define MP0_AXI_CONFIG				(MCUCFG_BASE + 0x02C)
#define MP0_MISC_CONFIG0			(MCUCFG_BASE + 0x030)
#define MP0_MISC_CONFIG1			(MCUCFG_BASE + 0x034)
#define MP0_MISC_CONFIG2			(MCUCFG_BASE + 0x038)
#define MP0_MISC_CONFIG_BOOT_ADDR(cpu)		(MCUCFG_BASE + 0x038 + ((cpu) * 8))
#define MP0_MISC_CONFIG3			(MCUCFG_BASE + 0x03C)
#define MP0_MISC_CONFIG9			(MCUCFG_BASE + 0x054)
#define MP0_CA7_MISC_CONFIG			(MCUCFG_BASE + 0x064)

#define MP0_RW_RSVD0				(MCUCFG_BASE + 0x06C)
#define MP1_CA7_CACHE_CONFIG			(MCUCFG_BASE + 0x200)
#define MP1_AXI_CONFIG				(MCUCFG_BASE + 0x22C)
#define MP1_MISC_CONFIG0			(MCUCFG_BASE + 0x230)
#define MP1_MISC_CONFIG1			(MCUCFG_BASE + 0x234)
#define MP1_MISC_CONFIG2			(MCUCFG_BASE + 0x238)
#define MP1_MISC_CONFIG_BOOT_ADDR(cpu)		(MCUCFG_BASE + 0x238 + ((cpu) * 8))
#define MP1_MISC_CONFIG3			(MCUCFG_BASE + 0x23C)
#define MP1_MISC_CONFIG9			(MCUCFG_BASE + 0x254)
#define MP1_CA7_MISC_CONFIG			(MCUCFG_BASE + 0x264)

#define CCI_ADB400_DCM_CONFIG			(MCUCFG_BASE + 0x740)
#define SYNC_DCM_CONFIG				(MCUCFG_BASE + 0x744)

#define MP0_CLUSTER_CFG0			(MCUCFG_BASE + 0xC8D0)

#define MP0_SPMC				(MCUCFG_BASE + 0x788)
#define MP1_SPMC				(MCUCFG_BASE + 0x78C)
#define MP2_AXI_CONFIG				(MCUCFG_BASE + 0x220C)
#define MP2_AXI_CONFIG_ACINACTM			(1U << 0)
#define MP2_AXI_CONFIG_AINACTS			(1U << 4)

#define MPx_AXI_CONFIG_ACINACTM			(1U << 4)
#define MPx_AXI_CONFIG_AINACTS			(1U << 5)

#define MPx_CA7_MISC_CONFIG_standbywfil2	(1U << 28)

#define MP0_CPU0_STANDBYWFE			(1U << 20)
#define MP0_CPU1_STANDBYWFE			(1U << 21)
#define MP0_CPU2_STANDBYWFE			(1U << 22)
#define MP0_CPU3_STANDBYWFE			(1U << 23)

#define MP1_CPU0_STANDBYWFE			(1U << 20)
#define MP1_CPU1_STANDBYWFE			(1U << 21)
#define MP1_CPU2_STANDBYWFE			(1U << 22)
#define MP1_CPU3_STANDBYWFE			(1U << 23)

#define CPUSYS0_SPARKVRETCNTRL			(MCUCFG_BASE+0x1c00)
#define CPUSYS0_SPARKEN				(MCUCFG_BASE+0x1c04)
#define CPUSYS0_AMUXSEL				(MCUCFG_BASE+0x1c08)
#define CPUSYS1_SPARKVRETCNTRL			(MCUCFG_BASE+0x3c00)
#define CPUSYS1_SPARKEN				(MCUCFG_BASE+0x3c04)
#define CPUSYS1_AMUXSEL				(MCUCFG_BASE+0x3c08)

#define MP2_PWR_RST_CTL				(MCUCFG_BASE + 0x2008)
#define MP2_PTP3_CPUTOP_SPMC0			(MCUCFG_BASE + 0x22A0)
#define MP2_PTP3_CPUTOP_SPMC1			(MCUCFG_BASE + 0x22A4)

#define MP2_COQ					(MCUCFG_BASE + 0x22BC)
#define MP2_COQ_SW_DIS				(1U << 0)

#define MP2_CA15M_MON_SEL			(MCUCFG_BASE + 0x2400)
#define MP2_CA15M_MON_L				(MCUCFG_BASE + 0x2404)

#define CPUSYS2_CPU0_SPMC_CTL			(MCUCFG_BASE + 0x2430)
#define CPUSYS2_CPU1_SPMC_CTL			(MCUCFG_BASE + 0x2438)
#define CPUSYS2_CPU0_SPMC_STA			(MCUCFG_BASE + 0x2434)
#define CPUSYS2_CPU1_SPMC_STA			(MCUCFG_BASE + 0x243C)

#define MP0_CA7L_DBG_PWR_CTRL			(MCUCFG_BASE + 0x068)
#define MP1_CA7L_DBG_PWR_CTRL			(MCUCFG_BASE + 0x268)
#define BIG_DBG_PWR_CTRL			(MCUCFG_BASE + 0x75C)

#define MP2_SW_RST_B				(1U << 0)
#define MP2_TOPAON_APB_MASK			(1U << 1)
#define B_SW_HOT_PLUG_RESET			(1U << 30)
#define B_SW_PD_OFFSET				(18)
#define B_SW_PD					(0x3F << B_SW_PD_OFFSET)

#define B_SW_SRAM_SLEEPB_OFFSET			(12)
#define B_SW_SRAM_SLEEPB			(0x3F << B_SW_SRAM_SLEEPB_OFFSET)

#define B_SW_SRAM_ISOINTB			(1U << 9)
#define B_SW_ISO				(1U << 8)
#define B_SW_LOGIC_PDB				(1U << 7)
#define B_SW_LOGIC_PRE2_PDB			(1U << 6)
#define B_SW_LOGIC_PRE1_PDB			(1U << 5)
#define B_SW_FSM_OVERRIDE			(1U << 4)
#define B_SW_PWR_ON				(1U << 3)
#define B_SW_PWR_ON_OVERRIDE_EN			(1U << 2)

#define B_FSM_STATE_OUT_OFFSET			(6)
#define B_FSM_STATE_OUT_MASK			(0x1F << B_FSM_STATE_OUT_OFFSET)
#define B_SW_LOGIC_PDBO_ALL_OFF_ACK		(1U << 5)
#define B_SW_LOGIC_PDBO_ALL_ON_ACK		(1U << 4)
#define B_SW_LOGIC_PRE2_PDBO_ALL_ON_ACK		(1U << 3)
#define B_SW_LOGIC_PRE1_PDBO_ALL_ON_ACK		(1U << 2)


#define B_FSM_OFF				(0U << B_FSM_STATE_OUT_OFFSET)
#define B_FSM_ON				(1U << B_FSM_STATE_OUT_OFFSET)
#define B_FSM_RET				(2U << B_FSM_STATE_OUT_OFFSET)

#ifndef __ASSEMBLER__
/* cpu boot mode */
enum mp0_coucfg_64bit_ctrl {
	MP0_CPUCFG_64BIT_SHIFT = 12,
	MP1_CPUCFG_64BIT_SHIFT = 28,
	MP0_CPUCFG_64BIT = 0xfu << MP0_CPUCFG_64BIT_SHIFT,
	MP1_CPUCFG_64BIT = 0xfu << MP1_CPUCFG_64BIT_SHIFT,
};

enum mp1_dis_rgu0_ctrl {
	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT = 0,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT = 4,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT = 8,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT = 12,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT = 16,
	MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK = 0xF << MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK = 0xF << MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK = 0xF << MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK = 0xF << MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK_SHIFT,
	MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK = 0xF << MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK_SHIFT,
};

enum mp1_ainacts_ctrl {
	MP1_AINACTS_SHIFT = 4,
	MP1_AINACTS = 1U << MP1_AINACTS_SHIFT,
};

enum mp1_sw_cg_gen {
	MP1_SW_CG_GEN_SHIFT = 12,
	MP1_SW_CG_GEN = 1U << MP1_SW_CG_GEN_SHIFT,
};

enum mp1_l2rstdisable {
	MP1_L2RSTDISABLE_SHIFT = 14,
	MP1_L2RSTDISABLE = 1U << MP1_L2RSTDISABLE_SHIFT,
};
#endif /*__ASSEMBLER__*/

#endif  /* MCUCFG_V1_H */
