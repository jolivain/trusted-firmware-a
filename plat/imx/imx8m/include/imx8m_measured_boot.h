/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX8M_MEASURED_BOOT_H
#define IMX8M_MEASURED_BOOT_H

#include <stdint.h>
#include <arch_helpers.h>

int imx8m_set_nt_fw_info(
/*
 * Currently OP-TEE does not support reading DTBs from Secure memory
 * and this option should be removed when feature is supported.
 */
#ifdef SPD_opteed
			uintptr_t log_addr,
#endif
			size_t log_size, uintptr_t *ns_log_addr);

#endif /* IMX8M_MEASURED_BOOT_H */
