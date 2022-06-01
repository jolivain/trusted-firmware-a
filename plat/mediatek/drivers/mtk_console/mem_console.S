/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <assert_macros.S>
#include <console_macros.S>
#include <platform_def.h>
#include "mem_console.h"

	.globl	console_mem_putc
	.globl	console_mem_register
	.globl	console_crash_mem_register
	.globl	console_crash_mem_putc

	/* -----------------------------------------------
	 * int console_crash_mem_putc(char c, console_t *console);
	 *
	 * Function to print character to debug memory
	 * console.
	 * In: x0 - ASCII character
	 *     x1 - pointer to console_t struct
	 * Out: return char on success, -1 on error
	 * Clobber list : x0, x1, x2, x16, x17(comply with
	 * plat_crash_console_putc requirement)
	 * -----------------------------------------------
	 */
func console_crash_mem_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	/* x17 point to variable of debug buf pointer */
	mov x17, #CONSOLE_T_DRVDATA
	add x17, x1, x17
	ldr x17, [x17]
	/* x17 is debug buf header magic number address */
	ldr x2, =TFA_CRASH_MAGIC_NO
	str w2, [x17]
	/* x17 point to crash_mem_record array */
	mov x17, #DEBUG_BUF_CRASH_BUF_PER_CPU
	add x17, x1, x17
	/* plat_my_core_pos clobber x0, x1 */
	/* x2 is char */
	mov x2, x0
	mov x16, x30
	bl plat_my_core_pos
	mov x30, x16
	mov x1, #SIZEOF_DEBUG_BUF_CRASH_MEM_RECORD
	/* x1 pointer this cpu's crash base(offset) */
	mul x1, x1, x0
	add x17, x17, x1
	/* x1 is crash_buf_addr */
	ldr x1, [x17]
	cbz x1, crash_putc_error
	/* x16 is w_offset */
	ldr w16, [x17, #DEBUG_BUF_CRASH_MEM_RECORD_W_OFFSET]
	/* x0 is crash_buf_size */
	ldr w0, [x17, #DEBUG_BUF_CRASH_MEM_RECORD_SIZE]
	cmp w16, w0
	b.hs stop_print
	add x1, x1, x16
	strb w2, [x1]
	/* update w_offset */
	cinc w16, w16, lo
	str w16, [x17, #DEBUG_BUF_CRASH_MEM_RECORD_W_OFFSET]
	ret
stop_print:
	/*
	 * Do not roll over to the head of crash buffer
	 * if reach the end of crash buffer
	 */
	mov	w0, #-2
	ret
crash_putc_error:
	mov	w0, #-1
	ret
endfunc console_crash_mem_putc
	/* -----------------------------------------------
	 * int console_mem_putc(char c, console_t *console);
	 *
	 * Function to print character to debug memory
	 * console.
	 * In: x0 - ASCII character
	 *     x1 - pointer to console_t struct
	 * Out: return char on success, -1 on error
	 * Clobber list : x0, x1, x2, x16, x17(comply with
	 * plat_crash_console_putc requirement)
	 * -----------------------------------------------
	 */
func console_mem_putc
#if ENABLE_ASSERTIONS
	cmp	x1, #0
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */
	/* x1 console_t pointer */
	ldr	x1, [x1, #CONSOLE_T_BASE]
	/* x16 is debug_buf_instance */
	mov x16, x1
	cbz	x1, putc_error
	/* x1 is debug_buf_hdr pointer */
	ldr x1, [x1]
	cbz x1, putc_error
	/* x2 is instance debug_buf_size */
	ldr w2, [x16, #DEBUG_BUF_INSTANCE_BUF_SIZE]
	/* x17 is instance->w_offset */
	ldr w17, [x16, #DEBUG_BUF_INSTANCE_W_OFFSET]
	cmp w17, w2
	/* x2 is debug_buf body head */
	add x2, x1, #SIZEOF_DEBUG_BUF_HDR
	b.lo not_roll_over
	mov	x17, #0
not_roll_over:
	add x2, x2, x17
	strb w0, [x2]
	/* update w_offset */
	add x17, x17, #1
	str w17, [x16, #DEBUG_BUF_INSTANCE_W_OFFSET]
	str w17, [x1, #DEBUG_BUF_HDR_W_OFFSET]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_mem_putc

	/* -----------------------------------------------
	 * int console_mem_register(uintptr_t baseaddr,
	 *     mem_console_t *console);
	 * Function to initialize and register a new memory
	 * console.
	 * In: x0 - mem base address
	 *     x1 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7
	 * -----------------------------------------------
	 */
func console_mem_register
	mov	x7, x30
	mov	x6, x1
	cbz	x6, register_fail
	cbz	x0, register_fail
	/* x0 is debug_buf_instance pointer */
	str	x0, [x6, #CONSOLE_T_BASE]
	/* Assign debug_buf_header address */
	str x0, [x6, #CONSOLE_T_DRVDATA]

register_mem:
	mov	x0, x6
	mov	x30, x7
	finish_console_register mem putc=1, flush=0
register_fail:
	ret	x7
endfunc console_mem_register

	/* -----------------------------------------------
	 * int console_crash_mem_register(uintptr_t baseaddr,
	 *     mem_console_t *console);
	 * Function to initialize and register a new memory
	 * console.
	 * In: x0 - mem base address
	 *     x1 - pointer to empty console_t struct
	 * Out: return 1 on success, 0 on error
	 * Clobber list : x0, x1, x2, x6, x7
	 * -----------------------------------------------
	 */
func console_crash_mem_register
	mov	x7, x30
	mov	x6, x1
	cbz	x6, register_crash_fail
	cbz	x0, register_crash_fail
	/* x0 is debug_buf_instance pointer */
	str	x0, [x6, #CONSOLE_T_BASE]

	mov	x0, x6
	mov	x30, x7
	finish_console_register crash_mem putc=1, flush=0
register_crash_fail:
	ret	x7
endfunc console_crash_mem_register
