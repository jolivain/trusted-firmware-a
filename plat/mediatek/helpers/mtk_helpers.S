/*
 * Copyright (c) 2020, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#if CONFIG_MTK_DEBUG_BUF
#include <lib/debug_buf/debug_buf_export.h>
#endif

	/* Declare function */
	.global mtk_bl31_ramdump
	.global plat_panic_handler
	.weak	mtk_plat_handler


func mtk_plat_handler
	ret
endfunc mtk_plat_handler
	/* ---------------------------------------------
	 * mtk_bl31_ramdump will dump bl31 to debug buf
	 * whenever an unhandled exception or assert
	 * is taken in BL31.
	 * Clobbers: x0 - x7, X30
	 *		dcsw_op_all use more registers
	 *		x0, x1, x2, x3, x4, x5, x7, x8, x9,
	 *		x10, x11, x14, x16, x17
	 * ---------------------------------------------
	 */

func mtk_bl31_ramdump
#if CONFIG_MTK_BL31_RAMDUMP
#if CONFIG_MTK_DEBUG_BUF
	/*
	 * x6 = g_bl31_ramdump_record
	 * x5 = ptr_ramdump_buf_base
	 * w4 = ramdump_buf_size
	 */
	adr	x6, g_bl31_ramdump_record
	ldr	x5, [x6,#BL31_RAMDUMP_BUF_BASE_OFSSET]
	ldr	w4, [x6,#BL31_RAMDUMP_SIZE_OFFSET]

	/*
	 * if g_bl31_ramdump_record is not initialized
	 * just return
	 */
	cbz	x5, done
	cbz	w4, done
	/*
	 * x6 = g_bl31_ramdump_record
	 * x5 = ptr_ramdump_buf_base
	 * x1 = dram_start_addr
	 * if (ptr_ramdump_buf_base < dram_start_addr)
	 *		return
	 */
	ldr	x1, [x6,#BL31_RAMDUMP_DRAM_START_ADDDR_OFFSET]
	cmp	x5, x1
	b.lo	done

	/*
	 * x5 = ptr_ramdump_buf_base
	 * x1 = dram_end_addr
	 * if (ptr_ramdump_buf_base > dram_end_addr)
	 *		return
	 */
	ldr	x1, [x6,#BL31_RAMDUMP_DRAM_END_ADDDR_OFFSET]
	cmp	x5, x1
	b.hi	done

	/*
	 * x5 = ptr_ramdump_buf_base
	 * x4 = ramdump_buf_size
	 * x1 = dram_end_addr
	 * x0 = ptr_bl31_ramdump_end
	 *    = ptr_ramdump_buf_base + ramdump_buf_size
	 * if (ptr_ramdump_buf_base + ramdump_buf_size > dram_end_addr)
	 *		return
	 */
	add x0, x5, x4
	cmp	x0, x1
	b.hi	done

	/* spin_lock() */
	mov x3, x30
	adr	x0, bl31_ramdump_lock
	bl spin_lock
	mov x30, x3

	/*
	 * x5 = ptr_ramdump_buf_base;
	 * x4 = ramdump_buf_size;
	 */
	ldr	x5, [x6,#BL31_RAMDUMP_BUF_BASE_OFSSET]
	ldr	w4, [x6,#BL31_RAMDUMP_SIZE_OFFSET]

	/* See if ram dump is done by other CPUs */
	ldr w0, =BL31_RAMDUMP_END_FLAG
	ldr	w1, [x5,x4]
	cmp	w1, w0
	b.eq	ramdump_unlock

	/* *ptr_bl31_ramdump_end = BL31_RAMDUMP_START_FLAG; */
	ldr w0, =BL31_RAMDUMP_START_FLAG
	str	w0, [x5,x4]

	/*
	 * calculate copy_size
	 * x6 = g_bl31_ramdump_record;
	 * x5 = ptr_ramdump_buf_base
	 * w4 = ramdump_buf_size
	 * w0 = bl31_size
	 * w2 = copy_size
	 * if (ramdump_buf_size < bl31_size)
	 *		copy_size = min(ramdump_buf_size, bl31_size)
	 */
	ldr	x5, [x6,#BL31_RAMDUMP_BUF_BASE_OFSSET]
	ldr	w4, [x6,#BL31_RAMDUMP_SIZE_OFFSET]
	adr	x1, __BL31_START__
	ldr	x0, =__BL31_END__
	sub	w0, w0, w1
	cmp	w4, w0
	csel	w2, w4, w0, lo
	/*
	 * x5 = ptr_ramdump_buf_base
	 * x0 = dest
	 * x1 = src
	 * x2 = copy_size
	 * x3 = temp buf
	 */
	mov x0, x5
	adr	x1, __BL31_START__
	cbz w2, ramdump_copy_end
ramdump_copy_loop:
	ldrb    w3, [x1],#1
	subs    w2, w2, #0x1
	strb    w3, [x0],#1
	b.ne    ramdump_copy_loop

ramdump_copy_end:
	ldr w0, =BL31_RAMDUMP_END_FLAG
	/* *ptr_bl31_ramdump_end = BL31_RAMDUMP_START_FLAG; */
	str	w0, [x5,x4]

ramdump_unlock:
	/* spin_unlock() */
	mov x3, x30
	adr	x0, bl31_ramdump_lock
	bl spin_unlock
	mov x30, x3

done:
	/*
	 * flush and invalidate
	 * dcsw_op_all(DCCISW)
	 *
	 * dcsw_op_all use more registers
	 *		x0, x1, x2, x3, x4, x5, x7, x8, x9,
	 *		x10, x11, x14, x16, x17
	 */
	mov w0, #0x1
	b dcsw_op_all
#endif
#endif /* CONFIG_MTK_BL31_RAMDUMP */
	/*
	 * if CONFIG_MTK_DEBUG_BUF is 0
	 * empty function need ret
	 */
	ret
endfunc mtk_bl31_ramdump

func plat_panic_handler
	bl	mtk_print_timestamp
	bl	mtk_plat_handler
	bl	mtk_bl31_ramdump
#if CONFIG_MTK_AEE
	bl	mtk_set_bl31_exp_type
#endif
#if CONFIG_MTK_RGU
	bl	rgu_debug_sw_reset
#endif
infinite_loop:
	b	infinite_loop
endfunc plat_panic_handler
