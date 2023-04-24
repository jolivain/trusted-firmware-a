
#include <asm_macros.S>

	.globl	plat_handle_uncontainable_ea

str_uncontainable:
	.byte	10 /* Log marker */
	.asciz "Injected Uncontainable Error\n"

str_double_fault:
	.byte	10 /* Log marker */
	.asciz "Injected Double Fault\n"

	/* -----------------------------------------------------
	 * Platform handler for Uncontainable External Abort.
	 *
	 * x0: EA reason
	 * x1: EA syndrome
	 * -----------------------------------------------------
	 */
func plat_handle_uncontainable_ea
	adr	x0, str_uncontainable
	bl	tf_log
1:
	wfe
	b	1b
endfunc plat_handle_uncontainable_ea

	/* -----------------------------------------------------
	 * Platform handler for Double Fault.
	 *
	 * x0: EA reason
	 * x1: EA syndrome
	 * -----------------------------------------------------
	 */
func plat_handle_double_fault
	adr	x0, str_double_fault
	bl	tf_log
1:
	wfe
	b	1b
endfunc plat_handle_double_fault
