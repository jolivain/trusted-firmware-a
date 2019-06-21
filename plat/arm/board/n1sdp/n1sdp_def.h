/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_DEF_H
#define N1SDP_DEF_H

/* SDS memory information defines */
#define N1SDP_SDS_MEM_INFO_STRUCT_ID		8
#define N1SDP_SDS_MEM_INFO_OFFSET		0
#define N1SDP_SDS_MEM_INFO_SIZE			4

/* SDS BL33 image information defines */
#define N1SDP_SDS_BL33_INFO_STRUCT_ID		9
#define N1SDP_SDS_BL33_INFO_OFFSET		0
#define N1SDP_SDS_BL33_INFO_SIZE		12

/* DMC ERR0CTLR0 registers */
#define N1SDP_DMC0_ERR0CTLR0_REG		0x4E000708
#define N1SDP_DMC1_ERR0CTLR0_REG		0x4E100708

/* DMC ECC enable bit in ERR0CTLR0 register */
#define N1SDP_DMC_ERR0CTLR0_ECC_EN		0x1

#endif /* N1SDP_DEF_H */
