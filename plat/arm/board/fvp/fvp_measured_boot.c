/*
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <plat/arm/common/plat_arm.h>

/*
 * This function writes the Event Log address and its size
 * in the NT_FW_CONFIG DTB.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int plat_set_nt_fw_info(void *dtb,
#ifdef SPD_opteed
			void *log_addr,
#endif
			size_t log_size, void **ns_log_addr)
{
	return arm_set_nt_fw_info(dtb,
#ifdef SPD_opteed
				  log_addr,
#endif
				  log_size, ns_log_addr);
}

/*
 * This function writes the Event Log address and its size
 * in the TOS_FW_CONFIG DTB.
 *
 * Returns:
 *	0 = success
 *    < 0 = error
 */
int plat_set_tos_fw_info(void *dtb, void *log_addr, size_t log_size)
{
	return arm_set_tos_fw_info(dtb, log_addr, log_size);
}
