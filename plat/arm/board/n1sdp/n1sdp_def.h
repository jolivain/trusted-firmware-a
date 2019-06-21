/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_DEF_H
#define N1SDP_DEF_H

/* BL33 source and destination base addresses */
#define N1SDP_BL33_SRC_BASE			0x14200000
#define N1SDP_BL33_DST_BASE			0xE0000000
#define N1SDP_BL33_SIZE				0x00200000

/*
 * MMU map for non secure SRAM which contains
 * platform information structure
 */
#define N1SDP_NS_RAM_BASE			(0x06000000)
#define N1SDP_NS_RAM_SIZE			(0x00010000)
#define N1SDP_MAP_NS_RAM			MAP_REGION_FLAT(	\
						N1SDP_NS_RAM_BASE,	\
						N1SDP_NS_RAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* DMC ERR0CTLR0 registers */
#define N1SDP_DMC0_ERR0CTLR0_REG		0x4E000708
#define N1SDP_DMC1_ERR0CTLR0_REG		0x4E100708

/* DMC ECC enable bit in ERR0CTLR0 register */
#define N1SDP_DMC_ERR0CTLR0_ECC_EN		0x1

/* Platform information structure base address */
#define N1SDP_INFO_STRUCT_BASE			(N1SDP_NS_RAM_BASE + 0x8000)

#endif /* N1SDP_DEF_H */
