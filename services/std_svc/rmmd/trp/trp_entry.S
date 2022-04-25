/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <services/rmmd_svc.h>
#include "trp_private.h"

#include <platform_def.h>

.global trp_head
.global trp_smc
.global trp_boot_abort_with_ec

.section ".head.text", "ax"

	/* ---------------------------------------------
	 * Populate the params in x0-x7 from the pointer
	 * to the smc args structure in x0.
	 * ---------------------------------------------
	 */
	.macro restore_args_call_smc
	ldp	x6, x7, [x0, #TRP_ARG6]
	ldp	x4, x5, [x0, #TRP_ARG4]
	ldp	x2, x3, [x0, #TRP_ARG2]
	ldp	x0, x1, [x0, #TRP_ARG0]
	smc	#0
	.endm

	/* ----------------------------------------------
 	 * Cancel the boot process and return to EL3 with
	 * the Error Code on _ec if condition _cc is met.
	 * ----------------------------------------------
 	 */
	.macro abort_boot_err_if _cc, _ec
	b.\_cc	1f
	b	2f
1:
	mov_imm	x0, RMMD_RMI_BOOT_COMPLETE
	mov_imm x1, \_ec
	smc	#0

	/* EL3 should never return back here, so panic if it does */
	b	trp_panic
2:
	.endm

	/* ---------------------------------------------
	 * Entry point for TRP
	 * ---------------------------------------------
	 */
trp_head:
	/*
	 * Stash arguments from previous boot stage
	 */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

	/* ---------------------------------------------
	 * Process the Boot parameters.
	 *
	 * According to the Boot Interface ABI v.0.1, the
	 * parameters recived from EL3 are:
	 * x0 (stashed in x20): CPUID
	 * x1 (stashed in x21): Boot Interface version
	 * x2 (stashed in x22): PLATFORM_CORE_COUNT (Not checked here)
	 * x3 (stashed in x23): Pointer to the shared memory area.
	 * ---------------------------------------------
	 */
	cmp	x20, #PLATFORM_CORE_COUNT
	abort_boot_err_if ge, E_RMM_BOOT_CPU_ID_OUT_OF_RANGE

	bl	plat_set_my_stack

	/*
	 * Find out whether this is a cold or warm boot
	 */
	ldr	x1, cold_boot_flag
	cbz	x1, warm_boot

	/* Verify that the Boot Interface ABI is supported */
	lsr	x0, x21, #16
	cmp	x0, #TRP_BOOT_ABI_VERS_MAJOR
	abort_boot_err_if ne, E_RMM_BOOT_VERSION_MISMATCH

	/*
	 * Check that the pointer to the shared space is not null.
	 */
	cmp	x23, #0
	abort_boot_err_if eq, E_RMM_BOOT_INVALID_SHARED_BUFFER

	/*
	 * Update cold boot flag to indicate cold boot is done
	 */
	adr	x2, cold_boot_flag
	str	xzr, [x2]

	/* ---------------------------------------------
	 * Zero out BSS section
	 * ---------------------------------------------
	 */
	ldr	x0, =__BSS_START__
	ldr	x1, =__BSS_SIZE__
	bl	zeromem

	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23
	bl	trp_setup
	bl	trp_main

warm_boot:
	mov_imm	x0, RMMD_RMI_BOOT_COMPLETE
	mov	x1, xzr /* RMM_BOOT_SUCCESS */
	smc	#0
	b	trp_handler

trp_panic:
	no_ret plat_panic_handler

	/*
	 * Flag to mark if it is a cold boot.
	 * 1: cold boot, 0: warmboot.
	 */
.align 3
cold_boot_flag:
	.dword		1

	/* ---------------------------------------------
	 *   Direct SMC call to BL31 service provided by
	 *   RMM Dispatcher
	 * ---------------------------------------------
	 */
func trp_smc
	restore_args_call_smc
	ret
endfunc trp_smc

	/* ---------------------------------------------
	 * RMI call handler
	 * ---------------------------------------------
	 */
func trp_handler
	bl	trp_rmi_handler
	restore_args_call_smc
	b	trp_handler
endfunc trp_handler

/* --------------------------------------------------------------------------
 * Immediately return to EL3 firmware with FID RMMD_RMI_BOOT_COMPLETE and the
 * error code passed on the argument.
 *
 * This function should be used by the manifest platform data handler to
 * abort if an error occurs.
 *
 * Args:	ec: Error code to use for SMC_RMM_BOOT_COMPLETE SMC call.
 * Return:	This function never returns once called.
 * --------------------------------------------------------------------------
 */
func trp_boot_abort_with_ec
	mov	x1, x0
	mov_imm x0, RMMD_RMI_BOOT_COMPLETE
	smc	#0
	/* EL3 Firmware should never return here, so panic if it does */
	b	.
endfunc trp_boot_abort_with_ec
