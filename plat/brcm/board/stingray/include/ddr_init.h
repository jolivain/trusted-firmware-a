/*
 * Copyright (c) 2016 - 2019, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_INIT_H
#define DDR_INIT_H

#include <fru.h>

#pragma weak ddr_initialize
#pragma weak ddr_secure_region_config
#pragma weak ddr_info_save
#pragma weak get_active_ddr_channel
#pragma weak is_warmboot

void ddr_initialize(struct ddr_info *ddr)
{
	return;
}

void ddr_secure_region_config(uint64_t start, uint64_t end)
{
	return;
}

void ddr_info_save(void)
{
	return;
}

unsigned char get_active_ddr_channel(void)
{
	return 0;
}

static inline unsigned int is_warmboot(void)
{
	return 0;
}

#if 0
#include <fru.h>
#include <lib/utils_def.h>
#include <scp_utils.h>
#include <sr_def.h>
#include "m0_cfg.h"

/* Channel to Channel register offset */
#define CHANNEL_OFFSET 0x80000
#define MEMC_BASE 0x66200000

#define INTERRUPT_CLR_OFFSET 0x00000508
#define CDRU_DDR_CTRL_OFFSET(ch) (CDRU_DDR0_CONTROL_OFFSET + 4 * (ch))
#define CDRU_DDR_CTRL_RESET_N_SHIFT 16

#define EMEM_GENPLL_CONTROL4 0x00000024
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_INT_SHIFT 20
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_Q_SHIFT 10
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_P_SHIFT 0
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_INT_MASK \
	(0x3ff << EMEM_GENPLL_CONTROL4_GENPLL_NDIV_INT_SHIFT)
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_Q_MASK \
	(0x3ff << EMEM_GENPLL_CONTROL4_GENPLL_NDIV_Q_SHIFT)
#define EMEM_GENPLL_CONTROL4_GENPLL_NDIV_P_MASK \
	(0x3ff << EMEM_GENPLL_CONTROL4_GENPLL_NDIV_P_SHIFT)

#define EMEM_GENPLL_CONTROL6 0x0000002c
#define EMEM_GENPLL_CONTROL6_GENPLL_CH0_MDIV_SHIFT 0
#define EMEM_GENPLL_CONTROL6_GENPLL_CH0_MDIV_MASK \
	(0x1ff << EMEM_GENPLL_CONTROL6_GENPLL_CH0_MDIV_SHIFT)

#define EMEM_GENPLL_CONTROL0 0x00000014
#define EMEM_GENPLL_CONTROL0_GENPLL_RESETB_SHIFT 12
#define EMEM_GENPLL_CONTROL0_GENPLL_POST_RESETB_SHIFT 11
#define EMEM_GENPLL_CONTROL0_GENPLL_KI_SHIFT 4
#define EMEM_GENPLL_CONTROL0_GENPLL_KP_SHIFT 0

#define EMEM_GENPLL_STATUS 0x00000044
#define EMEM_GENPLL_STATUS_GENPLL_LOCK_SHIFT 12

#define OLY_HNF_REGISTERS_NODE3_HNF_SAM_CONTROL 0x61200008
#define OLY_HNF_REGISTERS_NODE5_HNF_SAM_CONTROL 0x61210008
#define OLY_HNF_REGISTERS_NODE11_HNF_SAM_CONTROL 0x61220008
#define OLY_HNF_REGISTERS_NODE13_HNF_SAM_CONTROL 0x61230008
#define SNF_PORT_DMC0 0x2
#define SNF_PORT_DMC1 0x4
#define SNF_PORT_DMC2 0xa

#define DPFE_CONTROLLER_SEQ_TIMING_3 0x00010044
#define DPFE_CONTROLLER_DFI_CONTROL 0x00010058

#define EMEM_MEM_PWR_CFG_REG 0x00000008
#define EMEM_MEM_PWR_STAT_REG 0x0000000c

#define DDR34_PHY_CONTROL_REGS_ALERT_CLEAR 0x00000160
#define DDR34_PHY_CONTROL_REGS_SLIP_BUFFER_CONFIG 0x000001f4
#define DDR34_PHY_CONTROL_REGS_DFI_CNTRL 0x000001a0

/* MEMC register definitions */
#define MEMC_CMD 0x00000008
#define DIRECT_ADDR 0x00000108
#define DIRECT_CMD 0x0000010c
#define MEMC_STATUS 0x0
#define MEMC_STATUS_MASK 0x7
#define MEMC_STATUS_CONFIG 0x0
#define MEMC_STATUS_LOW_POWER 0x1
#define MEMC_STATUS_PAUSED 0x2
#define MEMC_STATUS_READY 0x3
#define MEMC_STATUS_ABORTED 0x4
#define MEMC_STATUS_RECOVE  0x5
#define MEMC_STATUS__MGR_ACTIVE BIT(8)
#define MEMC_CMD__EXECUTE 0x4
#define MEMC_CMD__EXECUTE_DRAIN 0x5
#define DDR_MEMC_RETRY_COUNT 100

#define ADDRESS_CONTROL_NEXT 0x00000010
#define ADDRESS_CONTROL_NEXT__RANK_BITS_NEXT_R 24
#define ADDRESS_CONTROL_NEXT__RANK_BITS_NEXT_WIDTH 2
#define ADDRESS_CONTROL_NEXT__BANK_BITS_NEXT_R 16
#define ADDRESS_CONTROL_NEXT__BANK_BITS_NEXT_WIDTH 3
#define ADDRESS_CONTROL_NEXT__ROW_BITS_NEXT_R 8
#define ADDRESS_CONTROL_NEXT__ROW_BITS_NEXT_WIDTH 3
#define ADDRESS_CONTROL_NEXT__COLUMN_BITS_NEXT_R 0
#define ADDRESS_CONTROL_NEXT__COLUMN_BITS_NEXT_WIDTH 3

/*
 * DECODE_CONTROL_NEXT__ADDRESS_DECODE_NEXT[1:0]
 * Determines in which pattern the DRAM address
 * components are decoded from the system address.
 * 0b00 : brb
 * Rank, Bank[msb:2], Row, Bank[1:0], Column
 * 0b01 : rb
 * Row, Rank, Bank, Column
 * 0b10 : br
 * Rank, Bank, Row, Column
 */
#define ADDRESS_DECODE_NEXT 0x00000001
#define DECODE_CONTROL_NEXT 0x00000014
#define ADDRESS_MAP_NEXT 0x0000001c

#define MEMORY_ADDRESS_MAX_31_00_NEXT 0x00000078
#define MEMORY_ADDRESS_MAX_43_32_NEXT 0x0000007c
#define ACCESS_ADDRESS_MIN0_31_00_NEXT 0x00000080
#define ACCESS_ADDRESS_MIN1_31_00_NEXT 0x00000090
#define ACCESS_ADDRESS_MIN2_31_00_NEXT 0x000000a0
#define ACCESS_ADDRESS_MIN3_31_00_NEXT 0x000000b0
#define ACCESS_ADDRESS_MIN4_31_00_NEXT 0x000000c0
#define ACCESS_ADDRESS_MIN5_31_00_NEXT 0x000000d0
#define ACCESS_ADDRESS_MIN6_31_00_NEXT 0x000000e0
#define ACCESS_ADDRESS_MIN7_31_00_NEXT 0x000000f0

#define ACCESS_ADDRESS_MIN0_43_32_NEXT 0x84
#define ACCESS_ADDRESS_MAX0_31_00_NEXT 0x88
#define ACCESS_ADDRESS_MAX0_43_32_NEXT 0x8c
#define REGION_S_WRITE_EN_NEXT_BIT BIT(3)
#define REGION_S_READ_EN_NEXT_BIT BIT(2)

#define ACCESS_ADDRESS_MIN_43_32_NEXT_OFFSET 0x4
#define ACCESS_ADDRESS_MAX_31_00_NEXT_OFFSET 0x8
#define ACCESS_ADDRESS_MAX_43_32_NEXT_OFFSET 0xc
#define ACCESS_ADDRESS_SPACE_CTRL_OFFSET 0x10
#define MAX_ACCESS_ADDRESS_SPACE_WINDOWS 0x8

#define DCI_REPLAY_TYPE_NEXT 0x00000110
#define DCI_STRB 0x00000118
#define REFRESH_CONTROL_NEXT 0x00000120

#define MEMORY_TYPE_NEXT 0x00000128
#define MEMORY_TYPE_NEXT__MEMORY_BANK_GROUPS_NEXT_R 16
#define MEMORY_TYPE_NEXT__MEMORY_BANK_GROUPS_NEXT_WIDTH 2
#define MEMORY_TYPE_NEXT__MEMORY_DEVICE_WIDTH_NEXT_R 8
#define MEMORY_TYPE_NEXT__MEMORY_DEVICE_WIDTH_NEXT_WIDTH 2

#define FEATURE_CONFIG 0x00000130
#define FEATURE_CONFIG__ECC_ENABLE_R 0
#define FEATURE_CONFIG__ECC_ENABLE_WIDTH 2
#define FEATURE_CONFIG__CORRECTED_WRITEBACK 2

#define FEATURE_CONTROL_NEXT 0x000001f0
#define FEATURE_CONTROL_NEXT__TWO_T_TIMING_NEXT_R 3
#define FEATURE_CONTROL_NEXT__TWO_T_TIMING_NEXT_WIDTH 1

#define MUX_CONTROL_NEXT 0x000001f4
#define LOW_POWER_CONTROL_NEXT 0x00000020

#define RANK_REMAP_CONTROL_NEXT 0x000001f8


#define T_REFI_NEXT 0x00000200
#define T_REFI_NEXT__T_REFI_NEXT_FIELD_R 0
#define T_REFI_NEXT__T_REFI_NEXT_FIELD_WIDTH 11

#define T_RFC_NEXT 0x00000204
#define T_RFC_NEXT__T_RFC_NEXT_FIELD_R 0
#define T_RFC_NEXT__T_RFC_NEXT_FIELD_WIDTH 10

#define T_MRR_NEXT 0x00000208

#define T_MRW_NEXT 0x0000020c
#define T_MRW_NEXT__T_MRW_NEXT_FIELD_R 0
#define T_MRW_NEXT__T_MRW_NEXT_FIELD_WIDTH 7

#define T_RDPDEN_NEXT 0x00000210

#define T_RCD_NEXT 0x00000218
#define T_RCD_NEXT__T_RCD_NEXT_FIELD_R 0
#define T_RCD_NEXT__T_RCD_NEXT_FIELD_WIDTH 5

#define T_RAS_NEXT 0x0000021c
#define T_RAS_NEXT__T_RAS_NEXT_FIELD_R 0
#define T_RAS_NEXT__T_RAS_NEXT_FIELD_WIDTH 6

#define T_RP_NEXT 0x00000220
#define T_RP_NEXT__T_RP_NEXT_FIELD_R 0
#define T_RP_NEXT__T_RP_NEXT_FIELD_WIDTH 5

#define T_RPALL_NEXT 0x00000224

#define T_RRD_NEXT 0x00000228
#define T_RRD_NEXT__T_RRD_L_NEXT_R 8
#define T_RRD_NEXT__T_RRD_L_NEXT_WIDTH 4
#define T_RRD_NEXT__T_RRD_S_NEXT_R 0
#define T_RRD_NEXT__T_RRD_S_NEXT_WIDTH 4

#define T_ACT_WINDOW_NEXT 0x0000022c
#define T_ACT_WINDOW_NEXT__T_FAW_NEXT_R 0
#define T_ACT_WINDOW_NEXT__T_FAW_NEXT_WIDTH 6

#define T_RTR_NEXT 0x00000234
#define T_RTR_NEXT__T_RTR_S_NEXT_R 0
#define T_RTR_NEXT__T_RTR_S_NEXT_WIDTH 5
#define T_RTR_NEXT__T_RTR_L_NEXT_R 8
#define T_RTR_NEXT__T_RTR_L_NEXT_WIDTH 5

#define T_RTW_NEXT 0x00000238

#define T_RTP_NEXT 0x0000023c
#define T_RTP_NEXT__T_RTP_NEXT_FIELD_R 0
#define T_RTP_NEXT__T_RTP_NEXT_FIELD_WIDTH 4

#define T_WR_NEXT 0x00000244
#define T_WR_NEXT__T_WR_NEXT_FIELD_R 0
#define T_WR_NEXT__T_WR_NEXT_FIELD_WIDTH 6

#define T_WTR_NEXT 0x00000248
#define T_WTR_NEXT__T_WTR_S_NEXT_R 0
#define T_WTR_NEXT__T_WTR_S_NEXT_WIDTH 6

#define T_WTW_NEXT 0x0000024c
#define T_WTW_NEXT__T_WTW_S_NEXT_R 0
#define T_WTW_NEXT__T_WTW_S_NEXT_WIDTH 6
#define T_WTW_NEXT__T_WTW_L_NEXT_R 8
#define T_WTW_NEXT__T_WTW_L_NEXT_WIDTH 6

#define T_XMPD_NEXT 0x00000254
#define T_EP_NEXT 0x00000258

#define T_XP_NEXT 0x0000025c
#define T_XP_NEXT__T_XPDLL_NEXT_R 16
#define T_XP_NEXT__T_XPDLL_NEXT_WIDTH 8

#define T_ESR_NEXT 0x00000260

#define T_XSR_NEXT 0x00000264
#define T_XSR_NEXT__T_XSRDLL_NEXT_R 16
#define T_XSR_NEXT__T_XSRDLL_NEXT_WIDTH 11

#define T_ESRCK_NEXT 0x00000268
#define T_CKXSR_NEXT 0x0000026c
#define T_CMD_NEXT 0x00000270
#define T_PARITY_NEXT 0x00000274

#define T_ZQCS_NEXT 0x00000278
#define T_ZQCS_NEXT__T_ZQCS_NEXT_FIELD_R 0
#define T_ZQCS_NEXT__T_ZQCS_NEXT_FIELD_WIDTH 10


#define T_RDDATA_EN_NEXT 0x00000300
#define T_PHYRDLAT_NEXT 0x00000304
#define T_PHYWRLAT_NEXT 0x00000308
#define RDLVL_CONTROL_NEXT 0x00000310
#define RDLVL_MRS_NEXT 0x00000314
#define T_RDLVL_EN_NEXT 0x00000318
#define T_RDLVL_RR_NEXT 0x0000031c
#define WRLVL_CONTROL_NEXT 0x00000320
#define WRLVL_MRS_NEXT 0x00000324
#define T_WRLVL_EN_NEXT 0x00000328
#define T_WRLVL_WW_NEXT 0x0000032c
#define T_LPRESP_NEXT 0x0000034c
#define PHY_UPDATE_CONTROL_NEXT 0x00000350
#define ODT_TIMING_NEXT 0x00000358
#define ODT_WR_CONTROL_31_00_NEXT 0x00000360
#define ODT_WR_CONTROL_63_32_NEXT 0x00000364
#define ODT_RD_CONTROL_31_00_NEXT 0x00000368
#define ODT_RD_CONTROL_63_32_NEXT 0x0000036c

#define GET_MASK_DMC(Reg_field) \
	(((1<<Reg_field##_WIDTH)-1)<<Reg_field##_R)
#define CLEAR_FIELD(Reg_field, Rd_data) \
	(Rd_data&(~(((1<<Reg_field##_WIDTH)-1)<<Reg_field##_R)))
#define GET_FIELD(Reg_field, Rd_data) \
	(((((1<<Reg_field##_WIDTH)-1)<<Reg_field##_R)&Rd_data)>>Reg_field##_R)
#define SET_FIELD(reg_field, val) \
	((((1<<reg_field##_WIDTH)-1)&val)<<reg_field##_R)

#define MCB_REF_ID_OFFSET 6
#define MCB_DATA_CLOCK_OFFSET 12

#define BCM_DDR_INFO_BASE         0x8f220000
#define BCM_DDR_INFO_LEN          72
#define BCM_DDR_SIG_VAL           0x42434d44
#define BCM_DDR_RG_NR_OFFSET      (BCM_DDR_INFO_BASE + 4)
#define BCM_DDR_RG_OFFSET(region) (BCM_DDR_INFO_BASE + 8 + ((region) * 16))
#define BCM_DDR_CHKSUM_OFFSET     (BCM_DDR_INFO_BASE + BCM_DDR_INFO_LEN)
#define MODULO_256_SUM            0x100

#define MEM_RG0_OFFSET 0x80000000
#define MEM_RG0_LEN    0x80000000
#define MEM_RG1_OFFSET 0x880000000
#define MEM_RG2_OFFSET 0x9000000000
#define MEM_RG3_OFFSET 0xa000000000

#define DDR34_PHY_CONTROL_REGS_STANDBY_CONTROL 0x198
#define DDR34_PHY_CONTROL_REGS_STANDBY_CONTROL__CKE 1
#define DDR34_PHY_CONTROL_REGS_STANDBY_CONTROL__CKE1 2
#define DDR34_PHY_CONTROL_REGS_STANDBY_CONTROL__RST_N 3
#define DDR34_PHY_CONTROL_REGS_STANDBY_CONTROL__FORCE_CKE_RST_N 4
#define CRMU_DDR_PHY_AON_CTRL_1__CRMU_DDRPHY0_STANDBY_AON 0
#define CRMU_DDR_PHY_AON_CTRL_1__CRMU_DDRPHY1_STANDBY_AON 1
#define CRMU_DDR_PHY_AON_CTRL_1__CRMU_DDRPHY2_STANDBY_AON 2
#define CRMU_DDR_PHY_AON_CTRL_1__CRMU_DDRPHYx_STANDBY_AON(x) BIT(x)

#define DDR_CONTROL_ROOT(idx) (DDR0_CONTROL_ROOT + (idx) * CHANNEL_OFFSET)
#define MEMC_OFFSET(memc_idx, offset) (DDR_CONTROL_ROOT(memc_idx) + (offset))

static inline void memc_cmd(unsigned int memc_idx, unsigned int cmd)
{
	mmio_write_32(MEMC_OFFSET(memc_idx, MEMC_CMD), cmd);
}

static inline unsigned int get_memc_status(unsigned int memc_idx)
{
	return mmio_read_32(MEMC_OFFSET(memc_idx, MEMC_STATUS));
}

/*
 * Total number of memory regions in the memory layout.
 * Max number of MEMCs is 3 (MEMC0, MEMC1, MEMC2).
 * MEMC0 is comprised of two areas: MEMC0 low and MEMC0 high.
 * Thus maximum number of memory regions is 4.
 */
#define NR_MEM_REGIONS 4

struct ddr_mem_region {
	uint64_t offset;
	uint64_t len;
};

struct mem_info {
	unsigned int ddr_size_mb;
	struct ddr_mem_region region[NR_MEM_REGIONS];
};

void ddr_info_save(void);
void ddr_initialize(struct ddr_info *ddr_info);
void ddr_secure_region_config(uint64_t start, uint64_t end);
unsigned char get_active_ddr_channel(void);
#if (CLEAN_DDR && !defined(MMU_DISABLED))
/* External assembly functions */
void clean_by_core(void);
void plat_l3_dcache_flush_all(void);

/* --- C functions --- */
void ddr_clean_by_core(const int virt_core, bool stop_in_wfe);
void check_ddr_clean(void);
#endif

int direct_cmd_execute(unsigned int memc_idx);

#ifdef DDR_SCRUB_ENA
void ddr_scrub_enable(void);
#endif

static inline unsigned int is_warmboot(void)
{
#if WARMBOOT_DDR_S3_SUPPORT
	return (SCP_READ_CFG(warm_boot_signature) == WARM_BOOT_SIGNATURE);
#else
	return false;
#endif
}

int memc_cmd_execute_drain(unsigned int memc_idx);
#endif
#endif
