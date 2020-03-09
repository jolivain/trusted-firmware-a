/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL_COMMON_LD_H
#define BL_COMMON_LD_H

#include <lib/xlat_tables/xlat_tables_defs.h>

/*
 * The xlat_table section is for full, aligned page tables (4K).
 * Removing them from .bss avoids forcing 4K alignment on
 * the .bss section. The tables are initialized to zero by the translation
 * tables library.
 */
#define XLAT_TABLE_SECTION				\
	xlat_table (NOLOAD) : {				\
		__XLAT_TABLE_START__ = .;		\
		*(SORT_BY_ALIGNMENT(xlat_table))	\
		. = ALIGN(PAGE_SIZE);			\
		__XLAT_TABLE_END__ = .;			\
	}

#endif /* BL_COMMON_LD_H */
