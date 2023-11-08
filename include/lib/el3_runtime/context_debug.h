/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if PLATFORM_REPORT_CTX_MEM_USE
/********************************************************************************
 * Reports the allocated memory for every security state and then reports the
 * total system-wide allocated memory.
 *******************************************************************************/
#ifdef __aarch64__
void report_ctx_memory_usage(void);
#else
void report_ctx_memory_usage(void)
{

}
#endif /* __aarch64__ */
#endif /* PLATFORM_REPORT_CTX_MEM_USE */
