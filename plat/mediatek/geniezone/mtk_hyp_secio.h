/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTK_HYP_SECIO_H
#define MTK_HYP_SECIO_H

#if CONFIG_MTK_GZ_SECIO

#define SECIO_REG(_type, _base_addr, _base_size) \
	[_type].io_type = _type, \
	[_type].base_addr = _base_addr, \
	[_type].base_size = _base_size \

enum secio_type {
	SECIO_INVALID = 0,
	SECIO_GCE = 1,
	SECIO_ISP_CAM_A = 2,
	SECIO_ISP_CAM_B = 3,
	SECIO_DIP_TOP = 4,
	SECIO_DAPC_AO_MM0 = 5,
	SECIO_DAPC_AO_MM1 = 6,
	SECIO_DIP_DMA = 7,
	SECIO_SENINF_MUX1 = 8,
	SECIO_SENINF_MUX2 = 9,
	SECIO_SENINF_MUX3 = 10,
	SECIO_SENINF_MUX4 = 11,
	SECIO_SENINF_MUX5 = 12,
	SECIO_SENINF_MUX6 = 13,
	SECIO_SENINF_MUX7 = 14,
	SECIO_SENINF_MUX8 = 15,
	SECIO_ISP_CAM_C = 16,
	SECIO_FD_TOP = 17,
	SECIO_GCE_M = 18,
	SECIO_ISP_CAM_A2 = 19,
	SECIO_ISP_CAM_B2 = 20,
	SECIO_ISP_CAM_C2 = 21,
	SECIO_ISP_CAM_SYS = 22,
	SECIO_M4U_CAM_SMI_LARB0 = 23,
	SECIO_M4U_CAM_SMI_LARB1 = 24,
	SECIO_M4U_CAM_SMI_LARB2 = 25,
	SECIO_M4U_CAM_SMI_LARB3 = 26,
	SECIO_M4U_CAM_SMI_LARB4 = 27,
	SECIO_M4U_CAM_SMI_LARB5 = 28,
	SECIO_M4U_CAM_SMI_LARB6 = 29,
	SECIO_ISP_YUV_CAM_A = 30,
	SECIO_ISP_YUV_CAM_B = 31,
	SECIO_ISP_YUV_CAM_C = 32,
	SECIO_SENINF_MUX9 = 33,
	SECIO_SENINF_MUX10 = 34,
	SECIO_SENINF_MUX11 = 35,
	SECIO_SENINF_MUX12 = 36,
	SECIO_SENINF_MUX13 = 37,
	SECIO_SENINF_MUX14 = 38,
	SECIO_SENINF_MUX15 = 39,
	SECIO_SENINF_MUX16 = 40,
	SECIO_SENINF_MUX17 = 41,
	SECIO_SENINF_MUX18 = 42,
	SECIO_SENINF_MUX19 = 43,
	SECIO_SENINF_MUX20 = 44,
	SECIO_SENINF_MUX21 = 45,
	SECIO_SENINF_MUX22 = 46,
	SECIO_ISP_INFRA_S = 47,

	SECIO_MAX = 48,
};

struct secio_register {
	enum secio_type io_type;
	uintptr_t base_addr;
	uint32_t base_size;
};

/* clang-format off */
/* clang-format on */
#endif

const struct secio_register *get_plat_secio_regs(void);

#endif /* MTK_HYP_SECIO_H */
