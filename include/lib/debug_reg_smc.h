/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_REG_SMC_H
#define DEBUG_REG_SMC_H

#define DEBUG_REG_SMC_OP_INFO		U(0x0)
#define DEBUG_REG_SMC_OP_READ		U(0x1)
#define DEBUG_REG_SMC_OP_WRITE		U(0x2)

/* Debug register-smc version returned through SMC interface */
#define DEBUG_REG_SMC_MAJOR_VERSION		(0x02U)
#define DEBUG_REG_SMC_MINOR_VERSION		(0x00U)


union debug_reg_smc_info {
	struct {
		uint8_t major_version;
		uint8_t minor_version;
		uint16_t num_registers;
	};
	uint32_t raw;
};

/* Function ID for accessing the register-smc interface
 * Vendor-Specific EL3 Range.
 */
#define DEBUG_REG_FID_VALUE		(0x30U)
#define DEBUG_REG_SMC_OP_MASK		(0xF)

#define is_debug_reg_smc_fid(_fid) \
	((GET_SMC_NUM(_fid) & ~DEBUG_REG_SMC_OP_MASK) == DEBUG_REG_FID_VALUE)

uintptr_t debug_reg_smc_handler(unsigned int smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags);

#endif /* DEBUG_REG_SMC_H */